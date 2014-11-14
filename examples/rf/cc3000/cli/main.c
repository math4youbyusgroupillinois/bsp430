/** This file is in the public domain.
 *
 * This program is an example; see detailed discussion within the
 * BSP430 documentation.
 *
 * @homepage http://github.com/pabigot/bsp430
 */

#include <bsp430/platform.h>
#include <bsp430/clock.h>
#include <bsp430/periph/port.h>
#include <bsp430/periph/sys.h>
#include <bsp430/periph/flash.h>
#include <bsp430/periph/pmm.h>
#if (BSP430_CORE_TOOLCHAIN_GCC_MSPGCC - 0)
#include <sys/crtld.h>
#elif (BSP430_CORE_TOOLCHAIN_GCC_MSP430_ELF - 0)
#ifdef __MSP430F5438A__
#define __infob ((unsigned char *)0x1980)
#define __info_segment_size 128
#endif
#endif /* get info section information */
#include <bsp430/utility/uptime.h>
#include <bsp430/utility/console.h>
#include <bsp430/utility/led.h>
#include <bsp430/utility/cli.h>
#include <bsp430/rf/cc3000.h>
#include <cc3000/host_driver_version.h>
#include <cc3000/wlan.h>
#include <cc3000/nvmem.h>
#include <cc3000/netapp.h>
#include <cc3000/hci.h>
#include <cc3000/security.h>
#include <cc3000/socket.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#ifndef BYPASS_SP_LOAD
/* If defined to a true value, the wlan_init() call at power-up will
 * inhibit loading of any service packs that may be present in the
 * NVMEM.  This will allow you to get access to the board when the
 * NVMEM is corrupt.
 *
 * BSP430 does not currently have support for complete restoration of
 * the NVMEM, which includes defining the FAT, providing radio module
 * parameters, etc. */
#define BYPASS_SP_LOAD 0
#endif /* BYPASS_SP_LOAD */

/* Memory is extremely tight on the FR5739 (16 kB including CC3000
 * buffers).  Set these to restrict the set of commands to ones of
 * interest that will fit.  You'll probably find you'll need to
 * disable WLAN_CONNECT, which makes this pretty useless.
 *
 * On the EXP430G2 there isn't enough memory to do anything, so
 * disable everything.
 *
 * No issues on the "real" chips: MSP430F5438, MSP430F5529, .... */
#if ! (BSP430_PLATFORM_EXP430G2 - 0)
#define CMD_WLAN 1
#define CMD_WLAN_BROKEN 1
#define CMD_WLAN_STOP 1
#define CMD_WLAN_STATUS 1
#define CMD_WLAN_CONNECT 1
#define CMD_WLAN_IPCONFIG 1
#define CMD_WLAN_DISCONNECT 1
#define CMD_WLAN_START 1
#define CMD_WLAN_PROFILE 1
#if (BSP430_CC3000_ENABLE_SMART - 0) && (BSP430_CC3000SPI_RX_BUFFER_SIZE < 1500)
#error BSP430_CC3000_ENABLE_SMART requires BSP430_CC3000SPI_RX_BUFFER_SIZE >= 1500
#endif /* BSP430_CC3000_ENABLE_SMART diagnostic */
#ifndef CMD_WLAN_SMART
#define CMD_WLAN_SMART (BSP430_CC3000SPI_RX_BUFFER_SIZE >= 1500)
#endif /* CMD_WLAN_SMART */
#define CMD_NVMEM 1
#define CMD_NVMEM_SP 1
#define CMD_NVMEM_RMPARAM 1
#define CMD_NVMEM_READ 1
#define CMD_NVMEM_MAC 1
#define CMD_SCAN 1
#define CMD_INFO 1
#define CMD_HELP 1
#endif /* CC3000BOOST */

#if (NO_HELP - 0)
#define HELP_STRING(h_) NULL
#else /* NO_HELP */
#define HELP_STRING(h_) h_
#endif /* NO_HELP */

#define MAGIC_CONNECT_PARAMS 0x3000

#define MAX_SSID_LEN 32
#define MAX_PASSPHRASE_LEN 64

typedef struct sConnectParams {
  unsigned int magic;
  int security_type;
  size_t ssid_len;
  size_t passphrase_len;
  char ssid[MAX_SSID_LEN+1];
  unsigned char passphrase[MAX_PASSPHRASE_LEN+1];
} sConnectParams;

static uint32_t lebtohl (uint8_t * vp)
{
  return ((uint32_t)vp[0]
          | ((uint32_t)vp[1] << 8)
          | ((uint32_t)vp[2] << 16)
          | ((uint32_t)vp[3] << 24));
}

static uint32_t bebtohl (uint8_t * vp)
{
  return ((uint32_t)vp[3]
          | ((uint32_t)vp[2] << 8)
          | ((uint32_t)vp[1] << 16)
          | ((uint32_t)vp[0] << 24));
}

const char *
ipv4AsText (const unsigned char * ipaddr)
{
  static char buffer[16];
  snprintf(buffer, sizeof(buffer), "%u.%u.%u.%u", ipaddr[3], ipaddr[2], ipaddr[1], ipaddr[0]);
  return buffer;
}

const char *
net_ipv4AsText (const unsigned char * ipaddr)
{
  static char buffer[16];
  snprintf(buffer, sizeof(buffer), "%u.%u.%u.%u", ipaddr[0], ipaddr[1], ipaddr[2], ipaddr[3]);
  return buffer;
}

volatile unsigned long lastKeepAlive_utt;
volatile unsigned long lastCallback_utt;
volatile long lastEventType;

static void wlan_cb (long event_type,
                     char * data,
                     unsigned char length)
{
  lastEventType = event_type;
  lastCallback_utt = ulBSP430uptime_ni();

  /* Ignore unsolicited keep-alives, which occur every 10 seconds
   * whenever the WLAN has been started, regardless of whether it's
   * connected or scanning. */
  if (HCI_EVNT_WLAN_KEEPALIVE == event_type) {
    lastKeepAlive_utt = lastCallback_utt;
    return;
  }

  cprintf("%s wlan_cb %#lx %u at %p SR %#x\n",
          xBSP430uptimeAsText_ni(lastCallback_utt),
          event_type, length, data, __read_status_register());
}

const sBSP430cliCommand * commandSet;
#define LAST_COMMAND NULL

#if (CMD_WLAN - 0)
#undef LAST_SUB_COMMAND
#define LAST_SUB_COMMAND NULL

#if (CMD_WLAN_STOP - 0)
static int
cmd_wlan_stop (const char * argstr)
{
  wlan_stop();
  return 0;
}
static sBSP430cliCommand dcmd_wlan_stop = {
  .key = "stop",
  .help = HELP_STRING("# shut down CC3000"),
  .next = LAST_SUB_COMMAND,
  .handler = iBSP430cliHandlerSimple,
  .param.simple_handler = cmd_wlan_stop
};
#undef LAST_SUB_COMMAND
#define LAST_SUB_COMMAND &dcmd_wlan_stop
#endif /* CMD_WLAN_STOP */

#if (CMD_WLAN_DISCONNECT - 0)
static int
cmd_wlan_disconnect (const char * argstr)
{
  long rl = wlan_disconnect();
  cprintf("disconnect returned %ld\n", rl);
  return 0;
}
static sBSP430cliCommand dcmd_wlan_disconnect = {
  .key = "disconnect",
  .help = HELP_STRING("# disconnect from AP"),
  .next = LAST_SUB_COMMAND,
  .handler = iBSP430cliHandlerSimple,
  .param.simple_handler = cmd_wlan_disconnect
};
#undef LAST_SUB_COMMAND
#define LAST_SUB_COMMAND &dcmd_wlan_disconnect
#endif /* CMD_WLAN_DISCONNECT */

#if (CMD_WLAN_IPCONFIG - 0)
static int
cmd_wlan_ipconfig (const char * argstr)
{
  tNetappIpconfigRetArgs ipc;
  const unsigned char * p;

  memset(&ipc, 0, sizeof(ipc));
  netapp_ipconfig(&ipc);
  cprintf("IP: %s\n", ipv4AsText(ipc.aucIP));
  cprintf("NM: %s\n", ipv4AsText(ipc.aucSubnetMask));
  cprintf("GW: %s\n", ipv4AsText(ipc.aucDefaultGateway));
  cprintf("DHCP: %s\n", ipv4AsText(ipc.aucDHCPServer));
  cprintf("DNS: %s\n", ipv4AsText(ipc.aucDNSServer));
  p = ipc.uaMacAddr;
  /* WTF?  A little-endian MAC address?  This may be because of a bug
   * fix in the host driver; the original version was completely
   * wrong. */
  cprintf("MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
          p[5], p[4], p[3], p[2], p[1], p[0]);
  cprintf("SSID: %s\n", ipc.uaSSID);
  return 0;
}
static sBSP430cliCommand dcmd_wlan_ipconfig = {
  .key = "ipconfig",
  .help = HELP_STRING("# show IP parameters"),
  .next = LAST_SUB_COMMAND,
  .handler = iBSP430cliHandlerSimple,
  .param.simple_handler = cmd_wlan_ipconfig
};
#undef LAST_SUB_COMMAND
#define LAST_SUB_COMMAND &dcmd_wlan_ipconfig

static int
cmd_wlan_dhcp (const char * argstr)
{
  size_t argstr_len = strlen(argstr);
  int store_ipc = 0;
  long lrc;
  union {
    uint32_t u32;
    uint8_t u8[4];
  } address[4];

  (void)iBSP430cliStoreExtractedI(&argstr, &argstr_len, &store_ipc);
  if (store_ipc) {
    tNetappIpconfigRetArgs ipc;
    memset(&ipc, 0, sizeof(ipc));
    netapp_ipconfig(&ipc);
    cprintf("Copying DHCP from IP config\n");
    address[0].u32 = bebtohl(ipc.aucIP);
    address[1].u32 = bebtohl(ipc.aucSubnetMask);
    address[2].u32 = bebtohl(ipc.aucDefaultGateway);
    address[3].u32 = bebtohl(ipc.aucDNSServer);
  } else {
    cprintf("Clearing DHCP config\n");
    memset(address, 0, sizeof(address));
  }
  cprintf("IP: %s\n", net_ipv4AsText(address[0].u8));
  cprintf("NM: %s\n", net_ipv4AsText(address[1].u8));
  cprintf("GW: %s\n", net_ipv4AsText(address[2].u8));
  cprintf("DNS: %s\n", net_ipv4AsText(address[3].u8));
  lrc = netapp_dhcp(&address[0].u32,
                    &address[1].u32,
                    &address[2].u32,
                    &address[3].u32);
  cprintf("dhcp got %ld\n", lrc);
  return 0;
}
static sBSP430cliCommand dcmd_wlan_dhcp = {
  .key = "dhcp",
  .help = HELP_STRING("[1] # reset(0)/set(1) dhcp params"),
  .next = LAST_SUB_COMMAND,
  .handler = iBSP430cliHandlerSimple,
  .param.simple_handler = cmd_wlan_dhcp
};
#undef LAST_SUB_COMMAND
#define LAST_SUB_COMMAND &dcmd_wlan_dhcp

#endif /* CMD_WLAN_IPCONFIG */

typedef struct sWlanSecMap {
  unsigned int val;
  const char * tag;
} sWlanSecMap;
static const sWlanSecMap wlanSecMap[] = {
  { WLAN_SEC_UNSEC, "unsec" },
  { WLAN_SEC_WEP, "wep" },
  { WLAN_SEC_WPA, "wpa" },
  { WLAN_SEC_WPA2, "wpa2" },
};
static const sWlanSecMap * const wlanSecMap_end = wlanSecMap + sizeof(wlanSecMap)/sizeof(*wlanSecMap);
static const sWlanSecMap * wlanSecMapFromValue (unsigned int val)
{
  const sWlanSecMap * mp;
  for (mp = wlanSecMap; mp < wlanSecMap_end; ++mp) {
    if (mp->val == val) {
      return mp;
    }
  }
  return NULL;
}
static const sWlanSecMap * wlanSecMapFromTag (const char * tag)
{
  const sWlanSecMap * mp;
  for (mp = wlanSecMap; mp < wlanSecMap_end; ++mp) {
    if (0 == strcmp(mp->tag, tag)) {
      return mp;
    }
  }
  return NULL;
}

#if (CMD_WLAN_CONNECT - 0)

sConnectParams connectParams;

static int
cmd_wlan_sectype (const char * argstr)
{
  size_t remaining = strlen(argstr);
  size_t len;
  const char * tp;
  const sWlanSecMap * mp;

  tp = xBSP430cliNextQToken(&argstr, &remaining, &len);
  if (0 < len) {
    mp = wlanSecMapFromTag(tp);
    if (0 != mp) {
      connectParams.security_type = mp->val;
    } else {
      cprintf("ERR: Unrecognized sectype '%s': valid are", tp);
      for (mp = wlanSecMap; mp < wlanSecMap_end; ++mp) {
        cprintf(" %s", mp->tag);
      }
      cprintf("\n");
    }
  }
  mp = wlanSecMapFromValue(connectParams.security_type);
  cprintf("AP security type is %s (%u)\n", (mp ? mp->tag : "<UNDEF>"), connectParams.security_type);
  return 0;
}

static int
cmd_wlan_ssid (const char * argstr)
{
  size_t remaining = strlen(argstr);
  size_t len;
  const char * tp;

  tp = xBSP430cliNextQToken(&argstr, &remaining, &len);
  if (0 < len) {
    memcpy(connectParams.ssid, tp, len);
    connectParams.ssid[len] = 0;
    connectParams.ssid_len = len;
  }
  cprintf("AP SSID is %s (%u chars)\n", connectParams.ssid, (unsigned int)connectParams.ssid_len);
  return 0;
}

static int
cmd_wlan_passphrase (const char * argstr)
{
  size_t remaining = strlen(argstr);
  size_t len;
  const char * tp;

  tp = xBSP430cliNextQToken(&argstr, &remaining, &len);
  if (0 < len) {
    if (sizeof(connectParams.passphrase) <= (len+1)) {
      cprintf("ERR: Passphrase too long (> %u chars + EOS)\n", (unsigned int)sizeof(connectParams.passphrase));
    } else {
      memcpy(connectParams.passphrase, tp, len);
      connectParams.passphrase[len] = 0;
      connectParams.passphrase_len = len;
    }
  }
  cprintf("AP passphrase is '%s' (%u chars)\n", connectParams.passphrase, (unsigned int)connectParams.passphrase_len);
  return 0;
}

static int
cmd_wlan_autocon (const char * argstr)
{
  long rc;
  int rv;
  size_t argstr_len = strlen(argstr);
  unsigned int should_connect_to_open_ap = 0;
  unsigned int should_use_fast_connect = 1;
  unsigned int use_profiles = 1;
  uint8_t u32octets[4];

  /* Analysis shows:
   * should_connect_to_open_ap in NVS at 0x44 32-bit int
   * should_use_fast_connect in NVS at 0x48 32-bit int
   * use_profiles in NVS at 0x50 32-bit int */
  cprintf("Current connection policy:\n");
  rc = nvmem_read(NVMEM_WLAN_CONFIG_FILEID, sizeof(uint32_t), 0x44, u32octets);
  if (0 == rc) {
    cprintf("should_connect_to_open_ap = 0x%08lx\n", lebtohl(u32octets));
  }
  rc = nvmem_read(NVMEM_WLAN_CONFIG_FILEID, sizeof(uint32_t), 0x48, u32octets);
  if (0 == rc) {
    cprintf("should_use_fast_connect = 0x%08lx\n", lebtohl(u32octets));
  }
  rc = nvmem_read(NVMEM_WLAN_CONFIG_FILEID, sizeof(uint32_t), 0x50, u32octets);
  if (0 == rc) {
    cprintf("use_profiles = 0x%08lx\n", lebtohl(u32octets));
  }
  rv = iBSP430cliStoreExtractedUI(&argstr, &argstr_len, &should_connect_to_open_ap);
  if (0 == rv) {
    rv = iBSP430cliStoreExtractedUI(&argstr, &argstr_len, &should_use_fast_connect);
  }
  if (0 == rv) {
    rv = iBSP430cliStoreExtractedUI(&argstr, &argstr_len, &use_profiles);
  }
  cprintf("New connection policy: open_ap=%u fast_connect=%u use_profiles=%u\n",
          should_connect_to_open_ap, should_use_fast_connect, use_profiles);
  rc = wlan_ioctl_set_connection_policy(should_connect_to_open_ap, should_use_fast_connect, use_profiles);
  cprintf("Connection policy set got %ld\n", rc);
  return 0;
}

static int
cmd_wlan_connect (const char * argstr)
{
  long rl;
  const sWlanSecMap * mp;
  unsigned long t[2];

  mp = wlanSecMapFromValue(connectParams.security_type);
  cprintf("connect to ssid '%s' by %s using '%s'\n",
          connectParams.ssid, (mp ? mp->tag : "<UNDEF>"),
          connectParams.passphrase);
  t[0] = ulBSP430uptime_ni();
  rl = wlan_connect(connectParams.security_type,
                    connectParams.ssid, connectParams.ssid_len,
                    NULL,
                    connectParams.passphrase, connectParams.passphrase_len);
  t[1] = ulBSP430uptime_ni();
  cprintf("con %ld in %s\n", rl, xBSP430uptimeAsText_ni(t[1]-t[0]));
  return 0;
}
static sBSP430cliCommand dcmd_wlan_ssid = {
  .key = "ssid",
  .help = HELP_STRING("[{ssid}] # Display or set AP SSID for connect"),
  .next = LAST_SUB_COMMAND,
  .handler = iBSP430cliHandlerSimple,
  .param.simple_handler = cmd_wlan_ssid
};
static sBSP430cliCommand dcmd_wlan_passphrase = {
  .key = "passphrase",
  .help = HELP_STRING("[{passphrase}] # Display or set AP passphrase"),
  .next = &dcmd_wlan_ssid,
  .handler = iBSP430cliHandlerSimple,
  .param.simple_handler = cmd_wlan_passphrase
};
static sBSP430cliCommand dcmd_wlan_sectype = {
  .key = "sectype",
  .help = HELP_STRING("[{sectype}] # Display or set AP sectype"),
  .next = &dcmd_wlan_passphrase,
  .handler = iBSP430cliHandlerSimple,
  .param.simple_handler = cmd_wlan_sectype
};
static sBSP430cliCommand dcmd_wlan_autocon = {
  .key = "autocon",
  .help = HELP_STRING("[open_ap=0 [fast_conn=1 [auto_conn=1]]]"),
  .next = &dcmd_wlan_sectype,
  .handler = iBSP430cliHandlerSimple,
  .param.simple_handler = cmd_wlan_autocon
};
static sBSP430cliCommand dcmd_wlan_connect = {
  .key = "connect",
  .help = HELP_STRING("# connect to specified access point using AP config"),
  .next = &dcmd_wlan_autocon,
  .handler = iBSP430cliHandlerSimple,
  .param.simple_handler = cmd_wlan_connect
};
#undef LAST_SUB_COMMAND
#define LAST_SUB_COMMAND &dcmd_wlan_connect
#endif /* CMD_WLAN_CONNECT */

#if (CMD_WLAN_PROFILE - 0)
static int
cmd_wlan_profile_del (const char * argstr)
{
  size_t argstr_len = strlen(argstr);
  long rc;
  int profile = 255;

  /* Use 255 to delete all profiles. */
  (void)iBSP430cliStoreExtractedI(&argstr, &argstr_len, &profile);
  rc = wlan_ioctl_del_profile(profile);
  cprintf("Del profile %d got %ld\n", profile, rc);
  return 0;
}

typedef struct sProfileEntry {
  unsigned int priority;        /**< 0 to 7; 0 is lowest priority, 1 used by Smart Config */
  int security_type;            /**< One of the WLAN_SEC_* types  */
  const char * ssid;            /**< SSID as text string, NUL terminated */
  const char * passphrase;      /**< Passphrase in plain text, NUL terminated */
} sProfileEntry;

static int
cmd_wlan_profile_update (const char * argstr)
{
  static const sProfileEntry entries[] = {
#if (WITH_PROFILES - 0)
#include "profiles.inc"
#endif /* WITH_PROFILES */
  };
  const sProfileEntry * pep = entries;
  const sProfileEntry * const pepe = pep + sizeof(entries)/sizeof(*entries);
  long rc;

  if (pep == pepe) {
    cprintf("No profiles available\n");
    return -1;
  }

  /* Per
   * http://e2e.ti.com/support/low_power_rf/f/851/p/285991/999909.aspx#999909
   * the magic constants below are from wpa_supplicant. */
#define WPA_DRIVER_CAPA_KEY_MGMT_WPA2              0x00000002
#define WPA_CIPHER_NONE 0x01
#define WPA_CIPHER_WEP40 0x02
#define WPA_CIPHER_WEP104 0x04
#define WPA_CIPHER_TKIP 0x08
#define WPA_CIPHER_CCMP 0x10

  while (pep < pepe) {
    uint32_t pairwisecipher_or_keylen = 0;
    uint32_t groupcipher_or_keyindex = 0;
    uint32_t key_mgmt = 0;
    uint8_t * pf_or_key = NULL;
    uint32_t key_length = 0;
    const sWlanSecMap * const mp = wlanSecMapFromValue(pep->security_type);

    if (NULL != mp) {
      if ((WLAN_SEC_WPA == pep->security_type)
          || (WLAN_SEC_WPA2 == pep->security_type)) {
        pairwisecipher_or_keylen = WPA_CIPHER_TKIP | WPA_CIPHER_CCMP;
        groupcipher_or_keyindex = WPA_CIPHER_WEP40 | WPA_CIPHER_WEP104 | WPA_CIPHER_TKIP | WPA_CIPHER_CCMP;
        key_mgmt = WPA_DRIVER_CAPA_KEY_MGMT_WPA2;
        pf_or_key = (uint8_t*)pep->passphrase;
        key_length = strlen(pep->passphrase);
      } else if (WLAN_SEC_WEP == pep->security_type) {
        pairwisecipher_or_keylen = strlen(pep->passphrase);
        groupcipher_or_keyindex = 0;
        key_mgmt = 0;
        pf_or_key = (uint8_t*)pep->passphrase;
        key_length = 0;
      } else if (WLAN_SEC_UNSEC == pep->security_type) {
        pairwisecipher_or_keylen = 0;
        groupcipher_or_keyindex = 0;
        key_mgmt = 0;
        pf_or_key = NULL;
        key_length = 0;
      }
      rc = wlan_add_profile(pep->security_type,
                            (uint8_t*)pep->ssid,
                            strlen(pep->ssid),
                            NULL,   /* bssid is inferred */
                            pep->priority,
                            pairwisecipher_or_keylen,
                            groupcipher_or_keyindex,
                            key_mgmt,
                            pf_or_key,
                            key_length);

      cprintf("Add SSID '%s' sec '%s' pri %u got %ld\n",
              pep->ssid, mp->tag, pep->priority, rc);
    } else {
      cprintf("ERR: Profile %u %s has invalid security type %d\n",
              (unsigned int)(pep - entries),
              pep->ssid, pep->security_type);
    }
    ++pep;
  }
  return 0;
}

static sBSP430cliCommand dcmd_wlan_profile_del = {
  .key = "del",
  .help = HELP_STRING("[profile=0] # delete profile, 255 for all"),
  .next = NULL,
  .handler = iBSP430cliHandlerSimple,
  .param.simple_handler = cmd_wlan_profile_del
};

static sBSP430cliCommand dcmd_wlan_profile_update = {
  .key = "update",
  .help = HELP_STRING("[profile=0] # add configured profiles"),
  .next = &dcmd_wlan_profile_del,
  .handler = iBSP430cliHandlerSimple,
  .param.simple_handler = cmd_wlan_profile_update
};


static sBSP430cliCommand dcmd_wlan_profile = {
  .key = "profile",
  .next = LAST_SUB_COMMAND,
  .child = &dcmd_wlan_profile_update
};
#undef LAST_SUB_COMMAND
#define LAST_SUB_COMMAND &dcmd_wlan_profile
#endif /* CMD_WLAN_PROFILE */

#if (CMD_WLAN_SMART - 0)
static int
cmd_wlan_smart_aes (const char * argstr)
{
  size_t argstr_len = strlen(argstr);
  const char * key = NULL;
  size_t keylen;
  unsigned char keybuf[AES128_KEY_SIZE];
  long rc;

  while (isspace((unsigned char)*argstr)) {
    ++argstr;
  }
  argstr_len = strlen(argstr);
  if (*argstr) {
    key = xBSP430cliNextQToken(&argstr, &argstr_len, &keylen);
    memset(keybuf, 0, sizeof(keybuf));
    if (sizeof(keybuf) < keylen) {
      keylen = sizeof(keybuf);
    }
    memcpy(keybuf, key, keylen);
    rc = aes_write_key(keybuf);
    cprintf("aes_write_key got %ld\n", rc);
  } else {
    rc = aes_read_key(keybuf);
    cprintf("aes_read_key got %ld\n", rc);
  }
  cprintf("Key:\n");
  vBSP430consoleDisplayMemory((unsigned char *)keybuf, sizeof(keybuf), 0);
  return 0;
}

static int
cmd_wlan_smart_process (const char * argstr)
{
  long rc;

  rc = wlan_smart_config_process();
  cprintf("wlan_smart_config_process() got %ld\n", rc);
  return 0;
}

static int
cmd_wlan_smart_stop (const char * argstr)
{
  long rc;

  rc = wlan_smart_config_stop();
  cprintf("wlan_smart_config_stop() got %ld\n", rc);
  return 0;
}

static int
cmd_wlan_smart_start (const char * argstr)
{
  size_t argstr_len = strlen(argstr);
  unsigned int encrypted = 0;
  long rc;

  /* This has to be TTT.  It doesn't matter what you pass as long as
   * it's non-null; SP 1.10.1 through 1.12 (at least) will use TTT.
   * In fact, it will helpfully overwrite whatever you pass in so that
   * you know you're using TTT.  So you can't pass a string literal
   * unless your MCU allows you to write into flash, which is probably
   * a bad thing to promote given the use of FRAM devices. */
  {
    char prefix[] = "123";
    cprintf("Setting prefix to '%s'\n", prefix);
    rc = wlan_smart_config_set_prefix(prefix);
    cprintf("Prefix set returned %ld, now '%s'\n", rc, prefix);
  }

  (void)iBSP430cliStoreExtractedUI(&argstr, &argstr_len, &encrypted);
  if (encrypted) {
    unsigned char keybuf[AES128_KEY_SIZE];
    rc = aes_read_key(keybuf);
    cprintf("Config with AES key:\n");
    vBSP430consoleDisplayMemory((unsigned char *)keybuf, sizeof(keybuf), 0);
  } else {
    cprintf("Config unencrypted\n");
  }
  rc = wlan_smart_config_start(encrypted);
  cprintf("wlan_smart_config_start(%d) got %ld\n", encrypted, rc);
  return 0;
}

static sBSP430cliCommand dcmd_wlan_smart_aes = {
  .key = "aes",
  .help = HELP_STRING("[key] # set or read AES key"),
  .next = NULL,
  .handler = iBSP430cliHandlerSimple,
  .param.simple_handler = cmd_wlan_smart_aes
};
static sBSP430cliCommand dcmd_wlan_smart_process = {
  .key = "process",
  .help = HELP_STRING("# process AES received data"),
  .next = &dcmd_wlan_smart_aes,
  .handler = iBSP430cliHandlerSimple,
  .param.simple_handler = cmd_wlan_smart_process
};
static sBSP430cliCommand dcmd_wlan_smart_stop = {
  .key = "stop",
  .help = HELP_STRING("# stop smart config process"),
  .next = &dcmd_wlan_smart_process,
  .handler = iBSP430cliHandlerSimple,
  .param.simple_handler = cmd_wlan_smart_stop
};
static sBSP430cliCommand dcmd_wlan_smart_start = {
  .key = "start",
  .help = HELP_STRING("[encrypted=0] # start smart config process"),
  .next = &dcmd_wlan_smart_stop,
  .handler = iBSP430cliHandlerSimple,
  .param.simple_handler = cmd_wlan_smart_start
};

static sBSP430cliCommand dcmd_wlan_smart = {
  .key = "smart",
  .next = LAST_SUB_COMMAND,
  .child = &dcmd_wlan_smart_start
};
#undef LAST_SUB_COMMAND
#define LAST_SUB_COMMAND &dcmd_wlan_smart
#endif /* CMD_WLAN_SMART */

#if (CMD_WLAN_STATUS - 0)
static int
cmd_wlan_status (const char * argstr)
{
  /* Contrary to the documentation, there are no macro constants
   * defined for these states */
  static const char * status_str[] = {
    "disconnected",
    "scanning",
    "connecting",
    "connected"
  };
  long rl = wlan_ioctl_statusget();
#if (CMD_WLAN_CONNECT - 0)
  {
    const sWlanSecMap * mp;
    mp = wlanSecMapFromValue(connectParams.security_type);

    cprintf("AP configuration SSID '%s', passphrase '%s'\n",
            connectParams.ssid, connectParams.passphrase);
    cprintf("AP security type %s (%u)\n", (mp ? mp->tag : "<UNDEF>"), connectParams.security_type);
  }
#endif /* CMD_WLAN_CONNECT */
  cprintf("WLAN status %ld", rl);
  if ((0 <= rl) && (rl < (sizeof(status_str)/sizeof(*status_str)))) {
    cprintf(": %s", status_str[rl]);
  }
  cputchar('\n');
  return 0;
}
static sBSP430cliCommand dcmd_wlan_status = {
  .key = "status",
  .help = HELP_STRING("# get CC3000 status and AP config"),
  .next = LAST_SUB_COMMAND,
  .handler = iBSP430cliHandlerSimple,
  .param.simple_handler = cmd_wlan_status
};
#undef LAST_SUB_COMMAND
#define LAST_SUB_COMMAND &dcmd_wlan_status
#endif /* CMD_WLAN_STATUS */

#if (CMD_WLAN_BROKEN - 0)
static int
cmd_wlan_timeouts (const char * argstr)
{
  long rc;
  struct {
    unsigned long dhcp_s;
    unsigned long arp_s;
    unsigned long ka_s;
    unsigned long inactive_s;
  } params;

  params.dhcp_s = 14400;
  params.arp_s = 3600;
  params.ka_s = 600000;
  params.inactive_s = 600000;
  cprintf("*** NOTE: No evidence this function works (KA does not)\n");
  cprintf("DHCP timeout: %lu sec\n", params.dhcp_s);
  cprintf("ARP timeout: %lu sec\n", params.arp_s);
  cprintf("Keep-Alive timeout: %lu sec\n", params.ka_s);
  cprintf("Inactive timeout: %lu sec\n", params.inactive_s);
  rc = netapp_timeout_values(&params.dhcp_s, &params.arp_s, &params.ka_s, &params.inactive_s);
  cprintf("Timeout set got %ld\n", rc);
  return 0;
}
static sBSP430cliCommand dcmd_wlan_timeouts = {
  .key = "timeouts",
  .help = HELP_STRING("# set CC3000 timeouts"),
  .next = LAST_SUB_COMMAND,
  .handler = iBSP430cliHandlerSimple,
  .param.simple_handler = cmd_wlan_timeouts
};
#undef LAST_SUB_COMMAND
#define LAST_SUB_COMMAND &dcmd_wlan_timeouts
#endif /* CMD_WLAN_BROKEN */

#if (CMD_WLAN_START - 0)
static int
cmd_wlan_start (const char * argstr)
{
  size_t argstr_len = strlen(argstr);
  unsigned int patches_available = 0;
  unsigned long t[2];

  (void)iBSP430cliStoreExtractedUI(&argstr, &argstr_len, &patches_available);
  t[0] = ulBSP430uptime_ni();
  wlan_start(patches_available);
  t[1] = ulBSP430uptime_ni();
  cprintf("wlan_start(%u) took %s\n", patches_available, xBSP430uptimeAsText_ni(t[1]-t[0]));
  return 0;
}
static sBSP430cliCommand dcmd_wlan_start = {
  .key = "start",
  .help = HELP_STRING("[patchcount=0] # power-up CC3000"),
  .next = LAST_SUB_COMMAND,
  .handler = iBSP430cliHandlerSimple,
  .param.simple_handler = cmd_wlan_start
};
#undef LAST_SUB_COMMAND
#define LAST_SUB_COMMAND &dcmd_wlan_start
#endif /* CMD_WLAN_START */

static sBSP430cliCommand dcmd_wlan = {
  .key = "wlan",
  .next = LAST_COMMAND,
  .child = LAST_SUB_COMMAND
};
#undef LAST_COMMAND
#define LAST_COMMAND &dcmd_wlan
#undef LAST_SUB_COMMAND
#define LAST_SUB_COMMAND NULL

#endif /* CMD_WLAN */

#if (CMD_SCAN - 0)
/** Scan status when there are no entries that have not already been
 * returned */
#define X_WLAN_SCAN_STATUS_AGED 0
/** Scan status when result is valid and has not previously been
 * returned */
#define X_WLAN_SCAN_STATUS_VALID 1
/** Scan status when no scan has yet been initiated */
#define X_WLAN_SCAN_STATUS_NONE 2

/** Structure filled in by wlan_ioctl_get_scan_results().
 *
 * @warning The four bit fields work only with little-endian data with
 * bits allocated from the least significant bit upward. */
typedef struct sWLANFullScanResults {
  /** Number of results remaining, including this one */
  uint32_t network_count;
  /** Scan status per X_WLAN_SCAN_STATUS_* */
  uint32_t scan_status;
  /** Non-zero if result is valid; alternatively, sign bit for
   * subsequent RSSI */
  unsigned int isValid:1;
  /** RSSI as the unsigned offset from -128 dBm. */
  unsigned int rssi:7;
  /** Security mode per WLAN_SEC_* */
  unsigned int securityMode:2;
  /** Number of valid characters in ssid array */
  unsigned int ssidNameLength:6;
  /** Time last heard from entry, in seconds since start of wlan */
  uint16_t entryTime;
  /** SSID, no termination (see #ssidNameLength) */
  char ssid[32];
  /** BSSID */
  unsigned char bssid[6];
} sWLANFullScanResults;

static int
cmd_scan_show (const char * argstr)
{
  size_t argstr_len = strlen(argstr);
  int with_mem = 0;
  long rc;
  union {
    sWLANFullScanResults structure;
    unsigned char bytes[1];
  } uresult;
  const sWLANFullScanResults * const sp = &uresult.structure;

  (void)iBSP430cliStoreExtractedI(&argstr, &argstr_len, &with_mem);
  cprintf("Scan results:\n"
          "SSID                             "
          "Scty  "
          "BSSID             "
          "RSSI "
          "Time"
          "\n");
  do {
    rc = wlan_ioctl_get_scan_results(0, uresult.bytes);
    if (0 != rc) {
      cprintf("ERR: rc %ld\n", rc);
      break;
    }
    if (X_WLAN_SCAN_STATUS_NONE == sp->scan_status) {
      cprintf("No results\n");
      break;
    }
    if (X_WLAN_SCAN_STATUS_AGED == sp->scan_status) {
      cprintf("No new results\n");
      break;
    }
    if (! sp->isValid) {
      cprintf("Invalid\n");
      continue;
    }
    if (with_mem) {
      vBSP430consoleDisplayMemory(uresult.bytes, sizeof(*sp), 0);
    }
    { /* SSID */
      const int nsp = sp->ssidNameLength;
      int i;
      for (i = 0; i < sizeof(sp->ssid); ++i) {
        cputchar((i < nsp) ? sp->ssid[i] : ' ');
      }
      cputchar(' ');
    }
    { /* Security */
      const sWlanSecMap * mp = wlanSecMapFromValue(sp->securityMode);
      cprintf("%-5s ", (NULL == mp) ? "???" : mp->tag);
    }
    { /* BSSI */
      int i;
      cprintf("%02x", sp->bssid[0]);
      for (i = 1; i < sizeof(sp->bssid); ++i) {
        cprintf(":%02x", sp->bssid[i]);
      }
      cputchar(' '); /* 17 chars plus separating space */
    }
    cprintf("% 3d  ", -128 + sp->rssi); /* RSSI */
    cprintf("%-5u ", sp->entryTime);    /* Time */
    cputchar('\n');
  } while (1 < sp->network_count);
  return 0;
}

static int
cmd_scan_start (const char * argstr)
{
  size_t argstr_len = strlen(argstr);
  unsigned long rc;
  unsigned long interval_ms = 1;
  unsigned long delays[16];
  int i;

  (void)iBSP430cliStoreExtractedUL(&argstr, &argstr_len, &interval_ms);

  for (i = 0; i < sizeof(delays)/sizeof(*delays); ++i) {
    delays[i] = 2000;
  }

  rc = wlan_ioctl_set_scan_params(interval_ms, /* Enable (1 is 10 min interval; other values are interval between active probe sweeps, in ms) */
                                  100, /* Min dwell per channel */
                                  150, /* Max dwell per channel */
                                  5,  /* Probes within dwell.  0 will still send one probe. */
                                  0x7ff, /* Channel mask: 0x7ff enables 1..11 */
                                  -80,   /* Default RSSI threshold.  No apparent effect changing this. */
                                  0,   /* SNR threshold */
                                  205, /* Probe TX power */
                                  delays); /* Timeout between scans */
  cprintf("Scan start %lu ms got %ld\n", interval_ms, rc);
  return rc;
}

static int
cmd_scan_stop (const char * argstr)
{
  unsigned long rc;
  unsigned long delays[16];
  int i;

  for (i = 0; i < sizeof(delays)/sizeof(*delays); ++i) {
    delays[i] = 2000;
  }

  /* This stops the scan within the current session, but it will
   * automatically restart on the next power-up. */
  rc = wlan_ioctl_set_scan_params(0, 150, 150, 5, 0x1fff, -80, 0, 205, delays);
  cprintf("Scan stop got %ld\n", rc);
  return rc;
}

static sBSP430cliCommand dcmd_scan_show = {
  .key = "show",
  .help = HELP_STRING(" [with_mem=0] # display scan results"),
  .next = NULL,
  .handler = iBSP430cliHandlerSimple,
  .param.simple_handler = cmd_scan_show
};
static sBSP430cliCommand dcmd_scan_stop = {
  .key = "stop",
  .help = HELP_STRING("# disable periodic scan"),
  .next = &dcmd_scan_show,
  .handler = iBSP430cliHandlerSimple,
  .param.simple_handler = cmd_scan_stop
};
static sBSP430cliCommand dcmd_scan_start = {
  .key = "start",
  .help = HELP_STRING(" [interval=1] # start periodic scan (ms)"),
  .next = &dcmd_scan_stop,
  .handler = iBSP430cliHandlerSimple,
  .param.simple_handler = cmd_scan_start
};
static sBSP430cliCommand dcmd_scan = {
  .key = "scan",
  .next = LAST_COMMAND,
  .child = &dcmd_scan_start
};
#undef LAST_COMMAND
#define LAST_COMMAND &dcmd_scan

#endif /* CMD_SCAN */

#if (CMD_NVMEM - 0)
typedef struct sNVMEMFileIds {
  unsigned int fileid;
  unsigned int size;
  const char * tag;
} sNVMEMFileIds;
static const sNVMEMFileIds nvmemFiles[] = {
  { NVMEM_NVS_FILEID, 512, "NVS", },
  { NVMEM_NVS_SHADOW_FILEID, 512, "NVS (shadow)" },
  { NVMEM_WLAN_CONFIG_FILEID, 4096, "WLAN Config", },
  { NVMEM_WLAN_CONFIG_SHADOW_FILEID, 4096, "WLAN Config (shadow)" },
  { NVMEM_WLAN_DRIVER_SP_FILEID, 0, "WLAN Driver SP" },
  { NVMEM_WLAN_FW_SP_FILEID, 0, "WLAN Firmware SP" },
  { NVMEM_MAC_FILEID, 16, "MAC address" },
  { NVMEM_FRONTEND_VARS_FILEID, 0, "Front End Vars ??" },
  { NVMEM_IP_CONFIG_FILEID, 64, "IP Config" },
  { NVMEM_IP_CONFIG_SHADOW_FILEID, 64, "IP Config (shadow)" },
  { NVMEM_BOOTLOADER_SP_FILEID, 0, "Bootloader SP" },
  { NVMEM_RM_FILEID, 0, "Radio Module" },
  { NVMEM_AES128_KEY_FILEID, 0, "AES128 Key" },
  { NVMEM_SHARED_MEM_FILEID, 0, "Shared Memory ??" },
};

#if (WITH_UPDATE - 0)
/* Firmware updates for the CC3000 are distributed by TI through the
 * Patch Programmer utility available at:
 * http://processors.wiki.ti.com/index.php/CC3000_Wi-Fi_Downloads#Patch_Programmer
 *
 * This utility is export-controlled, and there is no indication that
 * the binary firmware data is licensed with rights to redistribute.
 * Therefore, if you want to update firmware or compare radio module
 * parameters, you need to obtain the source to the patch programmer
 * and create files that contain the byte literal initializer sequence
 * to fill in the arrays below. */

/* This array should hold WLAN driver patches.  The length is
 * somewhere around 8kB, and the values can be found in the
 * wlan_drv_patch array in PatchProgrammer_DR_Patch.c from the TI
 * patch programmer utility. */
const uint8_t wlan_drv_patch[] = {
#include "wlan_drv_patch.inc"
};
/* This array should hold WLAN firmware patches.  The length is
 * somewhere around 8kB, and the values can be found in the fw_patch
 * array in PatchProgrammer_FW_Patch.c from the TI patch programmer
 * utility. */
const uint8_t fw_patch[] = {
#include "fw_patch.inc"
};

static int
cmd_nvmem_update (const char * argstr)
{
  int rc;
  int stage;
  const uint16_t drv_sp_size = sizeof(wlan_drv_patch);
  const uint16_t fw_sp_size = sizeof(fw_patch);
  uint16_t drv_sp_fatsize;
  uint16_t fw_sp_fatsize;

  do {
    stage = __LINE__;
    rc = nvmem_read(NVMEM_MAX_ENTRY, sizeof(drv_sp_fatsize), 4*(1 + NVMEM_WLAN_DRIVER_SP_FILEID) + 2, (uint8_t*)&drv_sp_fatsize);
    if (0 != rc) {
      break;
    }
    stage = __LINE__;
    rc = nvmem_read(NVMEM_MAX_ENTRY, sizeof(drv_sp_fatsize), 4*(1 + NVMEM_WLAN_FW_SP_FILEID) + 2, (uint8_t*)&fw_sp_fatsize);
    if (0 != rc) {
      break;
    }
    cprintf("Driver update size %u, file size %u\n"
            "Firmware update size %u, file size %u\n",
            drv_sp_size, drv_sp_fatsize,
            fw_sp_size, fw_sp_fatsize);
    if ((0 == drv_sp_size) || (drv_sp_size > drv_sp_fatsize)) {
      cprintf("ERROR: No driver, or driver too large\n");
      return -1;
    }
    if ((0 == fw_sp_size) || (fw_sp_size > fw_sp_fatsize)) {
      cprintf("ERROR: No firmware, or firmware too large\n");
      return -1;
    }
    cprintf("Shutting down WLAN for driver update\n");
    wlan_stop();
    cprintf("Waiting a few...");
    BSP430_CORE_DELAY_CYCLES(3 * BSP430_CLOCK_NOMINAL_MCLK_HZ);
    cprintf("back now\n");
    cprintf("Starting WLAN in patch mode\n");
    wlan_start(2);
    wlan_ioctl_set_connection_policy(0, 0, 0);
    wlan_ioctl_del_profile(255);
    wlan_set_event_mask(HCI_EVNT_WLAN_KEEPALIVE|HCI_EVNT_WLAN_UNSOL_INIT|HCI_EVNT_WLAN_ASYNC_PING_REPORT);
    cprintf("Updating driver firmware...");
    stage = __LINE__;
    rc = nvmem_write_patch(NVMEM_WLAN_DRIVER_SP_FILEID, drv_sp_size, (uint8_t *)wlan_drv_patch);
    if (0 != rc) {
      break;
    }
    cprintf("success.\n"
            "Updating firmware...");
    stage = __LINE__;
    rc = nvmem_write_patch(NVMEM_WLAN_FW_SP_FILEID, fw_sp_size, (uint8_t *)fw_patch);
    if (0 != rc) {
      break;
    }
    cprintf("success.\n"
            "Shutting down WLAN after driver update\n");
    wlan_stop();
    cprintf("Restarting WLAN with new drivers\n");
    wlan_start(0);
  } while (0);
  if (0 != rc) {
    cprintf("ERROR %d in update at line %u\n", rc, stage);
  }
  return 0;
}

static sBSP430cliCommand dcmd_nvmem_update = {
  .key = "update",
  .help = HELP_STRING("# Update firmware"),
  .next = LAST_SUB_COMMAND,
  .handler = iBSP430cliHandlerSimple,
  .param.simple_handler = cmd_nvmem_update
};
#undef LAST_SUB_COMMAND
#define LAST_SUB_COMMAND &dcmd_nvmem_update

#endif /* WITH_UPDATE */

static int
cmd_nvmem_dir (const char * argstr)
{
  int rc;
  const unsigned int num_doc_files = sizeof(nvmemFiles) / sizeof(*nvmemFiles);
  uint8_t fat[4*(NVMEM_MAX_ENTRY+1)];
  const uint16_t * fp = (const uint16_t *)(fat+4);
  const uint16_t * efp = (const uint16_t *)(fat+sizeof(fat));
  int ffi = 0;

  cprintf("%u NVMEM files with %u documented:\n", NVMEM_MAX_ENTRY, num_doc_files);
  rc = nvmem_read(NVMEM_MAX_ENTRY, sizeof(fat), 0, fat);
  if (0 != rc) {
    cprintf("ERROR: nvmem read got %d\n", rc);
    return 0;
  }
  if (('L' != fat[0]) || ('S' != fat[1])) {
    cprintf("ERROR: nvmem signature %02x %02x not %02x %02x\n",
            fat[0], fat[1], 'L', 'S');
    return 0;
  }
  cprintf("ID Addr  Size  Flg : Doc ID  Size Description\n");
  while (fp < efp) {
    cprintf("%2d %04x %5d  %c%c%c",
            ffi, fp[0] & ~(BIT0|BIT1|BIT2), fp[1],
            (fp[0] & BIT0) ? 'a' : ' ',
            (fp[0] & BIT1) ? 'v' : ' ',
            (fp[0] & BIT2) ? 'p' : ' ');
    if (ffi < sizeof(nvmemFiles)/sizeof(*nvmemFiles)) {
      cprintf("       %2d %5d %s",
              nvmemFiles[ffi].fileid,
              nvmemFiles[ffi].size,
              nvmemFiles[ffi].tag);
    }
    cputchar('\n');
    ++ffi;
    fp += 2;
  }

  return 0;
}
static sBSP430cliCommand dcmd_nvmem_dir = {
  .key = "dir",
  .help = HELP_STRING("# describe NVMEM sections"),
  .next = LAST_SUB_COMMAND,
  .handler = iBSP430cliHandlerSimple,
  .param.simple_handler = cmd_nvmem_dir
};
#undef LAST_SUB_COMMAND
#define LAST_SUB_COMMAND &dcmd_nvmem_dir

static int
cmd_nvmem_create (const char * argstr)
{
  size_t argstr_len = strlen(argstr);
  unsigned int fileid = NVMEM_AES128_KEY_FILEID;
  unsigned int length = AES128_KEY_SIZE;
  long rc;
  int rv;

  rv = iBSP430cliStoreExtractedUI(&argstr, &argstr_len, &fileid);
  if (0 == rv) {
    rv = iBSP430cliStoreExtractedUI(&argstr, &argstr_len, &length);
  }
  rc = nvmem_create_entry(fileid, length);
  cprintf("nvmem_create_entry(%u, %u) got %ld\n", fileid, length, rc);
  return 0;
}
static sBSP430cliCommand dcmd_nvmem_create = {
  .key = "create",
  .help = HELP_STRING("[fileid=12 [length=16]] # create a new NVMEM section"),
  .next = LAST_SUB_COMMAND,
  .handler = iBSP430cliHandlerSimple,
  .param.simple_handler = cmd_nvmem_create
};
#undef LAST_SUB_COMMAND
#define LAST_SUB_COMMAND &dcmd_nvmem_create

#if (CMD_NVMEM_SP - 0)
static int
cmd_nvmem_sp (const char * argstr)
{
  unsigned char patch_version[2];
  unsigned char rv;

  memset(patch_version, -1, sizeof(patch_version));
  rv = nvmem_read_sp_version(patch_version);
  cprintf("sp ret %u: %u.%u\n", rv, patch_version[0], patch_version[1]);
  cprintf("Host driver %u\n", DRIVER_VERSION_NUMBER);
  return 0;
}
static sBSP430cliCommand dcmd_nvmem_sp = {
  .key = "sp",
  .help = HELP_STRING("# read firmware service pack level"),
  .next = LAST_SUB_COMMAND,
  .handler = iBSP430cliHandlerSimple,
  .param.simple_handler = cmd_nvmem_sp
};
#undef LAST_SUB_COMMAND
#define LAST_SUB_COMMAND &dcmd_nvmem_sp
#endif /* CMD_NVMEM_SP */

#if (CMD_NVMEM_RMPARAM - 0)

#if (WITH_RMPARAM - 0)
/* This array should hold the default radio module parameters.  The
 * length is nominally 128 bytes, and the values can be found in the
 * cRMdefaultParams array in PatchProgrammer_DR_Patch.c from the TI
 * patch programmer utility. */
const uint8_t default_rm_param[] = {
#include "rm_param.inc"
};
#endif

static int
cmd_nvmem_rmparam (const char * argstr)
{
  uint8_t rmparam[128];
  const uint8_t * const rmpe = rmparam + sizeof(rmparam);
  uint8_t * rp = rmparam;
  int rc;

  while (rp < rmpe) {
    unsigned int offset = rp - rmparam;
    unsigned int nb = rmpe - rp;
    if (16 < nb) {
      nb = 16;
    }
    rc = nvmem_read(NVMEM_RM_FILEID, nb, offset, rp);
    if (0 != rc) {
      cprintf("ERROR %d reading at offset %u\n", rc, offset);
      return 0;
    }
    rp += nb;
  }
  cprintf("Radio module parameters:\n");
  vBSP430consoleDisplayMemory(rmparam, sizeof(rmparam), 0);
#if (WITH_RMPARAM - 0)
  {
    const uint8_t * drp = default_rm_param;
    const uint8_t * const drpe = default_rm_param + sizeof(default_rm_param);

    rp = rmparam;
    while ((rp < rmpe) && (drp < drpe)) {
      *rp++ ^= *drp++;
    }
  }
  cprintf("Bitwise difference from default:\n");
  vBSP430consoleDisplayMemory(rmparam, sizeof(rmparam), 0);
  cprintf("Default RM parameters:\n");
  vBSP430consoleDisplayMemory(default_rm_param, sizeof(default_rm_param), 0);
#endif /* WITH_RMPARAM */
  return 0;
}
static sBSP430cliCommand dcmd_nvmem_rmparam = {
  .key = "rmparam",
  .help = HELP_STRING("# display radio module parameters"),
  .next = LAST_SUB_COMMAND,
  .handler = iBSP430cliHandlerSimple,
  .param.simple_handler = cmd_nvmem_rmparam
};
#undef LAST_SUB_COMMAND
#define LAST_SUB_COMMAND &dcmd_nvmem_rmparam
#endif /* CMD_NVMEM_SP */

#if (CMD_NVMEM_READ - 0)
static int
cmd_nvmem_read (const char * argstr)
{
  int rc;
  unsigned int fileid = 0;
  unsigned int len = 128;
  unsigned int ofs = 0;
  unsigned char data[16];       /* Page size? */
  unsigned int end_read;
  unsigned int ui;
  size_t argstr_len = strlen(argstr);
  unsigned int nb;

  rc = iBSP430cliStoreExtractedUI(&argstr, &argstr_len, &ui);
  if (0 == rc) {
    fileid = ui;
    rc = iBSP430cliStoreExtractedUI(&argstr, &argstr_len, &ui);
  }
  if (0 == rc) {
    len = ui;
    rc = iBSP430cliStoreExtractedUI(&argstr, &argstr_len, &ui);
  }
  if (0 == rc) {
    ofs = ui;
  }
  end_read = ofs + len;
  rc = 0;
  while ((0 == rc) && (ofs < end_read)) {
    nb = (end_read - ofs);
    if (sizeof(data) < nb) {
      nb = sizeof(data);
    }
    /* Return is 0 if successful, 4 if offset too large, 3 unknown
     * error. */
    rc = nvmem_read(fileid, nb, ofs, data);
    if (0 == rc) {
      vBSP430consoleDisplayMemory(data, nb, ofs);
      ofs += nb;
    }
  }
  if (0 != rc) {
    cprintf("\nERR: Read returned %u for %u bytes at %u of fileid %u\n",
            rc, nb, ofs, fileid);
  }
  return 0;
}
static sBSP430cliCommand dcmd_nvmem_read = {
  .key = "read",
  .help = HELP_STRING("fileid [len(=128) [ofs(=0)]] # read block from nvmem file"),
  .next = LAST_SUB_COMMAND,
  .handler = iBSP430cliHandlerSimple,
  .param.simple_handler = cmd_nvmem_read
};
#undef LAST_SUB_COMMAND
#define LAST_SUB_COMMAND &dcmd_nvmem_read
#endif /* CMD_NVMEM_READ */

#if (CMD_NVMEM_MAC - 0)
static int
cmd_nvmem_mac (const char * argstr)
{
  int rc;
  unsigned char mac[6];

  /* Could extend this to parse "set {addr}" if you wanted. */
  rc = nvmem_get_mac_address(mac);
  if (0 == rc) {
    cprintf("nvmem mac is %02x:%02x:%02x:%02x:%02x:%02x\n",
            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  } else {
    cprintf("ERR: nvmem mac read got %u\n", rc);
  }
  return 0;
}
static sBSP430cliCommand dcmd_nvmem_mac = {
  .key = "mac",
  .help = HELP_STRING("# get mac address"),
  .next = LAST_SUB_COMMAND,
  .handler = iBSP430cliHandlerSimple,
  .param.simple_handler = cmd_nvmem_mac
};
#undef LAST_SUB_COMMAND
#define LAST_SUB_COMMAND &dcmd_nvmem_mac
#endif /* CMD_NVMEM_MAC */

static sBSP430cliCommand dcmd_nvmem = {
  .key = "nvmem",
  .next = LAST_COMMAND,
  .child = LAST_SUB_COMMAND
};
#undef LAST_COMMAND
#define LAST_COMMAND &dcmd_nvmem
#undef LAST_SUB_COMMAND
#define LAST_SUB_COMMAND NULL
#endif /* CMD_NVMEM */

#if ! (BSP430_MODULE_FLASH - 0)
/* No support for this on FRAM boards yet */
#undef CMD_INFO
#endif

#if (CMD_INFO - 0)

static sConnectParams * const infoConnectParams = (sConnectParams *)__infob;

static int
cmd_info_load (const char * argstr)
{
  if (MAGIC_CONNECT_PARAMS != infoConnectParams->magic) {
    cprintf("ERR: Stored connection params invalid\n");
    return -1;
  }
#if (CMD_WLAN_CONNECT - 0)
  connectParams = *infoConnectParams;
#endif /* CMD_WLAN_CONNECT */
  return 0;
}

static int
cmd_info_erase_ni (void)
{
  int rv;

  cprintf("Erasing stored params at %p...", infoConnectParams);
  rv = iBSP430flashEraseSegment_ni(infoConnectParams);
  cprintf("%d\n", rv);
  return rv;
}

static int
cmd_info_erase (const char * argstr)
{
  BSP430_CORE_SAVED_INTERRUPT_STATE(istate);
  int rv;

  BSP430_CORE_DISABLE_INTERRUPT();
  do {
    rv = cmd_info_erase_ni();
  } while (0);
  BSP430_CORE_RESTORE_INTERRUPT_STATE(istate);
  return rv;
}

static int
cmd_info_store (const char * argstr)
{
  BSP430_CORE_SAVED_INTERRUPT_STATE(istate);
  int rv;

  BSP430_CORE_DISABLE_INTERRUPT();
  do {
    rv = cmd_info_erase_ni();
    if (0 == rv) {
      connectParams.magic = MAGIC_CONNECT_PARAMS;
      cprintf("Copying current configuration...");
      rv = iBSP430flashWriteData_ni(infoConnectParams, &connectParams, sizeof(connectParams));
      cprintf("%d\n", rv);
    }
  } while (0);
  BSP430_CORE_RESTORE_INTERRUPT_STATE(istate);
  return rv;
}

static int
cmd_info_dump (const char * argstr)
{
  cprintf("Configuration memory:\n");
  vBSP430consoleDisplayMemory((const uint8_t *)infoConnectParams, __info_segment_size, (uintptr_t)infoConnectParams);
  return 0;
}

static sBSP430cliCommand dcmd_info_dump = {
  .key = "dump",
  .help = HELP_STRING("# display INFO_B contents"),
  .next = NULL,
  .handler = iBSP430cliHandlerSimple,
  .param.simple_handler = cmd_info_dump
};
static sBSP430cliCommand dcmd_info_store = {
  .key = "store",
  .help = HELP_STRING("# store wlan AP params in INFO_B"),
  .next = &dcmd_info_dump,
  .handler = iBSP430cliHandlerSimple,
  .param.simple_handler = cmd_info_store
};
static sBSP430cliCommand dcmd_info_erase = {
  .key = "erase",
  .help = HELP_STRING("# erase INFO_B"),
  .next = &dcmd_info_store,
  .handler = iBSP430cliHandlerSimple,
  .param.simple_handler = cmd_info_erase
};
static sBSP430cliCommand dcmd_info_load = {
  .key = "load",
  .help = HELP_STRING("# load wlan AP params from INFO_B"),
  .next = &dcmd_info_erase,
  .handler = iBSP430cliHandlerSimple,
  .param.simple_handler = cmd_info_load
};
static sBSP430cliCommand dcmd_info = {
  .key = "info",
  .next = LAST_COMMAND,
  .child = &dcmd_info_load
};
#undef LAST_COMMAND
#define LAST_COMMAND &dcmd_info

#endif /* CMD_INFO */

static int
cmd_mdns (const char * argstr)
{
  size_t argstr_len = strlen(argstr);
  const char * name = "CC3000";
  size_t name_len;
  int rc;

  name = xBSP430cliNextQToken(&argstr, &argstr_len, &name_len);
  if (0 == name_len) {
    rc = mdnsAdvertiser(0, "CC3000", strlen("CC3000"));
  } else {
    int i;
    cprintf("Will advertize: '");
    for (i = 0; i < name_len; ++i) {
      cputchar(name[i]);
    }
    cprintf("'\n");
    rc = mdnsAdvertiser(1, (char *)name, name_len);
  }
  cprintf("mdnsAdvertiser got %d\n", rc);
  return 0;
}
static sBSP430cliCommand dcmd_mdns = {
  .key = "mdns",
  .help = HELP_STRING("[name] # enable/disable mDNS"),
  .next = LAST_COMMAND,
  .handler = iBSP430cliHandlerSimple,
  .param.simple_handler = cmd_mdns
};
#undef LAST_COMMAND
#define LAST_COMMAND &dcmd_mdns

static int
cmd_uptime (const char * argstr)
{
  BSP430_CORE_SAVED_INTERRUPT_STATE(istate);
  unsigned long now_utt;
  char timebuf[BSP430_UPTIME_AS_TEXT_LENGTH];

  BSP430_CORE_DISABLE_INTERRUPT();
  now_utt = ulBSP430uptime_ni();
  BSP430_CORE_RESTORE_INTERRUPT_STATE(istate);
  cprintf("Time: %s\n", xBSP430uptimeAsText(now_utt, timebuf));
  cprintf("Last Event %lx: %s\n", lastEventType, xBSP430uptimeAsText(lastCallback_utt, timebuf));
  cprintf("Last KA: %s\n", xBSP430uptimeAsText(lastKeepAlive_utt, timebuf));
  return 0;
}
static sBSP430cliCommand dcmd_uptime = {
  .key = "uptime",
  .next = LAST_COMMAND,
  .handler = iBSP430cliHandlerSimple,
  .param.simple_handler = cmd_uptime
};
#undef LAST_COMMAND
#define LAST_COMMAND &dcmd_uptime

static int
cmd_ghbn (const char * argstr)
{
  size_t argstr_len = strlen(argstr);
  const char * name;
  size_t name_len;
  uint32_t addr = -1;
  int rc;

  name = xBSP430cliNextQToken(&argstr, &argstr_len, &name_len);
  if (0 == name_len) {
    name = "ntp.pool.org";
    name_len = strlen(name);
  }
  rc = gethostbyname((char*)name, name_len, &addr);
  cprintf("gethostbyname(%s) returned %d, ip: %s\n", name, rc, ipv4AsText((const uint8_t*)&addr));
  return 0;
}
static sBSP430cliCommand dcmd_ghbn = {
  .key = "ghbn",
  .next = LAST_COMMAND,
  .handler = iBSP430cliHandlerSimple,
  .param.simple_handler = cmd_ghbn
};
#undef LAST_COMMAND
#define LAST_COMMAND &dcmd_ghbn

static int
cmd_test (sBSP430cliCommandLink * chain,
          void * param,
          const char * command,
          size_t command_len)
{
  return 0;
}
static sBSP430cliCommand dcmd_test = {
  .key = "test",
  .next = LAST_COMMAND,
  .handler = cmd_test
};
#undef LAST_COMMAND
#define LAST_COMMAND &dcmd_test

#if (CMD_HELP - 0)
static int
cmd_help (sBSP430cliCommandLink * chain,
          void * param,
          const char * command,
          size_t command_len)
{
  vBSP430cliConsoleDisplayHelp(chain->cmd);
  return 0;
}
static sBSP430cliCommand dcmd_help = {
  .key = "help",
  .next = LAST_COMMAND,
  .handler = cmd_help
};
#undef LAST_COMMAND
#define LAST_COMMAND &dcmd_help
#endif /* CMD_HELP */

void main (void)
{
  int rv;
  const char * command;
  int flags;
#if (BSP430_MODULE_SYS - 0)
  unsigned long reset_causes = 0;
  unsigned int reset_flags = 0;
#endif /* BSP430_MODULE_SYS */

#if (BSP430_MODULE_SYS - 0)
  {
    unsigned int sysrstiv;

    /* Record all the reset causes */
    while (0 != ((sysrstiv = uiBSP430sysSYSRSTGenerator_ni(&reset_flags)))) {
      reset_causes |= 1UL << (sysrstiv / 2);
    }
#ifdef BSP430_PMM_ENTER_LPMXp5_NI
    /* If we woke from LPMx.5, we need to clear the lock in PM5CTL0.
     * We'll do it early, since we're not really interested in
     * retaining the current IFG settings. */
    if (reset_flags & BSP430_SYS_FLAG_SYSRST_LPM5WU) {
      PMMCTL0_H = PMMPW_H;
      PM5CTL0 = 0;
      PMMCTL0_H = 0;
    }
#endif /* BSP430_PMM_ENTER_LPMXp5_NI */
  }
#endif /* BSP430_MODULE_SYS */

  vBSP430platformInitialize_ni();
  (void)iBSP430consoleInitialize();
  vBSP430cliSetDiagnosticFunction(iBSP430cliConsoleDiagnostic);
  BSP430_CORE_DELAY_CYCLES(BSP430_CLOCK_NOMINAL_MCLK_HZ / 2);
  cprintf("\ncc3000 " __DATE__ " " __TIME__ "\n");

#if (BSP430_MODULE_SYS - 0)
  cprintf("System reset bitmask %lx; causes:\n", reset_causes);
  {
    int bit = 0;
    while (bit < (8 * sizeof(reset_causes))) {
      if (reset_causes & (1UL << bit)) {
        cprintf("\t%s\n", xBSP430sysSYSRSTIVDescription(2*bit));
      }
      ++bit;
    }
  }

  cputtext("System reset included:");
  if (reset_flags) {
    if (reset_flags & BSP430_SYS_FLAG_SYSRST_BOR) {
      cputtext(" BOR");
    }
    if (reset_flags & BSP430_SYS_FLAG_SYSRST_LPM5WU) {
      cputtext(" LPM5WU");
    }
    if (reset_flags & BSP430_SYS_FLAG_SYSRST_POR) {
      cputtext(" POR");
    }
    if (reset_flags & BSP430_SYS_FLAG_SYSRST_PUC) {
      cputtext(" PUC");
    }
  } else {
    cputtext(" no data");
  }
  cputchar('\n');
#endif /* BSP430_MODULE_SYS */

#ifdef BSP430_RF_CC3000_PWR_EN_PORT_PERIPH_HANDLE
  cprintf("PWR_EN at %s.%u\n", xBSP430portName(BSP430_RF_CC3000_PWR_EN_PORT_PERIPH_HANDLE), iBSP430portBitPosition(BSP430_RF_CC3000_PWR_EN_PORT_BIT));
#else /* BSP430_RF_CC3000_PWR_EN_PORT_PERIPH_HANDLE */
  cprintf("No PWR_EN control.\n");
#endif /* BSP430_RF_CC3000_PWR_EN_PORT_PERIPH_HANDLE */
  cprintf("SPI_IRQ HAL at %s.%u\n", xBSP430portName(BSP430_RF_CC3000_IRQn_PORT_PERIPH_HANDLE), iBSP430portBitPosition(BSP430_RF_CC3000_IRQn_PORT_BIT));
  cprintf("CSn HAL at %s.%u\n", xBSP430portName(BSP430_RF_CC3000_CSn_PORT_PERIPH_HANDLE), iBSP430portBitPosition(BSP430_RF_CC3000_CSn_PORT_BIT));
  cprintf("SPI is %s\n", xBSP430serialName(BSP430_RF_CC3000_SPI_PERIPH_HANDLE));
#if BSP430_PLATFORM_PERIPHERAL_HELP
  cprintf("SPI Pins: %s\n", xBSP430platformPeripheralHelp(BSP430_RF_CC3000_SPI_PERIPH_HANDLE, BSP430_PERIPHCFG_SERIAL_SPI3));
#endif /* BSP430_PLATFORM_PERIPHERAL_HELP */
  cprintf("CC3000 host driver: %u\n", DRIVER_VERSION_NUMBER);
  cprintf("RX buffer size %u, TX buffer size %u\n",
          BSP430_CC3000SPI_RX_BUFFER_SIZE,
          BSP430_CC3000SPI_TX_BUFFER_SIZE);
  cprintf(__DATE__ " " __TIME__ "\n");

  /* Initialization can be done with interrupts disabled, since the
   * function does nothing but store callbacks.  We use the same
   * callback for all three update capabilities. */
  rv = iBSP430cc3000spiInitialize(wlan_cb, NULL, NULL, NULL);
  cprintf("%s Initialize returned %d\n", xBSP430uptimeAsText_ni(ulBSP430uptime_ni()), rv);
  if (0 > rv) {
    cprintf("Aborting due to failure to initialized\n");
    return;
  }

  commandSet = LAST_COMMAND;
  command = NULL;
  flags = eBSP430cliConsole_REPAINT;

  BSP430_CORE_ENABLE_INTERRUPT();

  /* wlan_start() MUST be invoked with interrupts enabled.  Pass 0 for
   * a normal startup.  If the firmware is suspect, you need to pass 2
   * which will cause the NVMEM firmware sections to be ignored. */
#if (BYPASS_SP_LOAD - 0)
  cprintf("Bypassing service packs\n");
  wlan_start(2);
#else
  wlan_start(0);
#endif

  cprintf("\nAnd wlan has been started.\n");
  (void)vBSP430consoleDisplayMemory;
  rv = wlan_set_event_mask(0);
  cprintf("Unmask events returned %d\n", rv);

#if (CMD_INFO - 0)
  if (0 == cmd_info_load("")) {
    cmd_wlan_status("");
  }
#endif

  while (1) {
    if (flags & eBSP430cliConsole_DO_COMPLETION) {
      flags &= ~eBSP430cliConsole_DO_COMPLETION;
      flags |= iBSP430cliConsoleBufferCompletion(commandSet, &command);
    }
    if (flags & eBSP430cliConsole_READY) {
      int rv;

      rv = iBSP430cliExecuteCommand(commandSet, 0, command);
      if (0 != rv) {
        cprintf("Command execution returned %d\n", rv);
      }
      /* Ensure prompt is rewritten, but not the command we just
       * ran */
      flags |= eBSP430cliConsole_REPAINT;
      command = NULL;
    }
    if (flags & eBSP430cliConsole_REPAINT) {
      /* Draw the prompt along with whatever's left in the command buffer */
      cprintf("\r> %s", command ? command : "");
      flags &= ~eBSP430cliConsole_REPAINT;
    }
    if (flags & eBSP430cliConsole_REPAINT_BEL) {
      cputchar('\a');
      flags &= ~eBSP430cliConsole_REPAINT_BEL;
    }
    if (flags & eBSP430cliConsole_READY) {
      /* Clear the command we just completed */
      vBSP430cliConsoleBufferClear();
      flags &= ~eBSP430cliConsole_READY;
    }
    do {
      /* Discard any pending escape sequence */
      flags = iBSP430cliConsoleBufferConsumeEscape(flags);
      /* If no operations are pending, see if there is pending input. */
      if (0 == flags) {
        flags = iBSP430cliConsoleBufferProcessInput();
      }
      /* If still no operations pending wait for something to
       * happen. */
      if (0 == flags) {
        BSP430_CORE_LPM_ENTER(LPM2_bits);
      }
    } while (! flags);

    /* Got something to do; get the command contents in place so
     * we can update the screen. */
    command = xBSP430cliConsoleBuffer();
  }
}
