

/////////////////////////////////////////////////////////////////////////////////////
#include "config.h"
#include "Fun_wifi.h"

#include "Fun_mqtt.h"
#include "Fun_e2p.h"
#include "Fun_teaSeq.h"
#include "Fun_operation.h"
#include "Fun_nfc.h"

#include "Ui_lcd.h"

#include <ESP8266WebServer.h>
#include <ESP8266httpUpdate.h>
#include <ESP8266HTTPUpdateServer.h>
/////////////////////////////////////////////////////////////////////////////////////
#define INTERVAL_RSSI 15000 // ms
#define INTERVAL_SCAN 15000 // ms
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;

WiFiEventHandler handleStaConnected;
WiFiEventHandler handleStaGotIp;
WiFiEventHandler handleStaDisConnected;
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
Fun_wifi wifi;
/////////////////////////////////////////////////////////////////////////////////////
static void onWifiConnected(const WiFiEventStationModeConnected &event)
{
    wifi.ucChkErrIP = 1;
    wifi.ulMilGetIP = millis();
    wifi.ucErr = WIFI_NO_ERR;

#ifdef ESP_DEBUG_WIFI
    Serial.printf("onWifiConnected\r\n");
#endif
}
/////////////////////////////////////////////////////////////////////////////////////
static void onWifiGotIP(const WiFiEventStationModeGotIP &event)
{
    wifi.sIP = WiFi.localIP().toString();
    wifi.sMacAP = WiFi.BSSIDstr();
    wifi.lRssi = WiFi.RSSI();
    wifi.rssi_percent = (uint8_t)map(wifi.lRssi, wifi.arRssi[0], wifi.arRssi[3], 100, 0);
    if (wifi.rssi_percent > 100)
        wifi.rssi_percent = 100;

    wifi.ucErr = WIFI_NO_ERR;
    wifi.ucGotIP = 1;
    wifi.ucChkErrIP = 0;

    lcd_refresh();

#ifdef ESP_DEBUG_WIFI
    Serial.printf("onWifiGotIP, IP: %s, rssi: %ld\r\n", wifi.sIP.c_str(), wifi.lRssi);
#endif
}
/////////////////////////////////////////////////////////////////////////////////////
static void onWifiDisconnected(const WiFiEventStationModeDisconnected &event)
{
    wifi.ucErr = WIFI_ERR_AP;
    wifi.lRssi = -100;
    wifi.sIP = String("-");
    wifi.rssi_percent = 0;

    if (mqtt.bConnected)
    {
        mqtt_disconnect();
    }

    lcd_refresh();

#ifdef ESP_DEBUG_WIFI
    Serial.printf("onWifiDisconnected\r\n");
#endif
}
/////////////////////////////////////////////////////////////////////////////////////
void NOT_FOUND_API()
{
    String message = "Not Found\n\n";
    message += "URI: ";
    message += httpServer.uri();
    message += "\nMethod: ";
    message += (httpServer.method() == HTTP_GET) ? "GET" : "POST";
    message += "\nArguments: ";
    message += httpServer.args();
    message += "\n";
    for (uint8_t i = 0; i < httpServer.args(); i++)
        message += " " + httpServer.argName(i) + ": " + httpServer.arg(i) + "\n";

    httpServer.send(404, "text/plain", message);
}
/////////////////////////////////////////////////////////////////////////////////////
void handleRoot()
{
    String header = "HTTP/1.1 301 OK\r\nLocation: /config\r\nCache-Control: no-cache\r\n\r\n";
    httpServer.sendContent(header);
}
/////////////////////////////////////////////////////////////////////////////////////
Fun_wifi::Fun_wifi()
{
}
/////////////////////////////////////////////////////////////////////////////////////
Fun_wifi::~Fun_wifi()
{
}
/////////////////////////////////////////////////////////////////////////////////////
void Fun_wifi::init()
{
    WiFi.mode(WIFI_AP_STA);
    WiFi.enableAP(0);
    WiFi.enableSTA(1);
    WiFi.disconnect();
    WiFi.setAutoReconnect(true);
    // sMac = String(WiFi.macAddress());
    sIP = String("-");
    lRssi = -100;
    rssi_percent = 0;

    handleStaConnected = WiFi.onStationModeConnected(&onWifiConnected);
    handleStaGotIp = WiFi.onStationModeGotIP(&onWifiGotIP);
    handleStaDisConnected = WiFi.onStationModeDisconnected(&onWifiDisconnected);

    init_websvr();
    mqtt.init();

    // sSsid = "SVG RND";
    // sPassword = "svgims12345";

    // scanAndCheckNetwork();
}
/////////////////////////////////////////////////////////////////////////////////////
void Fun_wifi::loop()
{
    loop_websvr();

    if (WiFi.status() == WL_CONNECTED)
    {
        ////////////////////////////////////////////////////
        mqtt.loop();
        ////////////////////////////////////////////////////
        if (bUpdateOTA)
        {
#ifdef ESP_DEBUG_WIFI
            Serial.printf("call OTA from MQTT\r\nURL: %s\r\n", sUrlOTA.c_str());
#endif
            bUpdateOTA = false;

            WiFiClient client;
            ESPhttpUpdate.update(client, sUrlOTA);
        }
        else if (wifi.bOtaByHost)
        {
            wifi.bOtaByHost = false;

            WiFiClient client;
            ESPhttpUpdate.update(client, wifi.sUrlByHost);
        }
        ////////////////////////////////////////////////////
        if (millis() - ulMillis_rssi > INTERVAL_RSSI)
        {
            lRssi = WiFi.RSSI();
            rssi_percent = (uint8_t)map(wifi.lRssi, wifi.arRssi[0], wifi.arRssi[3], 100, 0);
            if (rssi_percent > 100)
                rssi_percent = 100;

            ulMillis_rssi = millis();

            lcd_refresh();
        }
        ////////////////////////////////////////////////////
    }
    else
    {
        if (millis() - ulMillis_scan > INTERVAL_SCAN)
        {
            // if (scanAndCheckNetwork())
            // {
            //     connectToWifi();
            // }

            connectToWifi();

            ulMillis_scan = millis();
        }
    }
}
/////////////////////////////////////////////////////////////////////////////////////
bool Fun_wifi::scanAndCheckNetwork()
{
    int8_t scanResult;
    scanResult = WiFi.scanNetworks();
    delay(10);

#ifdef ESP_DEBUG_WIFI
    Serial.printf("sSsid: %s, sPassword: %s\r\n", sSsid.c_str(), sPassword.c_str());
#endif

    if (scanResult > 0)
    {
        sScanWifiInfo = "";
        sScanWifiInfo += String("scan: ") + String(scanResult) + String(" // ");
        for (int8_t i = 0; i < scanResult; ++i)
        {
            String ssid_scan;
            int32_t rssi_scan;
            uint8_t sec_scan;
            uint8_t *BSSID_scan;
            int32_t chan_scan;
            bool hidden;

            WiFi.getNetworkInfo(i, ssid_scan, sec_scan, rssi_scan, BSSID_scan, chan_scan, hidden);

#ifdef ESP_DEBUG_WIFI
            Serial.printf("ssid: %s, rssi: %d\r\n", ssid_scan.c_str(), rssi_scan);
#endif

            char info[64];
            sprintf(info, "%s, %d, %d, %d // ", ssid_scan.c_str(), rssi_scan, chan_scan, hidden);
            sScanWifiInfo += String(info);
            bSendScanWifi = true;

            // have AP
            if (ssid_scan == wifi.sSsid)
            {
                if (rssi_scan < WIFI_RSSI_LIMIT) // low rssi
                {
                    // ucErr = WIFI_ERR_RSSI;
                }
                else
                {
                    sScanWifiInfo += String("<= OKKK // ");
                    // return true;
                }
            }
            // don't have AP
            else
            {
                ucErr = WIFI_ERR_AP;
                lRssi = -100;
            }
        }
    }
    // wifi.ssid != my_ssid || wifi.rssi < -80 db

    return false;
}
/////////////////////////////////////////////////////////////////////////////////////
void Fun_wifi::connectToWifi()
{
    if (sPassword != String("."))
        WiFi.begin(String(sSsid).c_str(), String(sPassword).c_str());
    else
        WiFi.begin(String(sSsid).c_str());
}
/////////////////////////////////////////////////////////////////////////////////////
void Fun_wifi::loop_websvr()
{
    httpServer.handleClient();
}
/////////////////////////////////////////////////////////////////////////////////////
void Fun_wifi::enable_webConfig()
{
    WiFi.enableAP(1);
    // WiFi.enableSTA(1);
    // WiFi.disconnect(true);

    String AP_SSID = String("SVG-IOT-") + sMac;
    String AP_PASSWORD = "11111111";

    IPAddress apIP(192, 168, 4, 1);
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    WiFi.softAP(AP_SSID.c_str(), AP_PASSWORD.c_str());
}
///////////////////////////////////////////////////////////////////////////////////
bool Fun_wifi::WEB_SVR_SB_COMM()
{
    String SSID = httpServer.arg("SSID");
    wifi.sSsid = SSID;

    String PASSWORD = httpServer.arg("PASSWORD");
    wifi.sPassword = PASSWORD;

    String SVR_ADDRESS = httpServer.arg("SVR_ADDRESS");
    mqtt.sSvrAddr = SVR_ADDRESS;

    String SVR_PORT = httpServer.arg("SVR_PORT");
    mqtt.usSvrPort = SVR_PORT.toInt();

    e2p_network.StringToE2P(E2PE_ADR_SSID, E2PE_LEN_SSID, wifi.sSsid);
    e2p_network.StringToE2P(E2PE_ADR_PASS, E2PE_LEN_PASS, wifi.sPassword);
    e2p_network.StringToE2P(E2PE_ADR_SVR_ADDR, E2PE_LEN_SVR_ADDR, mqtt.sSvrAddr);
    e2p_network.write_16(E2PE_ADR_SVR_PORT, mqtt.usSvrPort);
    e2p_network.commit_flag = true;

    return true;
}
/////////////////////////////////////////////////////////////////////////////////////
bool Fun_wifi::WEB_SVR_SB_SET_DELAY_SEN0()
{
    String ucDelay = httpServer.arg("SET_DELAY");
    tea.delay_sen0 = ucDelay.toInt();

    if (tea.delay_sen0 > Fun_teaSeq::TEA_MAX_DLY_SEN0) // more than 2000ms
        tea.delay_sen0 = Fun_teaSeq::TEA_MAX_DLY_SEN0;

    String ENABLE = httpServer.arg("CHECK_BOX");
    if (ENABLE == "CHECKED")
        oper.ucNonSewingMc = 1;
    else
        oper.ucNonSewingMc = 0;

    e2p_network.write(E2PE_ADR_NON_SEW_MC, oper.ucNonSewingMc);
    e2p_info.write_16(E2PE_ADR_TEA_DLY_SEN0, tea.delay_sen0);

    e2p_info.commit_flag = true;
    e2p_network.commit_flag = true;

    return true;
}
/////////////////////////////////////////////////////////////////////////////////////
void Fun_wifi::makeUiWebServer()
{
    String sHtml = R"(
      <html>
        <head><meta charset="utf-8"><title>NFC Select Type</title><style>
          .tab {overflow: hidden;border: 1px solid #ccc;background-color: #f1f1f1;}
          .tab input {background-color: inherit;float: left;border: none;outline: none;cursor: pointer;padding: 14px 16px;transition: 0.3s;font-size: 17px;}
          .tab input:hover {background-color: #ddd;}
          .tab input.active {background-color: #ccc;}
          .tabcontent {display: none;padding: 6px 12px;border: 1px solid #ccc;border-top: none;}
          .tabcontent.active {display:block;}</style>
        </head>)";
    sHtml += R"(<body><p><h3>Select type:</h3></p>)";
    sHtml += "<div class=\"tab\">";

    sHtml += "<input type=\"button\" class=\"tablinks\" onclick=\"openType(event, 'Network')\"  value=\"NETWORK\"     >";
    sHtml += "<input type=\"button\" class=\"tablinks\" onclick=\"openType(event, 'SetDelay')\" value=\"SETTING MC\"  >";
    sHtml += "</div>";

    sHtml += "<div id=\"Network\" class=\"tabcontent\"><h3>NETWORK</h3>";
    sHtml += "<form method=\"post\" action=\"/config\"><table>";
    sHtml += "<tr><td>SSID:</td><td><input      type=\"text\"   name=\"SSID\"         maxlength=\"16\" value=\"" + wifi.sSsid + "\"></td></tr>";
    sHtml += "<tr><td>Password:</td><td><input  type=\"text\"   name=\"PASSWORD\"     maxlength=\"16\" value=\"" + wifi.sPassword + "\"></td></tr>";
    sHtml += "<tr><td>SVR addr:</td><td><input  type=\"text\"   name=\"SVR_ADDRESS\"  maxlength=\"32\" value=\"" + mqtt.sSvrAddr + "\"></td></tr>";
    sHtml += "<tr><td>SVR port:</td><td><input  type=\"number\" name=\"SVR_PORT\"             value=\"" + String(mqtt.usSvrPort) + "\"></td></tr>";
    sHtml += "</table><input name=\"SM_NET\" type=\"submit\" value=\"Submit\">";
    sHtml += "</form></div>"; // end table Network

    sHtml += "<div id=\"SetDelay\" class=\"tabcontent\"><h3>SETTING MC</h3>";
    sHtml += "<form method=\"post\" action=\"/config\"><table>";
    sHtml += "<tr><td>Delay:</td><td><input  type=\"number\" name=\"SET_DELAY\" value=\"" + String(tea.delay_sen0) + "\"></td></tr>";
    if (oper.ucNonSewingMc == 1)
        sHtml += "<tr><td>Non-sewing:</td><td><input type=\"checkbox\" name=\"CHECK_BOX\" value=\"CHECKED\" checked></td></tr>";
    else
        sHtml += "<tr><td>Non-sewing:</td><td><input type=\"checkbox\" name=\"CHECK_BOX\" value=\"CHECKED\"></td></tr>";
    sHtml += "</table><input name=\"SM_SEN0\" type=\"submit\" value=\"Submit\">";
    sHtml += "</form></div>"; // end table set delay sen0

    sHtml += R"(<script>
      function openType(evt, typeName) {
        var i, tabcontent, tablinks;
        tabcontent = document.getElementsByClassName("tabcontent");
        for (i = 0; i < tabcontent.length; i++) {
          tabcontent[i].style.display = "none";
        }
        tablinks = document.getElementsByClassName("tablinks");
        for (i = 0; i < tablinks.length; i++) {
          tablinks[i].className = tablinks[i].className.replace(" active", "");
        }
        document.getElementById(typeName).style.display = "block";
        evt.currentTarget.className += " active";
      }
      </script><br>
      <form method="post"><input name="FINISH" type ="submit" value="Finish"></form>
      </body></html>
      )";

    httpServer.sendContent(sHtml);
}
/////////////////////////////////////////////////////////////////////////////////////
void page_config()
{
    if (httpServer.hasArg("SM_NET"))
    {
        wifi.WEB_SVR_SB_COMM();

        nfc.ucReadType = NFC_TAG_TYPE_NETWORK;
        lcd.disp_nfc_tag_read();
    }

    else if (httpServer.hasArg("SM_SEN0"))
        wifi.WEB_SVR_SB_SET_DELAY_SEN0();
    else if (httpServer.hasArg("FINISH"))
        Inf.ucFlagESPRestart = 1;

    wifi.makeUiWebServer();
}
/////////////////////////////////////////////////////////////////////////////////////
void handleDebug()
{
    if (wifi.bStartWebConfig != 0)
        return;

    calDapot(); // update power ontime

    String sLocation = oper.factory + "-" + oper.line + "-" + String(oper.position);

    String sHtml = "<html><body>";
    sHtml += "<table>";
    ///////////////
    sHtml += "<tr><td colspan='2'>&nbsp;</td></tr>";
    sHtml += "<tr><td>Version:</td><td>" + Iot.sVer + "</td></tr>";
    sHtml += "<tr><td>Mac adr:</td><td>" + wifi.sMac + "</td></tr>";
    sHtml += "<tr><td>M/c IP:</td><td>" + wifi.sIP + "</td></tr>";
    sHtml += "<tr><td>Location:</td><td>" + sLocation + "</td></tr>";
    sHtml += "<tr><td colspan='2'>&nbsp;</td></tr>";

    sHtml += "<tr><th colspan='2' style='text-align:left'>Output</th></tr>";
    sHtml += "<tr><td>output:</td><td>" + String(oper.output) + "</td></tr>";
    sHtml += "<tr><td>defect:</td><td>" + String(oper.defect) + "</td></tr>";
    sHtml += "<tr><td>target:</td><td>" + String(oper.target) + "</td></tr>";
    sHtml += "<tr><td colspan='2'>&nbsp;</td></tr>";

    sHtml += "<tr><th colspan='2' style='text-align:left'>Network</th></tr>";
    sHtml += "<tr><td>Ssid:</td><td>" + wifi.sSsid + "</td></tr>";
    sHtml += "<tr><td>Psk:</td><td>" + wifi.sPassword + "</td></tr>";
    sHtml += "<tr><td>SvrAddr:</td><td>" + mqtt.sSvrAddr + "</td></tr>";
    sHtml += "<tr><td>Port:</td><td>" + String(mqtt.usSvrPort) + "</td></tr>";
    sHtml += "<tr><td>Rssi:</td><td>" + String(WiFi.RSSI()) + "</td></tr>";
    sHtml += "<tr><td colspan='2'>&nbsp;</td></tr>";

    sHtml += "<tr><th colspan='2' style='text-align:left' >Machine data</th></tr>";
    sHtml += "<tr><td>Power ontime:</td><td>" + String(oper.power_ontime) + "</td></tr>";
    sHtml += "<tr><td>Mot runtime:</td><td>" + String(oper.motor_runtime / 1000) + "</td></tr>";
    sHtml += "<tr><td>Handling time:</td><td>" + String(oper.handling_time / 1000) + "</td></tr>";
    sHtml += "<tr><td>Active time:</td><td>" + String((oper.handling_time + oper.motor_runtime) / 1000) + "</td></tr>";
    sHtml += "<tr><td>Sum cyc time:</td><td>" + String(oper.cycle_time) + "</td></tr>";
    sHtml += "<tr><td>Cur cyc time:</td><td>" + String(oper.current_cycle_time) + "</td></tr>";
    sHtml += "<tr><td>Cur mot runtime:</td><td>" + String(oper.ulCurMotRunMs) + "</td></tr>";
    sHtml += "<tr><td>SUM_STI:</td><td>" + String(oper.sumof_stitch) + "</td></tr>";
    sHtml += "<tr><td>SUM_TRI:</td><td>" + String(oper.sumof_trim) + "</td></tr>";
    sHtml += "<tr><td>CurIndxPlan:</td><td>" + String(oper.current_idx_plan) + "</td></tr>";
    sHtml += "<tr><td>Process ID:</td><td>" + oper.process_id + "</td></tr>";
    sHtml += "<tr><td>Cur Han_time:</td><td>" + String(oper.handling_time_idx_plan) + "</td></tr>";
    sHtml += "<tr><td colspan='2'>&nbsp;</td></tr>";

    sHtml += "<tr><th colspan='2' style='text-align:left'>Teaching</th></tr>";
    for (int i = 0; i < (E2PE_LEN_TEA_SEQ - 1) / 3; i++)
    {
        if (tea.ucTeaSeqStore[oper.current_idx_plan][i * 3] == TEA_SEQ_END_ID)
            break;
        String sName = mqtt.getSeqName(tea.ucTeaSeqStore[oper.current_idx_plan][i * 3]);
        if (sName == "XXX")
            continue;
        uint16_t usValue;
        usValue = (uint16_t)(tea.ucTeaSeqStore[oper.current_idx_plan][i * 3 + 1] << 8);
        usValue += (uint16_t)(tea.ucTeaSeqStore[oper.current_idx_plan][i * 3 + 2]);

        sHtml += "<tr><td>" + sName + ":</td><td>" + String(usValue) + "</td></tr>";
    }
    sHtml += "<tr><td colspan='2'>&nbsp;</td></tr>";

    sHtml += "<tr><th colspan='2' style='text-align:left'>Memory Heap: " + String(ESP.getFreeHeap()) + "</th></tr>";
    sHtml += "<tr><td colspan='2'>&nbsp;</td></tr>";

    sHtml += "<tr><th colspan='2' style='text-align:left'>Data</th></tr>";
    sHtml += "<tr><td>Scalet:</td><td>" + String(oper.scale) + "</td></tr>";
    sHtml += "<tr><td>Daily rst output:</td><td>" + String(oper.daily_product_reset) + "</td></tr>";
    sHtml += "<tr><td>Non-sewing mc?:</td><td>" + String(oper.ucNonSewingMc) + "</td></tr>";
    sHtml += "<tr><td>Delay sen0:</td><td>" + String(tea.delay_sen0) + "</td></tr>";
    sHtml += "<tr><td colspan='2'>&nbsp;</td></tr>";

    ///////////////
    sHtml += "</table>";
    sHtml += "</body></html>";

    httpServer.sendContent(sHtml);
}
/////////////////////////////////////////////////////////////////////////////////////
void Fun_wifi::init_websvr()
{
    httpUpdater.setup(&httpServer);

    httpServer.onNotFound(NOT_FOUND_API);
    httpServer.on("/", handleRoot);
    httpServer.on("/config", page_config);
    httpServer.on("/debug", handleDebug);

    httpServer.begin();
}
/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////