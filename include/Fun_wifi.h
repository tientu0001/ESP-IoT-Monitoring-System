
#ifndef FUN_WIFI_H
#define FUN_WIFI_H
/////////////////////////////////////////////////////////////////////////////////////
#include <ESP8266WiFi.h>

#define WIFI_RSSI_LIMIT -100 // limit rssi

#define WIFI_CNT_CONNECT_LIMIT 3 // count try connect to wifi
#define WIFI_INTERCAL_CONNECT 15 // 15 seconds

// static void onWifiConnected(const WiFiEventStationModeConnected &event);
// static void onWifiGotIP(const WiFiEventStationModeGotIP &event);
// static void onWifiDisconnected(const WiFiEventStationModeDisconnected &event);

class Fun_wifi;

class Fun_wifi
{
private:
    bool scanAndCheckNetwork();
    void connectToWifi();

    void init_websvr();
    void loop_websvr();

    uint32_t ulMillis_rssi;
    uint32_t ulMillis_scan;

public:
    // functions
    Fun_wifi();
    ~Fun_wifi();

    void init();
    void loop();

    void enable_webConfig();

    bool WEB_SVR_SB_COMM();
    bool WEB_SVR_SB_SET_DELAY_SEN0();
    void makeUiWebServer();

    // variables
    String sMac;
    String sIP;
    String sMacAP;

    String sSsid;
    String sPassword;

    long lRssi;
    uint8_t rssi_percent;
    long arRssi[4];

    uint8_t ucErr;
    uint8_t ucChkErrIP;
    uint8_t ucGotIP;
    uint32_t ulMilGetIP;

    // uint8_t ucShowErrRssi;

    // uint8_t ucSuccessFlag;
    // uint8_t ucChkCreateWebSvr;
    uint8_t bStartWebConfig;

    // // add for control esp from server
    // String sUiControlData;
    // uint8_t ucSubmitWebconfig;

    bool bUpdateOTA;
    String sUrlOTA;

    bool bOtaByHost;
    String sUrlByHost;

    bool bScanWifiInfo;
    bool bSendScanWifi;
    String sScanWifiInfo;
};
/////////////////////////////////////////////////////////////////////////////////////
extern Fun_wifi wifi;
/////////////////////////////////////////////////////////////////////////////////////
#endif // FUN_WIFI_H
