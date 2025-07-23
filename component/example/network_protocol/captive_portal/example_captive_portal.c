#include "example_captive_portal.h"
#include "wifi_api.h"
#include <rtk_bt_common.h>
#include <rtk_bt_device.h>
#include <rtk_bt_le_gap.h>
#include <rtk_bt_gatts.h>
#include <bt_utils.h>
#include "../../../bluetooth/example/gatt_service/server/rtk_service_config.h"
#include "../../../bluetooth/example/gatt_service/server/rtk_simple_ble_service.h"
#include "../../../bluetooth/example/gatt_service/server/rtk_hids_kb.h"
/* ------------------------ Defines --------------------------------------- */
/* The size of the buffer in which the dynamic WEB page is created. */
#define webMAX_PAGE_SIZE		(6144/* 2800 */) /*FSL: buffer containing array*/
#define LOCAL_BUF_SIZE			1024
/* Standard GET response. */
#define webHTTP_OK  "HTTP/1.0 200 OK\r\nContent-type: text/html\r\n\r\n"

/* The port on which we listen. */
#define webHTTP_PORT			( 80 )

/* Delay on close error. */
#define webSHORT_DELAY			( 10 )

#define USE_DIV_CSS 1

#if USE_DIV_CSS

/* Format of the dynamic page that is returned on each connection. */
#define webHTML_HEAD_START \
"<html>\
<head>\
"
/*
<meta http-equiv=\"Content-Type\" content=\"text/html;charset=gb2312>\
<meta http-equiv=\"Cache-Control\" CONTENT=\"no-cache\">\
<meta http-equiv=\"Expires\" CONTENT=\"0\">\
*/

#define webHTML_TITLE \
"<title>Realtek SoftAP Config UI</title>"

#define webHTML_BODY_START \
"</head>\
<body  onLoad=\"onChangeSecType()\">\
<form name=\"form1\" method=\"post\" onSubmit=\"return onSubmitForm()\" accept-charset=\"utf-8\">\
<div class=\"wrapper\">\
<div class=\"header\">\
Realtek SoftAP Configuration\
</div>"

#define webHTML_CSS \
"<style>\
body {\
text-align:center;\
font-family: 'Segoe UI';\
}\
.wrapper {\
text-align:left;\
margin:0 auto;\
margin-top:200px;\
border:#000;\
width:500px;\
}\
.header {\
background-color:#CF9;\
font-size:18px;\
line-height:50px;\
text-align:center;\
}\
.oneline {\
width:100%;\
border-left:#FC3 10px;\
font-size:15px;\
height:30px;\
margin-top:3px;\
}\
.left {\
background-color:#FF0;\
line-height:30px;\
width:40%;\
height:100%;\
float:left;\
padding-left:20px;\
}\
.right {\
margin-left:20px;\
}\
\
.box {\
width:40%;\
height:28px;\
margin-left:20px;\
\
}\
\
.btn {\
background-color:#CF9;\
height:40px;\
text-align:center;\
}\
\
.btn input {\
font-size:16px;\
height:30px;\
width:150px;\
border:0px;\
line-height:30px;\
margin-top:5px;\
border-radius:20px;\
color:#000;\
}\
.btn input:hover{\
cursor:pointer;\
background-color:#FB4044;\
}\
\
.foot {\
text-align:center;\
font-size:15px;\
line-height:20px;\
border:#CCC;\
}\
#pwd {\
display:none;\
}\
</style>"

#define webHTML_SUBMIT_BTN	\
" <div class=\"oneline btn\">\
<input  type=\"submit\" value=\"Submit\" style=\"background-color: #FFF;\">\
</div>\
"

#define webHTML_STA_BOBY_START	\
" <div class=\"header\">\
Realtek STA WIFI Configuration\
</div>\
"

#define webHTML_SCAN_BTN \
" <div class=\"oneline btn\">\
<input type=\"submit\" name=\"scan\" value=\"Scan\" style=\"background-color: #FFF;\">\
<input type=\"button\" name=\"connect\" value=\"Connect\" style=\"background-color: #FFF;\" onClick=\"onChangeWifiStatus()\">\
<input type=\"submit\" name=\"refresh\" value=\"Refresh\" style=\"background-color: #FFF;\">\
</div>\
"

#define webHTML_PASSWORD	\
"<div class=\"oneline\" id=\"spwd\"><div class=\"left\">Password: </div>\
<div class=\"right\" >\
<input  class=\"box\" id=\"spwd_val\" type=\"text\" name=\"SPassword\" value=\"""\" >\
 </div></div>\
"

#define webHTML_END \
" <div class=\"oneline foot\">\
Copyright &copy;realtek.com\
</div>\
 </div>\
 </form>\
</body>\
</html>\
"

#define webWaitHTML_START \
"<html location.href='wait.html'>\
<head>\
"
#define webWaitHTML_END \
"</head>\
<BODY>\
<p>\
<h2>SoftAP is now restarting!</h2>\
<h2>Please wait a moment and reconnect!</h2>\
</p>"\
"</BODY>\r\n" \
"</html>"

#define onChangeSecType \
"<script>\
function onChangeSecType()\
{\
x=document.getElementById(\"sec\");\
y=document.getElementById(\"pwd\");\
if(x.value == \"open\"){\
y.style.display=\"none\";\
}else{\
y.style.display=\"block\";\
}\
}\
</script>"

#define onSubmitForm \
"<script>\
function onSubmitForm()\
{\
x=document.getElementById(\"Ssid\");\
y=document.getElementById(\"pwd\");\
z=document.getElementById(\"pwd_val\");\
if(x.value.length>32)\
{\
alert(\"SoftAP SSID is too long!(1-32)\");\
return false;\
}\
/*if(!(/^[A-Za-z0-9]+$/.test(x.value)))\
{\
alert(\"SoftAP SSID can only be [A-Za-z0-9]\");\
return false;\
}*/\
if(y.style.display == \"block\")\
{\
if((z.value.length < 8)||(z.value.length>64))\
{\
alert(\"Password length is between 8 to 64\");\
return false;\
}\
}\
}\
</script>"

#define onChangeColor \
"<script>\
function onChangeColor() {\
elementbtn = document.getElementById(\"led_btn\");\
elementlbl = document.getElementById(\"led_val\");\
if(elementbtn.value=='OFF')\
{\
elementbtn.value='ON';\
elementlbl.value='ON';\
elementbtn.style.backgroundColor = \"#00FF00\";\
}\
else\
{\
elementbtn.value='OFF';\
elementlbl.value='OFF';\
elementbtn.style.backgroundColor = \"#C0C0C0\";\
}\
document.form1.submit();\
}\
</script>\
"

#define onChangeWifiStatus \
"<script>\
function onChangeWifiStatus() {\
targetap = document.getElementById(\"sap_val\");\
if(targetap.value==\"""\")\
{\
alert(\"TargetAP is NULL!\");\
}\
else\
{\
elementlbl = document.getElementById(\"sconn_val\");\
elementlbl.value='Connecting';\
document.form1.submit();\
}\
}\
</script>\
"

#else

/* Format of the dynamic page that is returned on each connection. */
#define webHTML_HEAD_START \
"<html>\
<head>\
"
/*
<meta http-equiv=\"Content-Type\" content=\"text/html;charset=gb2312>\
<meta http-equiv=\"Cache-Control\" CONTENT=\"no-cache\">\
<meta http-equiv=\"Expires\" CONTENT=\"0\">\
*/

#define webHTML_BODY_START \
"</head>\
<BODY onLoad=\"onChangeSecType()\">\
\r\n\r\n<form name=\"form\" method=\"post\" onsubmit=\"return onSubmitForm()\" accept-charset=\"utf-8\">\
<table width=\"500\">\
<tr>\
<td colspan=\"2\" style=\"background-color:#FFA500;text-align:center;\">\
<h2>Realtek SoftAP Configuration</h2>\
</td>\
</tr>"

#define webHTML_END \
"<tr>\
<td colspan=\"2\" style=\"background-color:#FFD700;text-align:center;height:40px\">\
<input type=\"submit\" value=\"Submit\"><br></td>\
</tr>\
<tr>\
<td colspan=\"2\" style=\"background-color:#FFA500;text-align:center;\">\
Copyright ?realtek.com</td>\
</tr>\
</table>\
\r\n</form>" \
"</BODY>\r\n" \
"</html>"

#define webWaitHTML_START \
"<html location.href='wait.html'>\
<head>\
"
#define webWaitHTML_END \
"</head>\
<BODY>\
<p>\
<h2>SoftAP is now restarting!</h2>\
<h2>Please wait a moment and reconnect!</h2>\
</p>"\
"</BODY>\r\n" \
"</html>"

#define onChangeSecType \
"<script>\
function onChangeSecType()\
{\
x=document.getElementById(\"sec\");\
y=document.getElementById(\"pwd_row\");\
if(x.value == \"open\"){\
y.style.display=\"none\";\
}else{\
y.style.display=\"block\";\
}\
}\
</script>"

#define onSubmitForm \
"<script>\
function onSubmitForm()\
{\
x=document.getElementById(\"Ssid\");\
y=document.getElementById(\"pwd_row\");\
z=document.getElementById(\"pwd\");\
if(x.value.length>32)\
{\
alert(\"SoftAP SSID is too long!(1-32)\");\
return false;\
}\
if(!(/^[A-Za-z0-9]+$/.test(x.value)))\
{\
alert(\"SoftAP SSID can only be [A-Za-z0-9]\");\
return false;\
}\
if(y.style.display == \"block\")\
{\
if((z.value.length < 8)||(z.value.length>64))\
{\
alert(\"Password length is between 8 to 64\");\
return false;\
}\
}\
}\
</script>"

#endif


/*
alert(\"Please enter your password!\");\
return false;\
}\
if(z.value.length < 8)\
{\
alert(\"Your password is too short!(8-64)\");\
return false;\
}\
if(z.value.length>64)\
{\
alert(\"Your password is too long!(8-64)\");\
*/

#define MAX_SOFTAP_SSID_LEN		32
#define MAX_PASSWORD_LEN		64
#define MAX_CHANNEL_NUM			13

struct rtw_wifi_setting Wifi_Setting = {RTW_MODE_NONE, {0}, {0}, 0, RTW_SECURITY_OPEN, {0}, 0, 0, 0, 0, 0};
struct rtw_wifi_setting target_ap_setting = {RTW_MODE_NONE, {0}, {0}, 0, RTW_SECURITY_OPEN, {0}, 0, 0, 0, 0, 0};

static void vProcessConnection(void *param);

/*------------------------------------------------------------------------------*/
extern s32 wifi_get_setting(u8 wlan_idx, struct rtw_wifi_setting *psetting);
static void LoadWifiSetting(void)
{
	u8 wlan_idx = STA_WLAN_INDEX;

	if (wifi_is_running(SOFTAP_WLAN_INDEX)) {
		//STA_AP_MODE
		wlan_idx = SOFTAP_WLAN_INDEX;
	}

	wifi_get_setting(wlan_idx, &Wifi_Setting);

	//RTK_LOGI(NOTAG, "LoadWifiSetting(): Wifi_Setting.ssid=%s\n", Wifi_Setting.ssid);
	//RTK_LOGI(NOTAG, "LoadWifiSetting(): Wifi_Setting.channel=%d\n", Wifi_Setting.channel);
	//RTK_LOGI(NOTAG, "LoadWifiSetting(): Wifi_Setting.security_type=%d\n", Wifi_Setting.security_type);
	//RTK_LOGI(NOTAG, "LoadWifiSetting(): Wifi_Setting.password=%s\n", Wifi_Setting.password);
}

#if CONFIG_READ_FLASH
static const char *key = "ap_data";

void LoadWifiConfig(void)
{
	struct rtw_wifi_config local_config;
	int ret;

	RTK_LOGI(NOTAG, "LoadWifiConfig(): Read from FLASH!\n");

	ret = rt_kv_get(key, (uint8_t *)&local_config, sizeof(struct rtw_wifi_config));


	RTK_LOGI(NOTAG, "LoadWifiConfig(): local_config.boot_mode=0x%x\n", local_config.boot_mode);
	RTK_LOGI(NOTAG, "LoadWifiConfig(): local_config.ssid=%s\n", local_config.ssid);
	RTK_LOGI(NOTAG, "LoadWifiConfig(): local_config.channel=%d\n", local_config.channel);
	RTK_LOGI(NOTAG, "LoadWifiConfig(): local_config.security_type=%d\n", local_config.security_type);
	RTK_LOGI(NOTAG, "LoadWifiConfig(): local_config.password=%s\n", local_config.password);

	if (local_config.boot_mode == 0x77665502) {
		Wifi_Setting.mode = RTW_MODE_AP;
		if (local_config.ssid_len > 32) {
			local_config.ssid_len = 32;
		}
		memcpy(Wifi_Setting.ssid, local_config.ssid, local_config.ssid_len);
		Wifi_Setting.ssid[local_config.ssid_len] = '\0';
		Wifi_Setting.channel = local_config.channel;
		if (local_config.security_type == 1) {
			Wifi_Setting.security_type = RTW_SECURITY_WPA2_AES_PSK;
		} else {
			Wifi_Setting.security_type = RTW_SECURITY_OPEN;
		}
		if (local_config.password_len > 64) {
			local_config.password_len = 64;
		}
		memcpy(Wifi_Setting.password, local_config.password, local_config.password_len);
		Wifi_Setting.password[local_config.password_len] = '\0';
	} else {
		LoadWifiSetting();
	}

}

int StoreApInfo(void)
{
	struct rtw_wifi_config wifi_config;

	// clean wifi_config first
	memset(&wifi_config, 0x00, sizeof(struct rtw_wifi_config));


	wifi_config.boot_mode = 0x77665502;
	memcpy(wifi_config.ssid, Wifi_Setting.ssid, strlen((char *)Wifi_Setting.ssid));
	wifi_config.ssid_len = strlen((char *)Wifi_Setting.ssid);
	wifi_config.security_type = Wifi_Setting.security_type;
	if (Wifi_Setting.security_type != 0) {
		wifi_config.security_type = 1;
	} else {
		wifi_config.security_type = 0;
	}
	memcpy(wifi_config.password, Wifi_Setting.password, strlen((char *)Wifi_Setting.password));
	wifi_config.password_len = strlen((char *)Wifi_Setting.password);
	wifi_config.channel = Wifi_Setting.channel;
	RTK_LOGI(NOTAG, "Writting boot mode 0x77665502 and Wi-Fi setting to flash ...\n");

	rt_kv_set(key, (uint8_t *)&wifi_config, sizeof(struct rtw_wifi_config));

	return 0;
}

int EraseApinfo(void)
{
	rt_kv_delete(key);
	return 0;
}
#endif

extern void dhcps_init(struct netif *pnetif);
extern void dhcps_deinit(void);
int wifi_restart_ap(struct rtw_softap_info *softAP_config)
{
	unsigned char idx = 0;
#ifdef CONFIG_LWIP_LAYER
	u32 ip_addr;
	u32 netmask;
	u32 gw;
#endif

#ifdef  CONFIG_CONCURRENT_MODE
	struct rtw_wifi_setting setting;
	int sta_linked = 0;
	struct rtw_network_info connect_param = {0};
#endif

	if (wifi_is_running(SOFTAP_WLAN_INDEX)) {
		idx = 1;
	}

	// stop dhcp server
#ifdef CONFIG_LWIP_LAYER
	dhcps_deinit();
#endif

#ifdef  CONFIG_CONCURRENT_MODE
	if (idx > 0) {
		sta_linked = wifi_get_setting(STA_WLAN_INDEX, &setting);
		rtos_time_delay_ms(20);
		wifi_on(RTW_MODE_STA);
	} else
#endif
	{
#ifdef CONFIG_LWIP_LAYER
		dhcps_deinit();
		ip_addr = CONCAT_TO_UINT32(AP_IP_ADDR0, AP_IP_ADDR1, AP_IP_ADDR2, AP_IP_ADDR3);
		netmask = CONCAT_TO_UINT32(AP_NETMASK_ADDR0, AP_NETMASK_ADDR1, AP_NETMASK_ADDR2, AP_NETMASK_ADDR3);
		gw = CONCAT_TO_UINT32(AP_GW_ADDR0, AP_GW_ADDR1, AP_GW_ADDR2, AP_GW_ADDR3);
		LwIP_SetIP(SOFTAP_WLAN_INDEX, ip_addr, netmask, gw);
#endif
		wifi_stop_ap();
		rtos_time_delay_ms(20);
		wifi_on(RTW_MODE_STA);
	}
	// start ap
	if (wifi_start_ap(softAP_config) < 0) {
		RTK_LOGE(NOTAG, "ERROR: Operation failed!");
		return -1;
	}

#ifdef  CONFIG_CONCURRENT_MODE
	// connect to ap if wlan0 was linked with ap
	if (idx > 0 && sta_linked == 0) {
		volatile int ret;
		(void) ret;
		RTK_LOGI(NOTAG, "AP: ssid=%s\n", (char *)setting.ssid);
		RTK_LOGI(NOTAG, "AP: security_type=%d\n", setting.security_type);
		RTK_LOGI(NOTAG, "AP: password=%s\n", (char *)setting.password);
		RTK_LOGI(NOTAG, "AP: key_idx =%d\n", setting.key_idx);
		memcpy(connect_param.ssid.val, setting.ssid, strlen((char *)setting.ssid));
		connect_param.ssid.len = strlen((char *)setting.ssid);
		connect_param.password = setting.password;
		connect_param.password_len = strlen((char *)setting.password);
		connect_param.security_type = setting.security_type;
		connect_param.key_id = setting.key_idx;
		ret = wifi_connect(&connect_param, 1);
#if defined(CONFIG_DHCP_CLIENT) && CONFIG_DHCP_CLIENT
		if (ret == RTK_SUCCESS) {
			/* Start DHCPClient */
			LwIP_DHCP(0, DHCP_START);
		}
#endif
	}
#endif

#ifdef CONFIG_LWIP_LAYER
	// start dhcp server
	dhcps_init(&xnetif[idx]);
#endif

	return 0;
}

static void RestartSoftAP(void)
{
	RTK_LOGI(NOTAG, "RestartAP: ssid=%s\n", Wifi_Setting.ssid);
	RTK_LOGI(NOTAG, "RestartAP: ssid_len=%d\n", strlen((char *)Wifi_Setting.ssid));
	RTK_LOGI(NOTAG, "RestartAP: security_type=%d\n", Wifi_Setting.security_type);
	RTK_LOGI(NOTAG, "RestartAP: password=%s\n", Wifi_Setting.password);
	RTK_LOGI(NOTAG, "RestartAP: channel=%d\n", Wifi_Setting.channel);
	struct rtw_softap_info softAP_config = {0};
	softAP_config.ssid.len = strlen((char *)Wifi_Setting.ssid);
	memcpy(softAP_config.ssid.val, Wifi_Setting.ssid, softAP_config.ssid.len);
	softAP_config.password = Wifi_Setting.password;
	softAP_config.password_len = strlen((char *)Wifi_Setting.password);
	softAP_config.security_type = Wifi_Setting.security_type;
	softAP_config.channel = Wifi_Setting.channel;
	wifi_restart_ap(&softAP_config);
}

static u32 web_atoi(char *s)
{
	int num = 0, flag = 0;
	u32 i;

	for (i = 0; i <= strlen(s); i++) {
		if (s[i] >= '0' && s[i] <= '9') {
			num = num * 10 + s[i] - '0';
		} else if (s[0] == '-' && i == 0) {
			flag = 1;
		} else {
			break;
		}
	}

	if (flag == 1) {
		num = num * -1;
	}

	return (num);
}

static void CreateSsidTableItem(char *pbuf, u8_t *ssid, u8_t ssid_len)
{
	char local_ssid[MAX_SOFTAP_SSID_LEN + 1];

	if (ssid_len > MAX_SOFTAP_SSID_LEN) {
		ssid_len = MAX_SOFTAP_SSID_LEN;
	}
	memcpy(local_ssid, ssid, ssid_len);
	local_ssid[ssid_len] = '\0';

#if USE_DIV_CSS
	sprintf(pbuf, "<div class=\"oneline\"><div class=\"left\">SoftAP SSID:</div> <div class=\"right\">" \
			"<input class=\"box\" type=\"text\" name=\"Ssid\" id=\"Ssid\" value=\"%s\"></div></div>",
			local_ssid);
#else
	sprintf(pbuf, "<tr>"
			"<td style=\"background-color:#FFD700;width:100px;\">"
			"<b>SoftAP SSID:</b><br>"
			"</td>"
			"<td style=\"background-color:#eeeeee;height:30px;width:400px;\">"
			"<input type=\"text\" name=\"Ssid\" id=\"Ssid\" value=\"%s\"><br>"
			"</td>"
			"</tr>",
			local_ssid);

#endif
	//RTK_LOGI(NOTAG, "strlen(SsidTableItem)=%d\n", strlen(pbuf));
}

static void CreateSecTypeTableItem(char *pbuf, u32_t sectype)
{
	u8_t flag[2] = {0, 0};

	if (sectype == RTW_SECURITY_OPEN) {
		flag[0] = 1;
	} else if (sectype == RTW_SECURITY_WPA2_AES_PSK) {
		flag[1] = 1;
	} else {
		return;
	}

#if USE_DIV_CSS
	sprintf(pbuf, "<div class=\"oneline\"><div class=\"left\">Security Type: </div><div class=\"right\">"\
			"<select  class=\"box\" name=\"Security Type\"  id=\"sec\" onChange=onChangeSecType()>"\
			"<option value=\"open\" %s>OPEN</option><option value=\"wpa2-aes\" %s>WPA2-AES</option>"\
			"</select></div></div>",
			flag[0] ? "selected" : "",
			flag[1] ? "selected" : "");
#else
	sprintf(pbuf, "<tr>"
			"<td style=\"background-color:#FFD700;width:100px;\">"
			"<b>Security Type:</b><br>"
			"</td>"
			"<td style=\"background-color:#eeeeee;height:30px;\">"
			"<select name=\"Security Type\"  id=\"sec\" onChange=onChangeSecType()>"
			"<option value=\"open\" %s>OPEN</option>"
			"<option value=\"wpa2-aes\" %s>WPA2-AES</option>"
			"</select>"
			"</td>"
			"</tr>",
			flag[0] ? "selected" : "",
			flag[1] ? "selected" : "");
#endif

	//RTK_LOGI(NOTAG, "strlen(SecTypeTableItem)=%d\n", strlen(pbuf));
}

static void CreatePasswdTableItem(char *pbuf, u8_t *password, u8_t passwd_len)
{
	char local_passwd[MAX_PASSWORD_LEN + 1];

	if (passwd_len > MAX_PASSWORD_LEN) {
		passwd_len = MAX_PASSWORD_LEN;
	}
	if (passwd_len > 0) {
		memcpy(local_passwd, password, passwd_len);
		local_passwd[passwd_len] = '\0';
	}

#if USE_DIV_CSS

	sprintf(pbuf,	"<div class=\"oneline\" id=\"pwd\"><div class=\"left\">Password: </div>"\
			"<div class=\"right\" >"\
			"<input  class=\"box\" id=\"pwd_val\" type=\"text\" name=\"Password\" value=\"%s\" >"\
			" </div></div>",
			passwd_len ? local_passwd : "");
#else
	sprintf(pbuf, "<tr id=\"pwd_row\">"
			"<td style=\"background-color:#FFD700;width:100px;\">"
			"<b>Password:</b><br>"
			"</td>"
			"<td style=\"background-color:#eeeeee;height:30px;\">"
			"<input type=\"text\" name=\"Password\" id=\"pwd\" value=\"%s\" ><br>"
			"</td>"
			"</tr>",
			passwd_len ? local_passwd : "");
#endif
	//RTK_LOGI(NOTAG, "strlen(passwordTableItem)=%d\n", strlen(pbuf));
}

static void CreateChannelTableItem(char *pbuf, u8_t channel)
{
	u8_t flag[MAX_CHANNEL_NUM + 1] = {0};

	if (channel > MAX_CHANNEL_NUM) {
		RTK_LOGE(NOTAG, "Channel(%d) is out of range!\n", channel);
		channel = 1;
	}
	flag[channel] = 1;

#if USE_DIV_CSS

	sprintf(pbuf, "<div class=\"oneline\"><div class=\"left\">Channel: </div>"
			"<div class=\"right\"><select  class=\"box\" name=\"Channel\">"
			"<option value=\"1\" %s>1</option>"
			"<option value=\"2\" %s>2</option>"
			"<option value=\"3\" %s>3</option>"
			"<option value=\"4\" %s>4</option>"
			"<option value=\"5\" %s>5</option>"
			"<option value=\"6\" %s>6</option>"
			"<option value=\"7\" %s>7</option>"
			"<option value=\"8\" %s>8</option>"
			"<option value=\"9\" %s>9</option>"
			"<option value=\"10\" %s>10</option>"
			"<option value=\"11\" %s>11</option>"
			"</select> </div> </div>",

			flag[1] ? "selected" : "",
			flag[2] ? "selected" : "",
			flag[3] ? "selected" : "",
			flag[4] ? "selected" : "",
			flag[5] ? "selected" : "",
			flag[6] ? "selected" : "",
			flag[7] ? "selected" : "",
			flag[8] ? "selected" : "",
			flag[9] ? "selected" : "",
			flag[10] ? "selected" : "",
			flag[11] ? "selected" : "");
#else
	sprintf(pbuf, "<tr>"
			"<td style=\"background-color:#FFD700;width:100px;\">"
			"<b>Channel:</b><br>"
			"</td>"
			"<td style=\"background-color:#eeeeee;height:30px;\">"
			"<select name=\"Channel\">"
			"<option value=\"1\" %s>1</option>"
			"<option value=\"2\" %s>2</option>"
			"<option value=\"3\" %s>3</option>"
			"<option value=\"4\" %s>4</option>"
			"<option value=\"5\" %s>5</option>"
			"<option value=\"6\" %s>6</option>"
			"<option value=\"7\" %s>7</option>"
			"<option value=\"8\" %s>8</option>"
			"<option value=\"9\" %s>9</option>"
			"<option value=\"10\" %s>10</option>"
			"<option value=\"11\" %s>11</option>"
			"</select>"
			"</td>"
			"</tr>",
			flag[1] ? "selected" : "",
			flag[2] ? "selected" : "",
			flag[3] ? "selected" : "",
			flag[4] ? "selected" : "",
			flag[5] ? "selected" : "",
			flag[6] ? "selected" : "",
			flag[7] ? "selected" : "",
			flag[8] ? "selected" : "",
			flag[9] ? "selected" : "",
			flag[10] ? "selected" : "",
			flag[11] ? "selected" : "");
#endif

	//RTK_LOGI(NOTAG, "strlen(ChannelTableItem)=%d\n", strlen(pbuf));
}

static void CreateTargetAPTableItem(char *pbuf)
{
	char local_passwd[MAX_PASSWORD_LEN + 1];
	char local_ssid[MAX_SOFTAP_SSID_LEN + 1];
	u8_t passwd_len, ssid_len;

	passwd_len = strlen((char *)target_ap_setting.password);
	ssid_len = strlen((char *)target_ap_setting.ssid);

	if (ssid_len > MAX_SOFTAP_SSID_LEN) {
		ssid_len = MAX_SOFTAP_SSID_LEN;
	}
	memcpy(local_ssid, target_ap_setting.ssid, ssid_len);
	local_ssid[ssid_len] = '\0';

	if (passwd_len > MAX_PASSWORD_LEN) {
		passwd_len = MAX_PASSWORD_LEN;
	}
	if (passwd_len > 0) {
		memcpy(local_passwd, target_ap_setting.password, passwd_len);
		local_passwd[passwd_len] = '\0';
	}

#if USE_DIV_CSS
	sprintf(pbuf, "<div class=\"oneline\" id=\"sap\"><div class=\"left\">TargetAP: </div>"\
			"<div class=\"right\" ><input  class=\"box\" id=\"sap_val\" type=\"text\" name=\"TargetAP\" value=\"%s\" >"\
			" </div></div>"\
			"<div class=\"oneline\" id=\"spwd\"><div class=\"left\">Password: </div>"\
			"<div class=\"right\" ><input  class=\"box\" id=\"spwd_val\" type=\"text\" name=\"SPassword\" value=\"%s\" >"\
			" </div></div>",
			ssid_len ? local_ssid : "", passwd_len ? local_passwd : "");

#else
	sprintf(pbuf, "<tr id=\"pwd_row\">"
			"<td style=\"background-color:#FFD700;width:100px;\">"
			"<b>Password:</b><br>"
			"</td>"
			"<td style=\"background-color:#eeeeee;height:30px;\">"
			"<input type=\"text\" name=\"Password\" id=\"pwd\" value=\"%s\" ><br>"
			"</td>"
			"</tr>",
			passwd_len ? local_passwd : "");
#endif
	//RTK_LOGI(NOTAG, "strlen(passwordTableItem)=%d\n", strlen(pbuf));
}

static void CreateConnStatusTableItem(char *pbuf)
{
	u8 connect_status = RTW_JOINSTATUS_UNKNOWN;
	wifi_get_join_status(&connect_status);

#if USE_DIV_CSS
	sprintf(pbuf, "<div class=\"oneline\" id=\"sconn\"><div class=\"left\">CONNECTION STATUS: </div>"\
			"<div class=\"right\" ><input  class=\"box\" id=\"sconn_val\" type=\"text\" name=\"CONNSTATUS\" value=\"%s\" >"\
			" </div></div>",
			(connect_status != RTW_JOINSTATUS_SUCCESS) ? "connect fail" : "connect success");

#else

#endif
	//RTK_LOGI(NOTAG, "strlen(passwordTableItem)=%d\n", strlen(pbuf));
}


static void CreateScanSsidTableItem(char *pbuf)
{
	u8_t flag[MAX_CHANNEL_NUM + 1] = {0};
	int ret;

	flag[0] = 1;
	flag[1] = 0;

#if USE_DIV_CSS

	ret = DiagSnPrintf(pbuf, LOCAL_BUF_SIZE, "<div class=\"oneline\"><div class=\"left\">APList: </div>"\
					   "<div class=\"right\"><select  class=\"box\" name=\"APList\">"\
					   "<option value=\"%s\" %s>%s</option>"\
					   "<option value=\"%s\" %s>%s</option>"\
					   "<option value=\"%s\" %s>%s</option>"\
					   "<option value=\"%s\" %s>%s</option>"\
					   "<option value=\"%s\" %s>%s</option>"\
					   "<option value=\"%s\" %s>%s</option>"\
					   "<option value=\"%s\" %s>%s</option>"\
					   "<option value=\"%s\" %s>%s</option>"\
					   "<option value=\"%s\" %s>%s</option>"\
					   "<option value=\"%s\" %s>%s</option>"\
					   "</select> </div> </div>",
					   scan_result.ap_list[0].ssid, flag[0] ? "selected" : "", scan_result.ap_list[0].ssid,
					   scan_result.ap_list[1].ssid, flag[1] ? "selected" : "", scan_result.ap_list[1].ssid,
					   scan_result.ap_list[2].ssid, flag[1] ? "selected" : "", scan_result.ap_list[2].ssid,
					   scan_result.ap_list[3].ssid, flag[1] ? "selected" : "", scan_result.ap_list[3].ssid,
					   scan_result.ap_list[4].ssid, flag[1] ? "selected" : "", scan_result.ap_list[4].ssid,
					   scan_result.ap_list[5].ssid, flag[1] ? "selected" : "", scan_result.ap_list[5].ssid,
					   scan_result.ap_list[6].ssid, flag[1] ? "selected" : "", scan_result.ap_list[6].ssid,
					   scan_result.ap_list[7].ssid, flag[1] ? "selected" : "", scan_result.ap_list[7].ssid,
					   scan_result.ap_list[8].ssid, flag[1] ? "selected" : "", scan_result.ap_list[8].ssid,
					   scan_result.ap_list[9].ssid, flag[1] ? "selected" : "", scan_result.ap_list[9].ssid
					  );
#else
	ret = sprintf(pbuf, LOCAL_BUF_SIZE, "<tr>"
				  "<td style=\"background-color:#FFD700;width:100px;\">"
				  "<b>Channel:</b><br>"
				  "</td>"
				  "<td style=\"background-color:#eeeeee;height:30px;\">"
				  "<select name=\"Channel\">"
				  "<option value=\"1\" %s>1</option>"
				  "<option value=\"2\" %s>2</option>"
				  "<option value=\"3\" %s>3</option>"
				  "<option value=\"4\" %s>4</option>"
				  "<option value=\"5\" %s>5</option>"
				  "<option value=\"6\" %s>6</option>"
				  "<option value=\"7\" %s>7</option>"
				  "<option value=\"8\" %s>8</option>"
				  "<option value=\"9\" %s>9</option>"
				  "<option value=\"10\" %s>10</option>"
				  "<option value=\"11\" %s>11</option>"
				  "</select>"
				  "</td>"
				  "</tr>",
				  flag[1] ? "selected" : "",
				  flag[2] ? "selected" : "",
				  flag[3] ? "selected" : "",
				  flag[4] ? "selected" : "",
				  flag[5] ? "selected" : "",
				  flag[6] ? "selected" : "",
				  flag[7] ? "selected" : "",
				  flag[8] ? "selected" : "",
				  flag[9] ? "selected" : "",
				  flag[10] ? "selected" : "",
				  flag[11] ? "selected" : "");
#endif

	if (ret < 0) {
		RTK_LOGE(NOTAG, "%s error\n", __func__);
	}
	//RTK_LOGI(NOTAG, "strlen(ChannelTableItem)=%d\n", strlen(pbuf));
}


static void GenerateIndexHtmlPage(char *cDynamicPage, char *LocalBuf)
{
	while (rtos_sema_take(webs_wpage_sema, RTOS_MAX_DELAY) != RTK_SUCCESS);
	/* Generate the page index.html...
	... First the page header. */
	strcpy(cDynamicPage, webHTML_HEAD_START);

	/* Add script */
	strcat(cDynamicPage, onChangeSecType);
	strcat(cDynamicPage, onSubmitForm);
#if USE_DIV_CSS
	strcat(cDynamicPage, onChangeColor);
	strcat(cDynamicPage, onChangeWifiStatus);
#endif
#if USE_DIV_CSS
	/* add css */
	strcat(cDynamicPage, webHTML_CSS);

	strcat(cDynamicPage, webHTML_TITLE);
#endif
	/* Add Body start */
	strcat(cDynamicPage, webHTML_BODY_START);

	/* Add SSID */
	CreateSsidTableItem(LocalBuf, Wifi_Setting.ssid, strlen((char *)Wifi_Setting.ssid));
	strcat(cDynamicPage, LocalBuf);

	/* Add SECURITY TYPE  */
	CreateSecTypeTableItem(LocalBuf, Wifi_Setting.security_type);
	strcat(cDynamicPage, LocalBuf);

	/* Add PASSWORD */
	CreatePasswdTableItem(LocalBuf, Wifi_Setting.password, strlen((char *)Wifi_Setting.password));
	strcat(cDynamicPage, LocalBuf);

	/* Add CHANNEL  */
	CreateChannelTableItem(LocalBuf, Wifi_Setting.channel);
	strcat(cDynamicPage, LocalBuf);

#if USE_DIV_CSS
	/* Add Sbumit button */
	strcat(cDynamicPage, webHTML_SUBMIT_BTN);

	/* Add STA body */
	strcat(cDynamicPage, webHTML_STA_BOBY_START);
#endif

#if USE_DIV_CSS
	CreateScanSsidTableItem(LocalBuf);
	strcat(cDynamicPage, LocalBuf);

	CreateTargetAPTableItem(LocalBuf);
	strcat(cDynamicPage, LocalBuf);

	CreateConnStatusTableItem(LocalBuf);
	strcat(cDynamicPage, LocalBuf);

	strcat(cDynamicPage, webHTML_SCAN_BTN);

#endif
	/* ... Finally the page footer. */
	strcat(cDynamicPage, webHTML_END);
	//RTK_LOGI(NOTAG, "GenerateIndexHtmlPage(): %s\n", cDynamicPage);
	RTK_LOGI(NOTAG, "GenerateIndexHtmlPage Len: %d\n", strlen(cDynamicPage));
	rtos_sema_give(webs_wpage_sema);
}

static web_conn *web_conn_add(void)
{
	int i;
	web_conn *conn = NULL;

	for (i = 0; i < MAX_HTTP_CONNECTIONS; i++) {
		if (web_connections[i].status == -1) {
			web_connections[i].status = 1;
			conn = &web_connections[i];
#if CAPTIVE_PORTAL_DEBUG
			RTK_LOGI(NOTAG, "%s connection %d\n", __FUNCTION__, i);
#endif
			break;
		}
	}
	return conn;
}

static void web_conn_remove(web_conn *conn)
{
	int i;

	for (i = 0; i < MAX_HTTP_CONNECTIONS; i++) {
		if (&web_connections[i] == conn) {
			web_connections[i].status = -1;
#if CAPTIVE_PORTAL_DEBUG
			RTK_LOGI(NOTAG, "%s connection %d\n", __FUNCTION__, i);
#endif
			break;
		}
	}

}

static void web_conn_clear(void)
{
	int i;

	RTK_LOGI(NOTAG, "enter %s\n", __func__);
	for (i = 0; i < MAX_HTTP_CONNECTIONS; i++) {
		if (web_connections[i].status == 1) {
#if CAPTIVE_PORTAL_DEBUG
			RTK_LOGI(NOTAG, "%s connection %d, port = %d\n", __FUNCTION__, i, web_connections[i].conn->pcb.tcp->remote_port);
#endif
			netconn_close(web_connections[i].conn);
			netconn_delete(web_connections[i].conn);
			web_connections[i].status = -1;
		}
	}

}

static void GenerateWaitHtmlPage(char *cDynamicPage)
{
	while (rtos_sema_take(webs_wpage_sema, RTOS_MAX_DELAY) != RTK_SUCCESS);
	/* Generate the dynamic page...
	... First the page header. */
	strcpy(cDynamicPage, webWaitHTML_START);

	/* ... Finally the page footer. */
	strcat(cDynamicPage, webWaitHTML_END);

	//RTK_LOGI(NOTAG, "GenerateWaitHtmlPage(): %s\n",  cDynamicPage);
	//RTK_LOGI(NOTAG, "GenerateWaitHtmlPage Len: %d\n", strlen( cDynamicPage ));
	rtos_sema_give(webs_wpage_sema);
}

extern s32 wifi_get_scan_records(u32 *ap_num, struct rtw_scan_result *ap_list);
static s32 scan_result_handler(u32 scanned_AP_num, void *user_data)
{
	/* To avoid gcc warnings */
	(void) user_data;

	struct rtw_scan_result *scanned_AP_info;
	struct rtw_scan_result *scanned_AP_list = NULL;
	unsigned int i = 0;
	int ApNum = 0;

	if (scanned_AP_num == 0) {/* scanned no AP*/
		return RTK_FAIL;
	}

	scanned_AP_list = (struct rtw_scan_result *)rtos_mem_zmalloc(scanned_AP_num * sizeof(struct rtw_scan_result));
	if (scanned_AP_list == NULL) {
		return RTK_FAIL;
	}

	if (wifi_get_scan_records(&scanned_AP_num, scanned_AP_list) < 0) {
		rtos_mem_free((u8 *)scanned_AP_list);
		return RTK_FAIL;
	}

	for (i = 0; i < scanned_AP_num; i++) {
		scanned_AP_info = &scanned_AP_list[i];
		scanned_AP_info->ssid.val[scanned_AP_info->ssid.len] = 0; /* Ensure the SSID is null terminated */

		if (!strcmp((char *)scanned_AP_info->ssid.val, "")) {
			continue;
		}

		memcpy((scan_result.ap_list + ApNum)->ssid, scanned_AP_info->ssid.val, scanned_AP_info->ssid.len + 1);

		memcpy((scan_result.ap_list + ApNum)->bssid, scanned_AP_info->bssid.octet, 6);

		(scan_result.ap_list + ApNum)->channel = scanned_AP_info->channel;
		(scan_result.ap_list + ApNum)->sec_type = scanned_AP_info->security;
		(scan_result.ap_list + ApNum)->ap_power = scanned_AP_info->signal_strength;

#if CAPTIVE_PORTAL_DEBUG
		RTK_LOGI(NOTAG, MAC_FMT, MAC_ARG(scanned_AP_info->bssid.octet));
		RTK_LOGI(NOTAG, " %d\t ", scanned_AP_info->signal_strength);
		RTK_LOGI(NOTAG, " %d\t  ", scanned_AP_info->channel);
		RTK_LOGI(NOTAG, " %d\t  ", (unsigned int)scanned_AP_info->wps_type);
		RTK_LOGI(NOTAG, "%s\t\t ", (scanned_AP_info->security == RTW_SECURITY_OPEN) ? "Open" :
				 (scanned_AP_info->security == RTW_SECURITY_WEP_PSK) ? "WEP" :
				 (scanned_AP_info->security == RTW_SECURITY_WPA_TKIP_PSK) ? "WPA TKIP" :
				 (scanned_AP_info->security == RTW_SECURITY_WPA_AES_PSK) ? "WPA AES" :
				 (scanned_AP_info->security == RTW_SECURITY_WPA_MIXED_PSK) ? "WPA Mixed" :
				 (scanned_AP_info->security == RTW_SECURITY_WPA2_AES_PSK) ? "WPA2 AES" :
				 (scanned_AP_info->security == RTW_SECURITY_WPA2_TKIP_PSK) ? "WPA2 TKIP" :
				 (scanned_AP_info->security == RTW_SECURITY_WPA2_MIXED_PSK) ? "WPA2 Mixed" :
				 (scanned_AP_info->security == RTW_SECURITY_WPA_WPA2_TKIP_PSK) ? "WPA/WPA2 TKIP" :
				 (scanned_AP_info->security == RTW_SECURITY_WPA_WPA2_AES_PSK) ? "WPA/WPA2 AES" :
				 (scanned_AP_info->security == RTW_SECURITY_WPA_WPA2_MIXED_PSK) ? "WPA/WPA2 Mixed" :
				 "Unknown		");
		RTK_LOGI(NOTAG, " %s\n", scanned_AP_info->ssid.val);
#endif
		ApNum++;
		if (ApNum > SCAN_AP_LIST_MAX) {
			break;
		}
	}

	scan_result.ap_num = ApNum;

	ApNum = 0;
	rtos_mem_free((u8 *)scanned_AP_list);
	rtos_sema_give(scan_done_sema);
	return RTK_SUCCESS;
}

static int wifi_start_scan(void)
{
	RTK_LOGI(NOTAG, "%s\n", __func__);
	struct rtw_scan_param scan_param;
	char *scanned_AP_list;

	scanned_AP_list = rtos_mem_malloc(2 * SCAN_AP_LIST_MAX * sizeof(struct rtw_scan_result));
	if (!scanned_AP_list) {
		RTK_LOGE(NOTAG, "ERROR: malloc failed!\n");
		return -1;
	}
	memset(scanned_AP_list, 0, 2 * SCAN_AP_LIST_MAX * sizeof(struct rtw_scan_result));
	memset(scan_result.ap_list, 0, 2 * SCAN_AP_LIST_MAX * sizeof(ap_list_t));
	memset(&scan_param, 0, sizeof(struct rtw_scan_param));

	scan_param.scan_user_callback = scan_result_handler;
	scan_param.max_ap_record_num = 2 * SCAN_AP_LIST_MAX;

	if (wifi_scan_networks(&scan_param, 0) != RTK_SUCCESS) {
		RTK_LOGE(NOTAG, "ERROR: wifi scan failed!\n");
		return -1;
	}

	return 0;
}

static void wifi_scan_thread(void *pvParameters)
{
	(void)pvParameters;

	wifi_start_scan();
	rtos_task_delete(NULL);
}

/*get ap security mode from scan list*/
static int _get_ap_security_mode(IN char *ssid, OUT u32 *security_mode, OUT u8 *channel)
{
	struct rtw_scan_param scan_param;
	struct rtw_scan_result *scanned_ap_info;
	int scan_cnt = 0;
	struct rtw_scan_result *scanned_AP_list;
	int i = 0;

	memset(&scan_param, 0, sizeof(struct rtw_scan_param));
	scan_param.ssid = (u8 *)ssid;

	if ((scan_cnt = wifi_scan_networks(&scan_param, 1)) <= 0) {
		RTK_LOGE(NOTAG, "error %s, wifi scan failed\n", __func__);
	} else {
		scanned_AP_list = (struct rtw_scan_result *)rtos_mem_zmalloc(scan_cnt * sizeof(struct rtw_scan_result));
		if (scanned_AP_list == NULL) {
			RTK_LOGE(NOTAG, "error %s, malloc scan_buf failed\n", __func__);
			return -1;
		}
		if (wifi_get_scan_records((u32 *)&scan_cnt, scanned_AP_list) < 0) {
			rtos_mem_free((unsigned char *)scanned_AP_list);
			return -1;
		}
		for (i = 0; i < scan_cnt; i++) {
			scanned_ap_info = &scanned_AP_list[i];
			scanned_ap_info->ssid.val[scanned_ap_info->ssid.len] = 0; /* Ensure the SSID is null terminated */
			//RTK_LOGI(NOTAG, "info %s, ssid: %s\n", __func__, scanned_ap_info->ssid.val);
			if (strcmp(ssid, (char *)scanned_ap_info->ssid.val) == 0) {
				*security_mode = scanned_ap_info->security;
				*channel = scanned_ap_info->channel;
				RTK_LOGI(NOTAG, "info %s, channel: %d\n", __func__, *channel);
				rtos_mem_free((unsigned char *)scanned_AP_list);
				return 0;
			}
		}
		rtos_mem_free((unsigned char *)scanned_AP_list);
	}
	return -1;
}


static void ConnectTargetAP(void)
{
	int ret;
	struct rtw_network_info connect_param = {0};
	int security_retry_count = 0;
	u8 connect_channel;
	u8 connect_status;

	while (1) {
		if (_get_ap_security_mode((char *)target_ap_setting.ssid, &target_ap_setting.security_type, &connect_channel) == 0) {
			break;
		}
		security_retry_count++;
		if (security_retry_count >= 3) {
			RTK_LOGE(NOTAG, "Can't get AP security mode and channel.\n");
			ret = -1;
			return;
		}
	}

	connect_param.ssid.len = strlen((char *)target_ap_setting.ssid);
	strncpy((char *)connect_param.ssid.val, (char *)target_ap_setting.ssid, 33);
	connect_param.password_len = strlen((char *)target_ap_setting.password);
	connect_param.password = target_ap_setting.password;
	connect_param.channel = connect_channel;
	connect_param.security_type = target_ap_setting.security_type;

	RTK_LOGI(NOTAG, "Joining BSS by SSID %s [%s]...\n\r", connect_param.ssid.val, connect_param.password);

	wifi_get_join_status(&connect_status);
	if (connect_status == RTW_JOINSTATUS_SUCCESS) {
		wifi_disconnect();
	}

	ret = wifi_connect(&connect_param, 0);

	if (ret != RTK_SUCCESS) {
		if (ret == -RTK_ERR_WIFI_CONN_INVALID_KEY) {
			RTK_LOGE(NOTAG, "ERROR:Invalid Key\n");
		}

		RTK_LOGE(NOTAG, "ERROR: Can't connect to AP\n");
		return;
	}

#ifdef CONFIG_LWIP_LAYER
	/* Start DHCPClient */
	LwIP_DHCP(0, DHCP_START);
#endif
}

static void http_translate_url_encode(char *ptr)
{

	char *data = ptr;
	char tmp_data[3] = {0};
	char outdata[33] = {0};
	int buffer;
	char *outdata_ptr = outdata;

	while (*data != '\0') {

		if (*data == '%') {
			if ((*(data + 1) != 0) && (*(data + 2) != 0)) {
				tmp_data[0] = *(data + 1);
				tmp_data[1] = *(data + 2);
				_sscanf_ss(tmp_data, "%x", &buffer);
				*outdata_ptr = (char)buffer;

				/* destroy data */
				*data  = 0;
				*(data + 1)  = 0;
				*(data + 2)  = 0;

				data += 2;
				outdata_ptr++;
			}

		} else {
			*outdata_ptr = *data;
			if (*data == '+') {
				*outdata_ptr = ' ';
			}
			outdata_ptr++;
		}
		data++;
	}
	strcpy(ptr, outdata);

}

static u8_t ProcessPostMessage(struct netbuf  *pxRxBuffer, char *LocalBuf)
{
	struct pbuf *p;
	char *pcRxString, *ptr;
	unsigned short usLength;
	u8_t bChanged = 0;
	enum rtw_security secType;
	u8_t channel;
	u8_t len = 0;

	pcRxString = LocalBuf;
	p = pxRxBuffer->p;
	usLength = p->tot_len;

	while (p) {
		memcpy(pcRxString, p->payload, p->len);
		pcRxString += p->len;
		p = p->next;
	}
	pcRxString = LocalBuf;
	pcRxString[usLength] = '\0';
	RTK_LOGI(NOTAG, "usLength=%d pcRxString = %s\n", usLength, pcRxString);

	ptr = (char *)strstr(pcRxString, "Ssid=");
	if (ptr) {
		//RTK_LOGI(NOTAG, "ssid passed = %s\n", ptr);
		pcRxString = (char *)strstr(ptr, "&");
		*pcRxString++ = '\0';
		ptr += 5;
		http_translate_url_encode(ptr);
		if (strcmp((char *)Wifi_Setting.ssid, ptr)) {
			bChanged = 1;
			len = strlen(ptr);
			if (len > MAX_SOFTAP_SSID_LEN) {
				len = MAX_SOFTAP_SSID_LEN;
				ptr[len] = '\0';
			}
			strcpy((char *)Wifi_Setting.ssid, ptr);
		}
	}

	//RTK_LOGI(NOTAG, "get wifi_config.ssid = %s\n", Wifi_Setting.ssid);

	ptr = (char *)strstr(pcRxString, "Security+Type=");
	if (ptr) {
		pcRxString = (char *)strstr(ptr, "&");
		*pcRxString++ = '\0';
		ptr += 14;
		if (!strcmp(ptr, "open")) {
			secType = RTW_SECURITY_OPEN;
		} else if (!strcmp(ptr, "wpa2-aes")) {
			secType = RTW_SECURITY_WPA2_AES_PSK;
		} else {
			secType = RTW_SECURITY_OPEN;
		}
		if (Wifi_Setting.security_type != secType) {
			bChanged = 1;
			Wifi_Setting.security_type = secType;
		}
	}

	//RTK_LOGI(NOTAG, "wifi_config.security_type = %d\n", Wifi_Setting.security_type);
	if (Wifi_Setting.security_type > RTW_SECURITY_OPEN) {
		ptr = (char *)strstr(pcRxString, "Password=");
		if (ptr) {
			pcRxString = (char *)strstr(ptr, "&");
			*pcRxString++ = '\0';
			ptr += 9;
			if (strcmp((char *)Wifi_Setting.password, ptr)) {
				bChanged = 1;
				len = strlen(ptr);
				if (len > MAX_PASSWORD_LEN) {
					len = MAX_PASSWORD_LEN;
					ptr[len] = '\0';
				}
				strcpy((char *)Wifi_Setting.password, ptr);
			}
		}
		//RTK_LOGI(NOTAG, "wifi_config.password = %s\n", Wifi_Setting.password);
	}
	ptr = (char *)strstr(pcRxString, "Channel=");
	if (ptr) {
		ptr += 8;
		channel = web_atoi(ptr);
		if ((channel > MAX_CHANNEL_NUM) || (channel < 1)) {
			channel = 1;
		}
		if (Wifi_Setting.channel != channel) {
			bChanged = 1;
			Wifi_Setting.channel = channel;
		}
	}
	//RTK_LOGI(NOTAG, "wifi_config.channel = %d\n", Wifi_Setting.channel);

	ptr = (char *)strstr(pcRxString, "TargetAP=");
	if (ptr) {
		pcRxString = (char *)strstr(ptr, "&");
		*pcRxString++ = '\0';
		ptr += 9;
		strcpy((char *)target_ap_setting.ssid, ptr);
	}

	ptr = (char *)strstr(pcRxString, "SPassword=");
	if (ptr) {
		pcRxString = (char *)strstr(ptr, "&");
		*pcRxString++ = '\0';
		ptr += 10;
		len = strlen(ptr);
		strcpy((char *)target_ap_setting.password, ptr);
	}

	ptr = (char *)strstr(pcRxString, "CONNSTATUS=");
	if (ptr) {
		pcRxString = (char *)strstr(ptr, "&");
		if (pcRxString) {
			*pcRxString++ = '\0';
		} else {
			pcRxString = "\0";
		}
		ptr += 11;
		if (!strcmp(ptr, "Connecting")) {
			connect_target_ap = 1;
			bChanged = 0;
		}
	}

	ptr = (char *)strstr(pcRxString, "scan=");
	if (ptr) {
		scan_ap = 1;
		bChanged = 0;
	}

	return bChanged;
}

static void vProcessConnection(void *param)
{
	static char cDynamicPage[webMAX_PAGE_SIZE];
	struct netbuf *pxRxBuffer, *pxRxBuffer1 = NULL;
	char *pcRxString;
	unsigned short usLength;
	static char LocalBuf[LOCAL_BUF_SIZE];
	u8_t bChanged = 0;
	int ret_recv = ERR_OK;
	int ret_accept = ERR_OK;
	web_conn *pxNetCon  = (web_conn *)param;
	rtos_task_t conn_task = pxNetCon->task;

	/* Load WiFi Setting*/
	LoadWifiSetting();

	RTK_LOGI(NOTAG, "%s %d, remote_port = %d\n", __func__, __LINE__, pxNetCon->conn->pcb.tcp->remote_port);

	pxNetCon->conn->recv_timeout = 500;

	/* We expect to immediately get data. */
	port_netconn_recv(pxNetCon->conn, pxRxBuffer, ret_recv);

	if (pxRxBuffer != NULL && ret_recv == ERR_OK) {
		/* Where is the data? */
		netbuf_data(pxRxBuffer, (void *)&pcRxString, &usLength);

		//RTK_LOGI(NOTAG, "usLength=%d pcRxString = \n%s\n", usLength, pcRxString);
		/* Is this a GET?  We don't handle anything else. */
		if (!strncmp(pcRxString, "GET", 3)) {
			RTK_LOGI(NOTAG, "usLength=%d pcRxString=%s \n", usLength, pcRxString);

			/* Write out the HTTP OK header. */
			netconn_write(pxNetCon->conn, webHTTP_OK, (u16_t) strlen(webHTTP_OK), NETCONN_COPY);

			RTK_LOGI(NOTAG, "%s %d \n", __func__, __LINE__);

			/* Generate index.html page. */
			GenerateIndexHtmlPage(cDynamicPage, LocalBuf);

			RTK_LOGI(NOTAG, "cDynamicPage=%s \n", cDynamicPage);

			/* Write out the dynamically generated page. */
			netconn_write(pxNetCon->conn, cDynamicPage, (u16_t) strlen(cDynamicPage), NETCONN_COPY);
		} else if (!strncmp(pcRxString, "POST", 4)) {
			/* Write out the HTTP OK header. */
			netconn_write(pxNetCon->conn, webHTTP_OK, (u16_t) strlen(webHTTP_OK), NETCONN_COPY);

			bChanged = ProcessPostMessage(pxRxBuffer, LocalBuf);
			if (bChanged == 0) {
				port_netconn_recv(pxNetCon->conn, pxRxBuffer1, ret_recv);
				if (pxRxBuffer != NULL && ret_recv == ERR_OK) {
					bChanged = ProcessPostMessage(pxRxBuffer1, LocalBuf);
					netbuf_delete(pxRxBuffer1);
				}
			}

			if (bChanged) {
				GenerateWaitHtmlPage(cDynamicPage);

				/* Write out the generated page. */
				netconn_write(pxNetCon->conn, cDynamicPage, (u16_t) strlen(cDynamicPage), NETCONN_COPY);
#if CONFIG_READ_FLASH
				StoreApInfo();
#endif
			} else {

				if (connect_target_ap) {
					connect_target_ap = 0;
					ConnectTargetAP();
				}

				if (scan_ap) {
					scan_ap = 0;
					if (rtos_task_create(NULL, (const char *)"wifi_scan_thread", wifi_scan_thread, NULL, 1024 * 4, 1) != RTK_SUCCESS) {
						RTK_LOGE(NOTAG, "Create wifi_scan_thread task failed!\n");
					}

					rtos_sema_take(scan_done_sema, RTOS_MAX_DELAY);
					RTK_LOGI(NOTAG, "wifi scan done\n");
				}

				/* Generate index.html page. */
				GenerateIndexHtmlPage(cDynamicPage, LocalBuf);

				/* Write out the generated page. */
				netconn_write(pxNetCon->conn, cDynamicPage, (u16_t) strlen(cDynamicPage), NETCONN_COPY);
			}
		}
		netbuf_delete(pxRxBuffer);
	}
	netconn_close(pxNetCon->conn);

	if (bChanged) {
		struct netconn *pxNewConnection;
		RestartSoftAP();
		pxHTTPListener->recv_timeout = 1;
		port_netconn_accept(pxHTTPListener, pxNewConnection, ret_accept);
		if (pxNewConnection != NULL && ret_accept == ERR_OK) {
			netconn_close(pxNewConnection);
			while (netconn_delete(pxNewConnection) != ERR_OK) {
				rtos_time_delay_ms(webSHORT_DELAY);
			}
		}
		pxHTTPListener->recv_timeout = 0;
	}
	while (netconn_delete(pxNetCon->conn) != ERR_OK) {
		rtos_time_delay_ms(webSHORT_DELAY);
	}
	web_conn_remove(pxNetCon);
	rtos_task_delete(conn_task);
}

static void vCaptivePortalServer(void *pvParameters)
{
	web_conn *pxNewConnection;
	extern err_t ethernetif_init(struct netif * netif);
	int ret = ERR_OK;
	int i;
	/* Parameters are not used - suppress compiler error. */
	(void)pvParameters;

	/* Create a new tcp connection handle */
	pxHTTPListener = netconn_new(NETCONN_TCP);
	ip_set_option(pxHTTPListener->pcb.ip, SOF_REUSEADDR);
	netconn_bind(pxHTTPListener, NULL, webHTTP_PORT);
	netconn_listen(pxHTTPListener);

	web_connections = (web_conn *)rtos_mem_malloc(sizeof(web_conn) * MAX_HTTP_CONNECTIONS);
	if (!web_connections) {
		RTK_LOGE(NOTAG, "malloc web_connections failed !!");
		goto exit;
	} else {
		for (i = 0; i < MAX_HTTP_CONNECTIONS; i++) {
			web_connections[i].status = -1;
		}
	}

#if CONFIG_READ_FLASH
	/* Load wifi_config */
	LoadWifiConfig();
	RestartSoftAP();
#endif
	RTK_LOGI(NOTAG, "enter vCaptivePortalServer\n");
	RTK_LOGI(NOTAG, "MEMP_NUM_NETBUF = %d, MEMP_NUM_TCP_PCB =%d, MEMP_NUM_NETCONN = %d\n", MEMP_NUM_NETBUF, MEMP_NUM_TCP_PCB, MEMP_NUM_NETCONN);

	/* Loop forever */
	for (;;) {
		if (webs_terminate) {
			break;
		}

		pxNewConnection = web_conn_add();

		if (pxNewConnection) {
			/* Service connection. */
			port_netconn_accept(pxHTTPListener, pxNewConnection->conn, ret);
			if (pxNewConnection->conn != NULL && ret == ERR_OK) {
				rtos_time_delay_ms(500);
				if (rtos_task_create(&pxNewConnection->task, (const char *)"web_conn", vProcessConnection, pxNewConnection, 1024 * 4, 1) != RTK_SUCCESS) {
					RTK_LOGE(NOTAG, "ERROR: rtos_task_create web_server_conn");
				}
			} else {
				rtos_time_delay_ms(webSHORT_DELAY);
				web_conn_remove(pxNewConnection);
			}
		}
	}
	if (pxHTTPListener) {
		netconn_close(pxHTTPListener);
		netconn_delete(pxHTTPListener);
		pxHTTPListener = NULL;
	}

	RTK_LOGE(NOTAG, "stop_web_server done \n");

	for (i = 0; i < MAX_HTTP_CONNECTIONS; i++) {
		if (web_connections[i].status == 1) {
			RTK_LOGE(NOTAG, "active connect port: %d\n", web_connections[i].conn->pcb.tcp->remote_port);
		}
	}

exit:
	if (web_connections) {
		rtos_mem_free(web_connections);
	}
	rtos_sema_give(webs_sema);
	rtos_task_delete(NULL);
}

#define STACKSIZE				512

#ifdef CONFIG_LWIP_LAYER
extern struct netif xnetif[NET_IF_NUM];
#endif

/****************************************************** 
	Define for BLE
********************************************************/
#define RTK_BT_DEV_NAME "RTK_BT_PERIPHERAL"
static uint8_t adv_data[] = {
	0x02, //AD len
	RTK_BT_LE_GAP_ADTYPE_FLAGS, //AD types
	RTK_BT_LE_GAP_ADTYPE_FLAGS_GENERAL | RTK_BT_LE_GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED, //AD data
	0x12,
	RTK_BT_LE_GAP_ADTYPE_LOCAL_NAME_COMPLETE,
	'R', 'T', 'K', '_', 'B', 'T', '_', 'P', 'E', 'R', 'I', 'P', 'H', 'E', 'R', 'A', 'L',
};
static uint8_t scan_rsp_data[] = {
	0x3,
	RTK_BT_LE_GAP_ADTYPE_APPEARANCE, //GAP_ADTYPE_APPEARANCE
	LO_WORD(RTK_BT_LE_GAP_APPEARANCE_UNKNOWN),
	HI_WORD(RTK_BT_LE_GAP_APPEARANCE_UNKNOWN),
};
static rtk_bt_le_adv_param_t def_adv_param = {
	.interval_min = 200, //units of 0.625ms
	.interval_max = 250,
	.type = RTK_BT_LE_ADV_TYPE_IND,
	.own_addr_type = RTK_BT_LE_ADDR_TYPE_PUBLIC,
	.peer_addr = {
		.type = (rtk_bt_le_addr_type_t)0,
		.addr_val = {0},
	},
	.channel_map = RTK_BT_LE_ADV_CHNL_ALL,
	.filter_policy = RTK_BT_LE_ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};
static rtk_bt_le_security_param_t sec_param = {
	.io_cap = RTK_IO_CAP_NO_IN_NO_OUT,
	.oob_data_flag = 0,
	.bond_flag = 1,
	.mitm_flag = 0,
	.sec_pair_flag = 0,
	.use_fixed_key = 0,
	.fixed_key = 000000,
};

static void start_soft_ap(void) {
#ifdef CONFIG_LWIP_LAYER
	struct netif *pnetif = &xnetif[SOFTAP_WLAN_INDEX];
	u32 ip_addr;
	u32 netmask;
	u32 gw;
#endif

	int timeout = 20;

#ifdef CONFIG_LWIP_LAYER
	dhcps_deinit();
	ip_addr = CONCAT_TO_UINT32(AP_IP_ADDR0, AP_IP_ADDR1, AP_IP_ADDR2, AP_IP_ADDR3);
	netmask = CONCAT_TO_UINT32(AP_NETMASK_ADDR0, AP_NETMASK_ADDR1, AP_NETMASK_ADDR2, AP_NETMASK_ADDR3);
	gw = CONCAT_TO_UINT32(AP_GW_ADDR0, AP_GW_ADDR1, AP_GW_ADDR2, AP_GW_ADDR3);
	LwIP_SetIP(SOFTAP_WLAN_INDEX, ip_addr, netmask, gw);
#endif
	RTK_LOGE(NOTAG, "Enable Wi-Fi with STA + AP mode\n");
	if (wifi_on(RTW_MODE_STA) < 0) {
		RTK_LOGE(NOTAG, "wifi_on failed\n");
		return;
	}

	/*********************************************************************************
	*	1-3. Start AP
	*********************************************************************************/
	SOFTAP_CONFIG.ssid.len = strlen(SOFTAP_SSID);
	memcpy(SOFTAP_CONFIG.ssid.val, SOFTAP_SSID, SOFTAP_CONFIG.ssid.len);
	SOFTAP_CONFIG.password = (unsigned char *)SOFTAP_PASSWORD;
	SOFTAP_CONFIG.password_len = strlen(SOFTAP_PASSWORD);
	SOFTAP_CONFIG.security_type = SOFTAP_SECURITY;
	SOFTAP_CONFIG.channel = SOFTAP_CHANNEL;

	if (wifi_start_ap(&SOFTAP_CONFIG) < 0) {
		RTK_LOGE(NOTAG, "wifi_start_ap failed\n");
		return;
	}
	RTK_LOGI(NOTAG, "Check AP running\n");

	while (1) {
		struct rtw_wifi_setting setting;
		wifi_get_setting(SOFTAP_WLAN_INDEX, &setting);
		if (strlen((const char *)setting.ssid) > 0) {
			if (strcmp((const char *) setting.ssid, (const char *)SOFTAP_CONFIG.ssid.val) == 0) {
				RTK_LOGI(NOTAG, "%s started\n", SOFTAP_CONFIG.ssid.val);
				break;
			}
		}

		if (timeout == 0) {
			RTK_LOGI(NOTAG, "ERROR: Start AP timeout!");
			break;
		}

		rtos_time_delay_ms(1 * RTOS_TICK_RATE_HZ);
		timeout --;
	}

#ifdef CONFIG_LWIP_LAYER
	dhcps_init(pnetif);
#endif
}

static void stop_soft_ap(void) {
	u32 ip_addr;
	u32 netmask;
	u32 gw;
	// stop dhcp server
#ifdef CONFIG_LWIP_LAYER
	dhcps_deinit();
	ip_addr = CONCAT_TO_UINT32(AP_IP_ADDR0, AP_IP_ADDR1, AP_IP_ADDR2, AP_IP_ADDR3);
	netmask = CONCAT_TO_UINT32(AP_NETMASK_ADDR0, AP_NETMASK_ADDR1, AP_NETMASK_ADDR2, AP_NETMASK_ADDR3);
	gw = CONCAT_TO_UINT32(AP_GW_ADDR0, AP_GW_ADDR1, AP_GW_ADDR2, AP_GW_ADDR3);
	LwIP_SetIP(SOFTAP_WLAN_INDEX, ip_addr, netmask, gw);
#endif
	wifi_stop_ap();
}

static void app_server_disconnect(uint16_t conn_handle)
{
	simple_ble_srv_disconnect(conn_handle);
// 	bas_disconnect(conn_handle);
// 	hrs_disconnect(conn_handle);
// 	gls_disconnect(conn_handle);
// #if defined(RTK_BLE_5_1_CTE_SUPPORT) && RTK_BLE_5_1_CTE_SUPPORT
// 	cte_srv_disconnect(conn_handle);
// #endif
}

static void app_server_deinit(void)
{
	simple_ble_srv_status_deinit();
// 	bas_status_deinit();
// 	hrs_status_deinit();
// 	gls_status_deinit();
// #if defined(RTK_BLE_5_1_CTE_SUPPORT) && RTK_BLE_5_1_CTE_SUPPORT
// 	cte_srv_status_deinit();
// #endif
}
static rtk_bt_evt_cb_ret_t peripheral_gap_app_callback(uint8_t evt_code, void *param, uint32_t len)
{
	(void)param;
	(void)len;
	rtk_bt_evt_cb_ret_t ret = RTK_BT_EVT_CB_OK;

	switch (evt_code) {
#if defined(RTK_BT_5_2_L2C_ECFC_SUPPORT) && RTK_BT_5_2_L2C_ECFC_SUPPORT
	case RTK_BT_GAP_EVT_ECFC_CONN_CMPL_IND: {
		char addr_str[40] = {0};
		rtk_bt_ecfc_conn_cmpl_ind_t *p_ind = (rtk_bt_ecfc_conn_cmpl_ind_t *)param;
		rtk_bt_addr_to_str(p_ind->bd_type, p_ind->bd_addr, addr_str, 40);
		if (p_ind->err) {
			BT_LOGE("[APP] ECFC connection established fail(0x%x). %s\r\n", p_ind->err, addr_str);
		} else {
			BT_LOGA("[APP] ECFC connection established OK. %s\r\n", addr_str);
			BT_LOGA("[APP] proto_id(%d) conn_handle(%d) remote_mtu(%d) local_mtu(%d) local_mps(%d)\r\n",
					p_ind->proto_id, p_ind->conn_handle, p_ind->remote_mtu, p_ind->local_mtu, p_ind->local_mps);
			BT_DUMP16A("[APP] cid: ", p_ind->cid, p_ind->cid_num);
		}
		break;
	}
	case RTK_BT_GAP_EVT_ECFC_DATA_IND: {
		rtk_bt_ecfc_data_ind_t *p_ind = (rtk_bt_ecfc_data_ind_t *)param;
		BT_LOGA("[APP] ECFC data received. proto_id(%d) conn_handle(%d) cid(%d) length(%d)\r\n",
				p_ind->proto_id, p_ind->conn_handle, p_ind->cid, p_ind->length);
		break;
	}
	case RTK_BT_GAP_EVT_ECFC_CONN_REQ_IND: {
		char addr_str[40] = {0};
		rtk_bt_ecfc_conn_req_ind_t *p_ind = (rtk_bt_ecfc_conn_req_ind_t *)param;
		rtk_bt_addr_to_str(p_ind->bd_type, p_ind->bd_addr, addr_str, 40);
		BT_LOGA("[APP] ECFC connection requset. %s\r\n", addr_str);
		BT_LOGA("[APP] proto_id(%d) conn_handle(%d) remote_mtu(%d) identity_id(%d)\r\n",
				p_ind->proto_id, p_ind->conn_handle, p_ind->remote_mtu, p_ind->identity_id);
		BT_DUMP16A("[APP] cid: ", p_ind->cid, p_ind->cid_num);
		break;
	}
	case RTK_BT_GAP_EVT_ECFC_DISCONN_IND: {
		rtk_bt_ecfc_disconn_ind_t *p_ind = (rtk_bt_ecfc_disconn_ind_t *)param;
		BT_LOGA("[APP] ECFC disconnect. proto_id(%d) conn_handle(%d) cid(%d) cause(0x%x)\r\n",
				p_ind->proto_id, p_ind->conn_handle, p_ind->cid, p_ind->cause);
		break;
	}
	case RTK_BT_GAP_EVT_ECFC_RECONF_REQ_IND: { //BT api shall not be called here
		rtk_bt_ecfc_reconf_req_ind_t *p_ind = (rtk_bt_ecfc_reconf_req_ind_t *)param;
		BT_LOGA("[APP] ECFC reconfigure requset. proto_id(%d) conn_handle(%d) remote_mtu(%d) remote_mps(%d)\r\n",
				p_ind->proto_id, p_ind->conn_handle, p_ind->remote_mtu, p_ind->remote_mps);
		BT_DUMP16A("[APP] cid: ", p_ind->cid, p_ind->cid_num);
		if (p_ind->remote_mtu < RTK_BT_GAP_ECFC_MIN_MTU || p_ind->remote_mps < RTK_BT_GAP_ECFC_MIN_MPS) {
			ret = RTK_BT_EVT_CB_REJECT;
			BT_LOGE("[APP] Reject!\r\n");
		} else {
			ret = RTK_BT_EVT_CB_OK;
			BT_LOGA("[APP] Accept!\r\n");
		}
		break;
	}
	case RTK_BT_GAP_EVT_ECFC_RECONF_RSP_IND: {
		rtk_bt_ecfc_reconf_rsp_ind_t *p_ind = (rtk_bt_ecfc_reconf_rsp_ind_t *)param;
		if (p_ind->cause) {
			BT_LOGE("[APP] ECFC reconfigure fail\r\n");
		} else {
			BT_LOGA("[APP] ECFC reconfigure OK, local_mtu(%d) local mps(%d)\r\n",
					p_ind->local_mtu, p_ind->local_mps);
			BT_DUMP16A("[APP] cid: ", p_ind->cid, p_ind->cid_num);
		}
		break;
	}
#endif
	default:
		BT_LOGE("[APP] Unkown common gap cb evt type: %d\r\n", evt_code);
		break;
	}

	return ret;
}

static rtk_bt_evt_cb_ret_t ble_peripheral_gap_app_callback(uint8_t evt_code, void *param, uint32_t len)
{
	(void)len;
	char le_addr[30] = {0};
	char *role;

	switch (evt_code) {
	case RTK_BT_LE_GAP_EVT_ADV_START_IND: {
		rtk_bt_le_adv_start_ind_t *adv_start_ind = (rtk_bt_le_adv_start_ind_t *)param;
		if (!adv_start_ind->err) {
			BT_LOGA("[APP] ADV started: adv_type %d  \r\n", adv_start_ind->adv_type);
		} else {
			BT_LOGE("[APP] ADV start failed, err 0x%x \r\n", adv_start_ind->err);
		}
		BT_AT_PRINT("+BLEGAP:adv,start,%d,%d\r\n", (adv_start_ind->err == 0) ? 0 : -1, adv_start_ind->adv_type);
		break;
	}

	case RTK_BT_LE_GAP_EVT_ADV_STOP_IND: {
		rtk_bt_le_adv_stop_ind_t *adv_stop_ind = (rtk_bt_le_adv_stop_ind_t *)param;
		if (!adv_stop_ind->err) {
			BT_LOGA("[APP] ADV stopped: reason 0x%x \r\n", adv_stop_ind->stop_reason);
		} else {
			BT_LOGE("[APP] ADV stop failed, err 0x%x \r\n", adv_stop_ind->err);
		}
		BT_AT_PRINT("+BLEGAP:adv,stop,%d,0x%x\r\n", (adv_stop_ind->err == 0) ? 0 : -1, adv_stop_ind->stop_reason);
		break;
	}

#if defined(RTK_BLE_5_0_PA_ADV_SUPPORT) && RTK_BLE_5_0_PA_ADV_SUPPORT
	case RTK_BT_LE_GAP_EVT_PA_IND: {
		rtk_bt_le_pa_ind_t *pa_ind = (rtk_bt_le_pa_ind_t *)param;
		if (pa_ind->state == RTK_BT_LE_PA_STATE_IDLE) {
			if (pa_ind->cause) {
				BT_LOGE("[APP] Periodic ADV(%d) stopped failed, err 0x%x\r\n", pa_ind->adv_handle, pa_ind->cause);
			} else {
				BT_LOGA("[APP] Periodic ADV(%d) stopped\r\n", pa_ind->adv_handle);
			}
		} else if (pa_ind->state == RTK_BT_LE_PA_STATE_ADVERTISING) {
			if (pa_ind->cause) {
				BT_LOGE("[APP] Periodic ADV(%d) started failed, err 0x%x\r\n", pa_ind->adv_handle, pa_ind->cause);
			} else {
				BT_LOGA("[APP] Periodic ADV(%d) started\r\n", pa_ind->adv_handle);
			}
		} else if (pa_ind->state == RTK_BT_LE_PA_STATE_WAIT_EXT_ADV_ADVERTISING) {
			BT_LOGE("[APP] Periodic ADV(%d) waiting for ext adv start, cause 0x%x\r\n", pa_ind->adv_handle, pa_ind->cause);
		}
		BT_AT_PRINT("+BLEGAP:pa_adv,%s,%d,%d\r\n",
					((pa_ind->state == RTK_BT_LE_PA_STATE_IDLE) ? "stop" : ((pa_ind->state == RTK_BT_LE_PA_STATE_ADVERTISING) ? "start" : "wait_eadv")),
					((pa_ind->state == RTK_BT_LE_PA_STATE_WAIT_EXT_ADV_ADVERTISING) ? 0 : ((pa_ind->cause == 0) ? 0 : -1)),
					pa_ind->adv_handle);
		break;
	}
#endif


	case RTK_BT_LE_GAP_EVT_SCAN_START_IND: {
		rtk_bt_le_scan_start_ind_t *scan_start_ind = (rtk_bt_le_scan_start_ind_t *)param;
		if (!scan_start_ind->err) {
			BT_LOGA("[APP] Scan started, scan_type: %d\r\n", scan_start_ind->scan_type);
		} else {
			BT_LOGE("[APP] Scan start failed(err: 0x%x)\r\n", scan_start_ind->err);
		}
		BT_AT_PRINT("+BLEGAP:scan,start,%d,%d\r\n", (scan_start_ind->err == 0) ? 0 : -1, scan_start_ind->scan_type);
		break;
	}

	case RTK_BT_LE_GAP_EVT_SCAN_RES_IND: {
		rtk_bt_le_scan_res_ind_t *scan_res_ind = (rtk_bt_le_scan_res_ind_t *)param;
		rtk_bt_le_addr_to_str(&(scan_res_ind->adv_report.addr), le_addr, sizeof(le_addr));
		BT_LOGA("[APP] Scan info, [Device]: %s, AD evt type: %d, RSSI: %d\r\n",
				le_addr, scan_res_ind->adv_report.evt_type, scan_res_ind->adv_report.rssi);
		BT_AT_PRINT("+BLEGAP:scan,info,%s,%d,%d,%d\r\n",
					le_addr, scan_res_ind->adv_report.evt_type, scan_res_ind->adv_report.rssi,
					scan_res_ind->adv_report.len);
		break;
	}

	case RTK_BT_LE_GAP_EVT_SCAN_STOP_IND: {
		rtk_bt_le_scan_stop_ind_t *scan_stop_ind = (rtk_bt_le_scan_stop_ind_t *)param;
		if (!scan_stop_ind->err) {
			BT_LOGA("[APP] Scan stopped, reason: 0x%x\r\n", scan_stop_ind->stop_reason);
		} else {
			BT_LOGE("[APP] Scan stop failed(err: 0x%x)\r\n", scan_stop_ind->err);
		}
		BT_AT_PRINT("+BLEGAP:scan,stop,%d,0x%x\r\n", (scan_stop_ind->err == 0) ? 0 : -1, scan_stop_ind->stop_reason);
		break;
	}


	case RTK_BT_LE_GAP_EVT_CONNECT_IND: {
		rtk_bt_le_conn_ind_t *conn_ind = (rtk_bt_le_conn_ind_t *)param;
		rtk_bt_le_addr_to_str(&(conn_ind->peer_addr), le_addr, sizeof(le_addr));
		if (!conn_ind->err) {
			role = conn_ind->role ? "slave" : "master";
			BT_LOGA("[APP] Connected, handle: %d, role: %s, remote device: %s\r\n",
					conn_ind->conn_handle, role, le_addr);
					start_soft_ap();
		} else {
			BT_LOGE("[APP] Connection establish failed(err: 0x%x), remote device: %s\r\n",
					conn_ind->err, le_addr);
		}
		BT_AT_PRINT("+BLEGAP:conn,%d,%d,%s\r\n", (conn_ind->err == 0) ? 0 : -1, (int)conn_ind->conn_handle, le_addr);
		break;
	}

	case RTK_BT_LE_GAP_EVT_DISCONN_IND: {
		rtk_bt_le_disconn_ind_t *disconn_ind = (rtk_bt_le_disconn_ind_t *)param;
		rtk_bt_le_addr_to_str(&(disconn_ind->peer_addr), le_addr, sizeof(le_addr));
		role = disconn_ind->role ? "slave" : "master";
		BT_LOGA("[APP] Disconnected, reason: 0x%x, handle: %d, role: %s, remote device: %s\r\n",
				disconn_ind->reason, disconn_ind->conn_handle, role, le_addr);
		BT_AT_PRINT("+BLEGAP:disconn,0x%x,%d,%s,%s\r\n",
					disconn_ind->reason, disconn_ind->conn_handle, role, le_addr);
		stop_soft_ap();
#if !defined(RTK_BLE_5_0_USE_EXTENDED_ADV) || !RTK_BLE_5_0_USE_EXTENDED_ADV
		rtk_bt_le_gap_dev_state_t dev_state;
		rtk_bt_le_adv_param_t adv_param = {0};
		if (rtk_bt_le_gap_get_dev_state(&dev_state) == RTK_BT_OK &&
			dev_state.gap_adv_state == RTK_BT_LE_ADV_STATE_IDLE) {
			memcpy(&adv_param, &def_adv_param, sizeof(rtk_bt_le_adv_param_t));
			BT_LOGA("[APP] Reconnect ADV starting, adv type:%d,  own_addr_type: %d, filter_policy: %d\r\n"
					, adv_param.type,  adv_param.own_addr_type, adv_param.filter_policy);
			BT_APP_PROCESS(rtk_bt_le_gap_start_adv(&adv_param));
		}
#endif /* RTK_BLE_5_0_USE_EXTENDED_ADV */
		/* gatts action */
		app_server_disconnect(disconn_ind->conn_handle);
		break;
	}

	case RTK_BT_LE_GAP_EVT_CONN_UPDATE_IND: {
		rtk_bt_le_conn_update_ind_t *conn_update_ind =
			(rtk_bt_le_conn_update_ind_t *)param;
		if (conn_update_ind->err) {
			BT_LOGE("[APP] Update conn param failed, conn_handle: %d, err: 0x%x\r\n",
					conn_update_ind->conn_handle, conn_update_ind->err);
			BT_AT_PRINT("+BLEGAP:conn_update,%d,-1\r\n", conn_update_ind->conn_handle);
		} else {
			BT_LOGA("[APP] Conn param is updated, conn_handle: %d, conn_interval: 0x%x, "       \
					"conn_latency: 0x%x, supervision_timeout: 0x%x\r\n",
					conn_update_ind->conn_handle,
					conn_update_ind->conn_interval,
					conn_update_ind->conn_latency,
					conn_update_ind->supv_timeout);
			BT_AT_PRINT("+BLEGAP:conn_update,%d,0,0x%x,0x%x,0x%x\r\n",
						conn_update_ind->conn_handle,
						conn_update_ind->conn_interval,
						conn_update_ind->conn_latency,
						conn_update_ind->supv_timeout);
		}
		break;
	}

	case RTK_BT_LE_GAP_EVT_REMOTE_CONN_UPDATE_REQ_IND: { //BT api shall not be called here
		rtk_bt_le_remote_conn_update_req_ind_t *rmt_update_req =
			(rtk_bt_le_remote_conn_update_req_ind_t *)param;
		BT_LOGA("[APP] Remote device request a change in conn param, conn_handle: %d, "      \
				"conn_interval_max: 0x%x, conn_interval_min: 0x%x, conn_latency: 0x%x, "      \
				"timeout: 0x%x. The host stack accept it.\r\n",
				rmt_update_req->conn_handle,
				rmt_update_req->conn_interval_max,
				rmt_update_req->conn_interval_min,
				rmt_update_req->conn_latency,
				rmt_update_req->supv_timeout);
		return RTK_BT_EVT_CB_ACCEPT;
		break;
	}

	case RTK_BT_LE_GAP_EVT_DATA_LEN_CHANGE_IND: {
		rtk_bt_le_data_len_change_ind_t *data_len_change =
			(rtk_bt_le_data_len_change_ind_t *)param;
		BT_LOGA("[APP] Data len is updated, conn_handle: %d, "       \
				"max_tx_octets: 0x%x, max_tx_time: 0x%x, "        \
				"max_rx_octets: 0x%x, max_rx_time: 0x%x\r\n",
				data_len_change->conn_handle,
				data_len_change->max_tx_octets,
				data_len_change->max_tx_time,
				data_len_change->max_rx_octets,
				data_len_change->max_rx_time);
		BT_AT_PRINT("+BLEGAP:conn_datalen,%d,0x%x,0x%x,0x%x,0x%x\r\n",
					data_len_change->conn_handle,
					data_len_change->max_tx_octets,
					data_len_change->max_tx_time,
					data_len_change->max_rx_octets,
					data_len_change->max_rx_time);
		break;
	}

	case RTK_BT_LE_GAP_EVT_PHY_UPDATE_IND: {
		rtk_bt_le_phy_update_ind_t *phy_update_ind =
			(rtk_bt_le_phy_update_ind_t *)param;
		if (phy_update_ind->err) {
			BT_LOGE("[APP] Update PHY failed, conn_handle: %d, err: 0x%x\r\n",
					phy_update_ind->conn_handle,
					phy_update_ind->err);
			BT_AT_PRINT("+BLEGAP:conn_phy,%d,-1\r\n", phy_update_ind->conn_handle);
		} else {
			BT_LOGA("[APP] PHY is updated, conn_handle: %d, tx_phy: %d, rx_phy: %d\r\n",
					phy_update_ind->conn_handle,
					phy_update_ind->tx_phy,
					phy_update_ind->rx_phy);
			BT_AT_PRINT("+BLEGAP:conn_phy,%d,0,%d,%d\r\n",
						phy_update_ind->conn_handle,
						phy_update_ind->tx_phy,
						phy_update_ind->rx_phy);
		}
		break;
	}

	case RTK_BT_LE_GAP_EVT_AUTH_PAIRING_CONFIRM_IND: {
		rtk_bt_le_auth_pair_cfm_ind_t *pair_cfm_ind =
			(rtk_bt_le_auth_pair_cfm_ind_t *)param;
		BT_LOGA("[APP] Just work pairing need user to confirm, conn_handle: %d!\r\n",
				pair_cfm_ind->conn_handle);
		BT_AT_PRINT("+BLEGAP:pair_cfm,%d\r\n", pair_cfm_ind->conn_handle);
		rtk_bt_le_pair_cfm_t pair_cfm_param = {0};
		uint16_t ret = 0;
		pair_cfm_param.conn_handle = pair_cfm_ind->conn_handle;
		pair_cfm_param.confirm = 1;
		ret = rtk_bt_le_sm_pairing_confirm(&pair_cfm_param);
		if (RTK_BT_OK == ret) {
			BT_LOGA("[APP] Just work pairing auto confirm succcess\r\n");
		}
		break;
	}

	case RTK_BT_LE_GAP_EVT_AUTH_PASSKEY_DISPLAY_IND: {
		rtk_bt_le_auth_key_display_ind_t *key_dis_ind =
			(rtk_bt_le_auth_key_display_ind_t *)param;
		BT_LOGA("[APP] Auth passkey display: %d, conn_handle:%d\r\n",
				key_dis_ind->passkey,
				key_dis_ind->conn_handle);
		BT_AT_PRINT("+BLEGAP:passkey_display,%d,%d\r\n",
					(int)key_dis_ind->conn_handle,
					(int)key_dis_ind->passkey);
		break;
	}

	case RTK_BT_LE_GAP_EVT_AUTH_PASSKEY_INPUT_IND: {
		rtk_bt_le_auth_key_input_ind_t *key_input_ind =
			(rtk_bt_le_auth_key_input_ind_t *)param;
		BT_LOGA("[APP] Please input the auth passkey get from remote, conn_handle: %d\r\n",
				key_input_ind->conn_handle);
		BT_AT_PRINT("+BLEGAP:passkey_input,%d\r\n", key_input_ind->conn_handle);
		break;
	}

	case RTK_BT_LE_GAP_EVT_AUTH_PASSKEY_CONFIRM_IND: {
		rtk_bt_le_auth_key_cfm_ind_t *key_cfm_ind =
			(rtk_bt_le_auth_key_cfm_ind_t *)param;
		BT_LOGA("[APP] Auth passkey confirm: %d, conn_handle: %d. "  \
				"Please comfirm if the passkeys are equal!\r\n",
				key_cfm_ind->passkey,
				key_cfm_ind->conn_handle);
		BT_AT_PRINT("+BLEGAP:passkey_cfm,%d,%d\r\n",
					(int)key_cfm_ind->conn_handle,
					(int)key_cfm_ind->passkey);
		break;
	}

	case RTK_BT_LE_GAP_EVT_AUTH_OOB_KEY_INPUT_IND: {
		rtk_bt_le_auth_oob_input_ind_t *oob_input_ind =
			(rtk_bt_le_auth_oob_input_ind_t *)param;
		BT_LOGA("[APP] Bond use oob key, conn_handle: %d. Please input the oob tk \r\n",
				oob_input_ind->conn_handle);
		BT_AT_PRINT("+BLEGAP:oobkey_input,%d\r\n", oob_input_ind->conn_handle);
		break;
	}

	case RTK_BT_LE_GAP_EVT_AUTH_COMPLETE_IND: {
		rtk_bt_le_auth_complete_ind_t *auth_cplt_ind =
			(rtk_bt_le_auth_complete_ind_t *)param;
		BT_AT_PRINT("+BLEGAP:sec,%d,%d\r\n",
					auth_cplt_ind->conn_handle,
					(auth_cplt_ind->err == 0) ? 0 : -1);
		if (auth_cplt_ind->err) {
			BT_LOGE("[APP] Pairing failed(err: 0x%x), conn_handle: %d\r\n",
					auth_cplt_ind->err, auth_cplt_ind->conn_handle);
		} else {
			BT_LOGA("[APP] Pairing success, conn_handle: %d\r\n", auth_cplt_ind->conn_handle);
			BT_DUMPHEXA("[APP] long term key is 0x", auth_cplt_ind->dev_ltk, auth_cplt_ind->dev_ltk_length, true);
		}
		break;
	}

	case RTK_BT_LE_GAP_EVT_BOND_MODIFY_IND: {
		rtk_bt_le_bond_modify_ind_t *bond_mdf_ind =
			(rtk_bt_le_bond_modify_ind_t *)param;
		char ident_addr[30] = {0};
		rtk_bt_le_addr_to_str(&(bond_mdf_ind->remote_addr), le_addr, sizeof(le_addr));
		rtk_bt_le_addr_to_str(&(bond_mdf_ind->ident_addr), ident_addr, sizeof(ident_addr));
		BT_LOGA("[APP] Bond info modified, op: %d, addr: %s, ident_addr: %s\r\n",
				bond_mdf_ind->op, le_addr, ident_addr);
		BT_AT_PRINT("+BLEGAP:bond_modify,%d,%s,%s\r\n", bond_mdf_ind->op, le_addr, ident_addr);

		break;
	}

#if defined(RTK_BLE_PRIVACY_SUPPORT) && RTK_BLE_PRIVACY_SUPPORT
	case RTK_BT_LE_GAP_EVT_RESOLV_LIST_MODIFY_IND: {
		rtk_bt_le_modify_resolv_list_ind_t *p_ind = (rtk_bt_le_modify_resolv_list_ind_t *)param;
		rtk_bt_le_addr_t addr;

		if (p_ind->op == RTK_BT_LE_RESOLV_LIST_OP_ADD || p_ind->op == RTK_BT_LE_RESOLV_LIST_OP_REMOVE) {
			addr.type = (rtk_bt_le_addr_type_t)p_ind->entry.addr_type;
			memcpy(addr.addr_val, p_ind->entry.addr, RTK_BD_ADDR_LEN);
			rtk_bt_le_addr_to_str(&addr, le_addr, sizeof(le_addr));
			if (p_ind->err) {
				BT_LOGE("[APP] Resolving list %s %s fail, cause:%x.\r\n",
						(p_ind->op == RTK_BT_LE_RESOLV_LIST_OP_ADD) ? "add" : "remove",
						le_addr, p_ind->err);
				BT_AT_PRINT("+BLEGAP:resolv_list_modify,%d,-1\r\n", p_ind->op);
			} else {
				BT_LOGA("[APP] Resolving list %s %s success, %s privacy mode.\r\n",
						(p_ind->op == RTK_BT_LE_RESOLV_LIST_OP_ADD) ? "add" : "remove",
						le_addr, p_ind->entry.device_mode ? "device" : "network");
				BT_AT_PRINT("+BLEGAP:resolv_list_modify,%d,0,%s,%s\r\n",
							p_ind->op, le_addr, p_ind->entry.device_mode ? "device" : "network");
			}
		} else if (p_ind->op == RTK_BT_LE_RESOLV_LIST_OP_CLEAR) {
			if (p_ind->err) {
				BT_LOGE("[APP] Resolving list clear fail, cause:%x.\r\n", p_ind->err);
			} else {
				BT_LOGA("[APP] Resolving list clear success.\r\n");
			}
			BT_AT_PRINT("+BLEGAP:resolv_list_modify,%d,%d\r\n", p_ind->op, (p_ind->err == 0) ? 0 : -1);
		}
		break;
	}

	case RTK_BT_LE_GAP_EVT_RESOLV_LIST_PENDING_IND: {
		rtk_bt_le_resolv_list_pending_ind_t *p_ind = (rtk_bt_le_resolv_list_pending_ind_t *)param;
		BT_LOGA("[APP] WARN: Resolving list modification is pending because of adv(%d) scan(%d) connect(%d)!!!\r\n",
				(p_ind->reason & RTK_BT_LE_RESOLV_LIST_PEND_BY_ADV) ? 1 : 0,
				(p_ind->reason & RTK_BT_LE_RESOLV_LIST_PEND_BY_SCAN) ? 1 : 0,
				(p_ind->reason & RTK_BT_LE_RESOLV_LIST_PEND_BY_CONNECT) ? 1 : 0);
		BT_AT_PRINT("+BLEGAP:resolv_list_pending,%d,%d,%d\r\n",
					(p_ind->reason & RTK_BT_LE_RESOLV_LIST_PEND_BY_ADV) ? 1 : 0,
					(p_ind->reason & RTK_BT_LE_RESOLV_LIST_PEND_BY_SCAN) ? 1 : 0,
					(p_ind->reason & RTK_BT_LE_RESOLV_LIST_PEND_BY_CONNECT) ? 1 : 0);
		break;
	}
#endif

	default:
		BT_LOGE("[APP] Unkown gap cb evt type: %d\r\n", evt_code);
		break;
	}

	return RTK_BT_EVT_CB_OK;
}



static uint16_t app_get_gatts_app_id(uint8_t event, void *data)
{
	uint16_t app_id = 0xFFFF;

	switch (event) {
	case RTK_BT_GATTS_EVT_REGISTER_SERVICE: {
		rtk_bt_gatts_reg_ind_t *p_reg_ind = (rtk_bt_gatts_reg_ind_t *)data;
		app_id = p_reg_ind->app_id;
		break;
	}
	case RTK_BT_GATTS_EVT_READ_IND: {
		rtk_bt_gatts_read_ind_t *p_read_ind = (rtk_bt_gatts_read_ind_t *)data;
		app_id = p_read_ind->app_id;
		break;
	}
	case RTK_BT_GATTS_EVT_WRITE_IND: {
		rtk_bt_gatts_write_ind_t *p_write_ind = (rtk_bt_gatts_write_ind_t *)data;
		app_id = p_write_ind->app_id;
		break;
	}
	case RTK_BT_GATTS_EVT_CCCD_IND: {
		rtk_bt_gatts_cccd_ind_t *p_cccd_ind = (rtk_bt_gatts_cccd_ind_t *)data;
		app_id = p_cccd_ind->app_id;
		break;
	}
	case RTK_BT_GATTS_EVT_NOTIFY_COMPLETE_IND:
	case RTK_BT_GATTS_EVT_INDICATE_COMPLETE_IND: {
		rtk_bt_gatts_ntf_and_ind_ind_t *p_ind_ntf = (rtk_bt_gatts_ntf_and_ind_ind_t *)data;
		app_id = p_ind_ntf->app_id;
		break;
	}
	default:
		break;
	}

	return app_id;
}

static rtk_bt_evt_cb_ret_t ble_peripheral_gatts_app_callback(uint8_t event, void *data, uint32_t len)
{
	(void)len;
	uint16_t app_id = 0xFFFF;

	if (RTK_BT_GATTS_EVT_MTU_EXCHANGE == event) {
		rtk_bt_gatt_mtu_exchange_ind_t *p_gatt_mtu_ind = (rtk_bt_gatt_mtu_exchange_ind_t *)data;
		if (p_gatt_mtu_ind->result == RTK_BT_OK) {
			BT_LOGA("[APP] GATTS mtu exchange successfully, mtu_size: %d, conn_handle: %d \r\n",
					p_gatt_mtu_ind->mtu_size, p_gatt_mtu_ind->conn_handle);
		} else {
			BT_LOGE("[APP] GATTS mtu exchange fail \r\n");
		}
		return RTK_BT_EVT_CB_OK;
	}

	if (RTK_BT_GATTS_EVT_CLIENT_SUPPORTED_FEATURES == event) {
		rtk_bt_gatts_client_supported_features_ind_t *p_ind = (rtk_bt_gatts_client_supported_features_ind_t *)data;
		if (p_ind->features & RTK_BT_GATTS_CLIENT_SUPPORTED_FEATURES_EATT_BEARER_BIT) {
			BT_LOGA("[APP] Client Supported features is writed: conn_handle %d, features 0x%02x. Remote client supports EATT.\r\n",
					p_ind->conn_handle, p_ind->features);
		}
		return RTK_BT_EVT_CB_OK;
	}

	app_id = app_get_gatts_app_id(event, data);
	switch (app_id) {
	case SIMPLE_BLE_SRV_ID:
		simple_ble_service_callback(event, data);
		break;
	// case DEVICE_INFO_SRV_ID:
	// 	device_info_srv_callback(event, data);
	// 	break;
	// case HEART_RATE_SRV_ID:
	// 	heart_rate_srv_callback(event, data);
	// 	break;
	// case BATTERY_SRV_ID:
	// 	battery_service_callback(event, data);
	// 	break;
	// case IMMEDIATE_ALERT_SRV_ID:
	// 	immediate_alert_srv_callback(event, data);
	// 	break;
	// case GLUCOSE_SRV_ID:
	// 	glucose_srv_callback(event, data);
	// 	break;
	case HID_SRV_ID:
		hid_srv_callback(event, data);
		break;
// 	case LONG_UUID_SRV_ID:
// 		long_uuid_service_callback(event, data);
// 		break;
// 	case CTE_SRV_ID:
// #if defined(RTK_BLE_5_1_CTE_SUPPORT) && RTK_BLE_5_1_CTE_SUPPORT
// 		cte_srv_callback(event, data);
// #endif
// 		break;

	default:
		break;
	}

	return RTK_BT_EVT_CB_OK;
}
int ble_peripheral_main(uint8_t enable)
{
	rtk_bt_app_conf_t bt_app_conf = {0};
	rtk_bt_le_addr_t bd_addr = {(rtk_bt_le_addr_type_t)0, {0}};
	bool adv_filter_whitelist = false;
	char addr_str[30] = {0};
	char name[30] = {0};
	rtk_bt_le_adv_param_t adv_param = {0};


	if (1 == enable) {
		//set GAP configuration
		bt_app_conf.app_profile_support = RTK_BT_PROFILE_GATTS;
		bt_app_conf.mtu_size = 180;
		bt_app_conf.master_init_mtu_req = true;
		bt_app_conf.slave_init_mtu_req = false;
		bt_app_conf.prefer_all_phy = 0;
		bt_app_conf.prefer_tx_phy = 1 | 1 << 1 | 1 << 2;
		bt_app_conf.prefer_rx_phy = 1 | 1 << 1 | 1 << 2;
		bt_app_conf.max_tx_octets = 0x40;
		bt_app_conf.max_tx_time = 0x200;
		bt_app_conf.user_def_service = false;
		bt_app_conf.cccd_not_check = false;

		/* Enable BT */
		BT_APP_PROCESS(rtk_bt_enable(&bt_app_conf));
		BT_APP_PROCESS(rtk_bt_le_gap_get_bd_addr(&bd_addr));
		rtk_bt_le_addr_to_str(&bd_addr, addr_str, sizeof(addr_str));
		BT_LOGA("[APP] BD_ADDR: %s\r\n", addr_str);

		BT_APP_PROCESS(rtk_bt_evt_register_callback(RTK_BT_COMMON_GP_GAP, peripheral_gap_app_callback));
		BT_APP_PROCESS(rtk_bt_evt_register_callback(RTK_BT_LE_GP_GAP, ble_peripheral_gap_app_callback));
		memcpy(name, (const char *)RTK_BT_DEV_NAME, strlen((const char *)RTK_BT_DEV_NAME));
		BT_APP_PROCESS(rtk_bt_le_gap_set_device_name((uint8_t *)name));
		BT_APP_PROCESS(rtk_bt_le_gap_set_appearance(RTK_BT_LE_GAP_APPEARANCE_HEART_RATE_BELT));

		BT_APP_PROCESS(rtk_bt_le_sm_set_security_param(&sec_param));

		memcpy(&adv_param, &def_adv_param, sizeof(rtk_bt_le_adv_param_t));

		BT_APP_PROCESS(rtk_bt_evt_register_callback(RTK_BT_LE_GP_GATTS, ble_peripheral_gatts_app_callback));
		BT_APP_PROCESS(simple_ble_srv_add());
		// BT_APP_PROCESS(device_info_srv_add());
		// BT_APP_PROCESS(heart_rate_srv_add());
		// BT_APP_PROCESS(battery_srv_add());
		// BT_APP_PROCESS(immediate_alert_srv_add());
		// BT_APP_PROCESS(glucose_srv_add());
		// BT_APP_PROCESS(long_uuid_srv_add());

		BT_APP_PROCESS(rtk_bt_le_gap_set_adv_data(adv_data, sizeof(adv_data)));
		BT_APP_PROCESS(rtk_bt_le_gap_set_scan_rsp_data(scan_rsp_data, sizeof(scan_rsp_data)));
		if (adv_filter_whitelist) {
			adv_param.filter_policy = RTK_BT_LE_ADV_FILTER_ALLOW_SCAN_WLST_CON_WLST;
		}
		BT_APP_PROCESS(rtk_bt_le_gap_start_adv(&adv_param));


	} else if (0 == enable) {

		app_server_deinit();

		/* no need to unreg callback here, it is done in rtk_bt_disable */
		// BT_APP_PROCESS(rtk_bt_evt_unregister_callback(RTK_BT_LE_GP_GATTS));

		/* Disable BT */
		BT_APP_PROCESS(rtk_bt_disable());
	}

	return 0;
}



static void example_start_captive_portal(void *param)
{
	/* To avoid gcc warnings */
	(void) param;

	rtos_time_delay_ms(500);

	ble_peripheral_main(1);

	goto exit;

#ifdef CONFIG_LWIP_LAYER
	struct netif *pnetif = &xnetif[SOFTAP_WLAN_INDEX];
	u32 ip_addr;
	u32 netmask;
	u32 gw;
#endif

	int timeout = 20;

#ifdef CONFIG_LWIP_LAYER
	dhcps_deinit();
	ip_addr = CONCAT_TO_UINT32(AP_IP_ADDR0, AP_IP_ADDR1, AP_IP_ADDR2, AP_IP_ADDR3);
	netmask = CONCAT_TO_UINT32(AP_NETMASK_ADDR0, AP_NETMASK_ADDR1, AP_NETMASK_ADDR2, AP_NETMASK_ADDR3);
	gw = CONCAT_TO_UINT32(AP_GW_ADDR0, AP_GW_ADDR1, AP_GW_ADDR2, AP_GW_ADDR3);
	LwIP_SetIP(SOFTAP_WLAN_INDEX, ip_addr, netmask, gw);
#endif

	RTK_LOGE(NOTAG, "Enter start captive_portal!\n");

	RTK_LOGE(NOTAG, "Enable Wi-Fi with STA + AP mode\n");
	if (wifi_on(RTW_MODE_STA) < 0) {
		RTK_LOGE(NOTAG, "wifi_on failed\n");
		return;
	}

	/*********************************************************************************
	*	1-3. Start AP
	*********************************************************************************/
	SOFTAP_CONFIG.ssid.len = strlen(SOFTAP_SSID);
	memcpy(SOFTAP_CONFIG.ssid.val, SOFTAP_SSID, SOFTAP_CONFIG.ssid.len);
	SOFTAP_CONFIG.password = (unsigned char *)SOFTAP_PASSWORD;
	SOFTAP_CONFIG.password_len = strlen(SOFTAP_PASSWORD);
	SOFTAP_CONFIG.security_type = SOFTAP_SECURITY;
	SOFTAP_CONFIG.channel = SOFTAP_CHANNEL;

	if (wifi_start_ap(&SOFTAP_CONFIG) < 0) {
		RTK_LOGE(NOTAG, "wifi_start_ap failed\n");
		return;
	}
	RTK_LOGI(NOTAG, "Check AP running\n");

	while (1) {
		struct rtw_wifi_setting setting;
		wifi_get_setting(SOFTAP_WLAN_INDEX, &setting);
		if (strlen((const char *)setting.ssid) > 0) {
			if (strcmp((const char *) setting.ssid, (const char *)SOFTAP_CONFIG.ssid.val) == 0) {
				RTK_LOGI(NOTAG, "%s started\n", SOFTAP_CONFIG.ssid.val);
				break;
			}
		}

		if (timeout == 0) {
			RTK_LOGI(NOTAG, "ERROR: Start AP timeout!");
			break;
		}

		rtos_time_delay_ms(1 * RTOS_TICK_RATE_HZ);
		timeout --;
	}

#ifdef CONFIG_LWIP_LAYER
	dhcps_init(pnetif);
#endif

	scan_result.ap_list = (ap_list_t *)malloc(2 * SCAN_AP_LIST_MAX * sizeof(ap_list_t));
	if (scan_result.ap_list == NULL) {
		RTK_LOGE(NOTAG, "ERROR: scan_result.ap_list malloc failed!\n");
		goto exit;
	}

	rtos_sema_create(&scan_done_sema, 0, RTOS_SEMA_MAX_COUNT);
	wifi_start_scan();
	rtos_sema_take(scan_done_sema, RTOS_MAX_DELAY);
	RTK_LOGI(NOTAG, "wifi scan done\n");

	webs_terminate = 0;
	if (webs_task == NULL) {
		if (rtos_task_create(&webs_task, (const char *)"web_server", vCaptivePortalServer, NULL, STACKSIZE * 4, 1) != RTK_SUCCESS) {
			RTK_LOGE(NOTAG, "Create webserver task failed!\n");
		}
	}
	if (webs_sema == NULL) {
		rtos_sema_create(&webs_sema, 0, 0xffffffff);	//Set max count 0xffffffff
	}
	if (webs_wpage_sema == NULL) {
		rtos_sema_create_binary(&webs_wpage_sema);
	}
	rtos_sema_give(webs_wpage_sema);
exit:
	rtos_task_delete(NULL);
}

void stop_captive_portal(void)
{
	webs_terminate = 1;
	if (pxHTTPListener) {
		netconn_abort(pxHTTPListener);
	}
	if (webs_sema) {
		if (rtos_sema_take(webs_sema, 15 * RTOS_TICK_RATE_HZ) != RTK_SUCCESS) {
			if (pxHTTPListener) {
				netconn_close(pxHTTPListener);
				netconn_delete(pxHTTPListener);
				pxHTTPListener = NULL;
			}
			RTK_LOGE(NOTAG, "Take webs sema(%p) failed!!!!!!!!!!!\n", webs_sema);
		}
		rtos_sema_delete(webs_sema);
		webs_sema = NULL;
	}
	if (webs_wpage_sema) {
		rtos_sema_take(webs_wpage_sema, 15 * RTOS_TICK_RATE_HZ);
		rtos_sema_delete(webs_wpage_sema);
		webs_wpage_sema = NULL;
	}
	if (webs_task) {
		rtos_task_delete(webs_task);
		webs_task = NULL;
	}
	web_conn_clear();

	RTK_LOGI(NOTAG, "Exit stop captive_portal!\n");
}

void example_captive_portal(void)
{
	if (rtos_task_create(NULL, ((const char *)"example_start_captive_portal"), example_start_captive_portal, NULL, 1024 * 4, 1) != RTK_SUCCESS) {
		RTK_LOGE(NOTAG, "%s rtos_task_create failed\n", __FUNCTION__);
	}
}

