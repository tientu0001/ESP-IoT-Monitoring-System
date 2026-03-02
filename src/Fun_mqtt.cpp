

#include "config.h"
#include "Fun_mqtt.h"

#include "Fun_time.h"
#include "Fun_wifi.h"
#include "Fun_operation.h"
#include "Fun_nfc.h"
#include "Fun_teaSeq.h"

#include "Fun_e2p.h"

#include "Ui_lcd.h"
#include "Ui_button.h"

#include <ArduinoJson.h>

#include <list>
#include <string>

#include <Ticker.h>
Ticker mqttReconnectTimer;

/////////////////////////////////////////////////////////////////////////////////////
#include "ESP8266Ping.h"
bool bPing2Host;
String hostPing;
/////////////////////////////////////////////////////////////////////////////////////
#define TOPIC_SYCTRL "/SysCtrl"

#define TOPIC_EVT_PRODUCT "/Event/Cnt"
#define TOPIC_EVT_MC_TIME "/Event/Mchn_time"
#define TOPIC_EVT_ANDON "/Event/Andon"
#define TOPIC_EVT_TEACHING "/Event/Teaching"
#define TOPIC_EVT_BLANK_NFC "/Event/Nfc/Uid"

#define TOPIC_DB_RESET_OUTPUT "/Event/ResetOutput"
/////////////////////////////////////////////////////////////////////////////////////
std::list<uint32_t> list_pubId;
std::list<String> list_pubTopic;
std::list<String> list_pubJson;
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
uint8_t ucCntCheckPubAck;
uint8_t ucCntSendPayload;
#define MAX_SEND_PAYLOAD 3
/////////////////////////////////////////////////////////////////////////////////////
#define INTERVAL_TRY_CONNECT_MQTT 30000 // ms
#define INTERVAL_COMM_SVR 2000          // ms delay time send data to server

String msg_setWill;

#define E2PE_LEN_SVR_ADDR 32
char pcSvrAddr[E2PE_LEN_SVR_ADDR];

extern uint8_t ucHourCur;
extern uint8_t ucMinCur;
extern uint8_t ucSecCur;

uint32_t ulTimeOutputOld;
/////////////////////////////////////////////////////////////////////////////////////
AsyncMqttClient mqttClient;

Fun_mqtt mqtt;
/////////////////////////////////////////////////////////////////////////////////////
void ping2Host();
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
void mqtt_disconnect()
{
    mqttClient.disconnect();
}
/////////////////////////////////////////////////////////////////////////////////////
static void onMqttConnect(bool sessionPresent)
{
#ifdef ESP_DEBUG_MQTT
    Serial.printf("mqttConnected\r\n");
#endif

    mqtt.bConnected = true;

    if (mqtt.bDisconnected)
    {
        mqtt.bDisconnected = false;

        mqtt.mqttStep = mqtt.SEND_SYSCTRL;
    }

    if (!mqtt.bMqttSubscribe)
    {
        mqtt.bMqttSubscribe = true;

        mqtt.mqttSubscribe();
    }

    wifi.ucErr = WIFI_NO_ERR;
}
/////////////////////////////////////////////////////////////////////////////////////
static void onMqttDisconnect(AsyncMqttClientDisconnectReason reason)
{
#ifdef ESP_DEBUG_MQTT
    Serial.printf("onMqttDisconnect\r\n");
#endif

    mqtt.bConnected = false;
    mqtt.bDisconnected = true;

    if (WiFi.status() == WL_CONNECTED)
        wifi.ucErr = WIFI_ERR_MQTT;
    else
        wifi.ucErr = WIFI_ERR_AP;

    mqtt.b_send_mqtt_disconnected = true;
    mqtt.u8_mqtt_dis_reason = (uint8_t)reason;
}
/////////////////////////////////////////////////////////////////////////////////////
static void onMqttMessage(char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total)
{
    mqtt.mqttMessage(topic, payload, properties, len, index, total);
}
/////////////////////////////////////////////////////////////////////////////////////
static void onMqttPublish(uint16_t packetId)
{
#ifdef ESP_DEBUG_MQTT
    Serial.printf("Publish acknowledged, packetId: %d\r\n", packetId);
#endif

    if (!list_pubId.empty())
    {
        uint16_t packet = list_pubId.front();
        if (packetId == packet)
        {
            list_pubId.pop_front();
            list_pubTopic.pop_front();
            list_pubJson.pop_front();

#ifdef ESP_DEBUG_MQTT
            Serial.printf("list_pubId pop_font, id: %d\r\n", packetId);
#endif
        }
    }
}
/////////////////////////////////////////////////////////////////////////////////////
Fun_mqtt::Fun_mqtt()
{
}
/////////////////////////////////////////////////////////////////////////////////////
Fun_mqtt::~Fun_mqtt()
{
}
/////////////////////////////////////////////////////////////////////////////////////
void Fun_mqtt::init()
{
    sSvrAddr.toCharArray(pcSvrAddr, E2PE_LEN_SVR_ADDR + 1);
    mqttClient.setServer(pcSvrAddr, 1883);
    mqttClient.onConnect(onMqttConnect);
    mqttClient.onDisconnect(onMqttDisconnect);
    mqttClient.onMessage(onMqttMessage);
    mqttClient.onPublish(onMqttPublish);
    mqttClient.setKeepAlive(120);

    // set will
    msg_setWill = msgSendInfo("OFF"); // must be save at heap memory
    mqttClient.setWill("/SysCtrl", 2, false, msg_setWill.c_str());

    // first, send /sysctrl for get date time
    mqttStep = SEND_SYSCTRL;
}
/////////////////////////////////////////////////////////////////////////////////////
void Fun_mqtt::loop()
{
    mqttCommunicateSvr();
}
/////////////////////////////////////////////////////////////////////////////////////
void Fun_mqtt::connectToMqtt()
{
#ifdef ESP_DEBUG_MQTT
    Serial.printf("connectToMqtt\r\n");
#endif
    mqttClient.connect();
}
/////////////////////////////////////////////////////////////////////////////////////
void Fun_mqtt::mqttSubscribe()
{
    ///////////////////////////
    // subscribe
    // get date time
    sSubTopicConfig = "/SysCtrl/" + wifi.sMac;
    mqttClient.subscribe(sSubTopicConfig.c_str(), 0);
    // get plan
    sSubTopicPlan = "/Plan/" + wifi.sMac;
    mqttClient.subscribe(sSubTopicPlan.c_str(), 0);
    // get config
    sSubTopGetConfig = "/SysCtrl/Config/" + wifi.sMac;
    mqttClient.subscribe(sSubTopGetConfig.c_str(), 0);
    // ota manual
    sSubTopicOta = "/SysCtrl/OTA/" + Iot.sHwVer;
    mqttClient.subscribe(sSubTopicOta.c_str(), 0);
    // ota iot auto check
    sSubTopicOtaAuto = "/OTA/" + wifi.sMac;
    mqttClient.subscribe(sSubTopicOtaAuto.c_str(), 0);
    // event update output-defect
    sEventUpdate = "/Event/" + wifi.sMac;
    mqttClient.subscribe(sEventUpdate.c_str(), 0);
    // set teaching
    sSubTopSetTeaching = "/Teaching/" + wifi.sMac;
    mqttClient.subscribe(sSubTopSetTeaching.c_str(), 0);

    ////////////////////////////////////////////////////
    // for debug
    sDebugInfo = "/Debug/" + wifi.sMac;
    mqttClient.subscribe(sDebugInfo.c_str(), 0);

    debugOTA = "/Debug/OTA/" + wifi.sMac;
    mqttClient.subscribe(debugOTA.c_str(), 0);

    ////////////////////////////////////////////////////
    debugPing = "/Ping/" + wifi.sMac;
    mqttClient.subscribe(debugPing.c_str(), 0);
}
/////////////////////////////////////////////////////////////////////////////////////
void Fun_mqtt::mqttMessage(char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total)
{
    usLenMsg += len;
    sMsg += String(payload).substring(0, len);

#ifdef ESP_DEBUG_MQTT
    Serial.printf("mqttMessage\r\n");
    Serial.printf("%s: %s\r\n", topic, sMsg.c_str());
#endif

    if (usLenMsg >= total)
    {
        if (String(topic) == sSubTopicConfig)
            getDateTime(sMsg);
        else if (String(topic) == sSubTopicPlan)
            getPlan(sMsg);
        else if (String(topic) == sSubTopGetConfig)
            getInfConfig(sMsg);
        else if (String(topic) == sSubTopicOtaAuto || String(topic) == sSubTopicOta)
            getOta(sMsg);
        else if (String(topic) == sEventUpdate)
            updateCount(sMsg);
        else if (String(topic) == sSubTopSetTeaching)
            setTeaching(sMsg);
        //////////////////////////////////////////////
        // debug
        else if (String(topic) == sDebugInfo)
            debugInfo(sMsg);
        else if (String(topic) == debugOTA)
            getDebugOTA(sMsg);
        //////////////////////////////////////////////
        else if (String(topic) == debugPing)
            checkPing(sMsg);

        usLenMsg = 0;
        sMsg = "";
    }
}
/////////////////////////////////////////////////////////////////////////////////////
void Fun_mqtt::mqttCommunicateSvr()
{
    if (!mqttClient.connected() || mqtt.bDisconnected)
    {
        if (millis() - ulMilTryConnect > INTERVAL_TRY_CONNECT_MQTT)
        {
            ulMilTryConnect = millis();
            connectToMqtt();
        }
        return;
    }

    if ((millis() - statusChange < INTERVAL_COMM_SVR))
        return;

    if (oper.bGetDatetime)
    {
        if (oper.bSendMcTime) // send mc data time
        {
            oper.bSendMcTime = false;
            send_mc_time();
        }
        else if (tea.bSendIdxTeaSeq2Svr && oper.bGetPlan) // send teaching sequence - current index plan
        {
            tea.bSendIdxTeaSeq2Svr = false;
            send_idxTeachingSequence(oper.current_idx_plan);
        }
        else if (tea.bSendAllTeaSeq2Svr && oper.bGetPlan) // send all teaching sequence - length plan
        {
            tea.bSendAllTeaSeq2Svr = false;
            send_allTeachingSequence();
        }
        else if (nfc.bSendUidNfcBlank2Svr)
        {
            nfc.bSendUidNfcBlank2Svr = false;
            e2p_info.write(E2PE_ADR_SEN_BLA_NFC, 0);
            e2p_info.commit_flag = true;

            sen_uidNfcBlank();
        }
        else if (nfc.bSendUidWithWifiConnected)
        {
            nfc.bSendUidWithWifiConnected = 0;
            sen_uidNfcBlank();
        }
        else if (oper.bSendAndon)
        {
            oper.bSendAndon = false;
            send_andonMode(); // read andon mode and send to server;
        }
        else if (bPing2Host)
        {
            bPing2Host = false;
            ping2Host();
        }
        else if (wifi.bSendScanWifi)
        {
            wifi.bSendScanWifi = false;

            String topic = String("/Wifi_Scan/") + wifi.sMac;
            mqttClient.publish(topic.c_str(), 2, false, wifi.sScanWifiInfo.c_str());
        }
        else if (b_send_mqtt_disconnected)
        {
            b_send_mqtt_disconnected = false;

            String topic = String("/MQTT/Disconnected/") + wifi.sMac;
            String payload = String("Reason: ") + String(u8_mqtt_dis_reason);
            mqttClient.publish(topic.c_str(), 2, false, payload.c_str());
        }
        // end debug
        else if (inOut_mode.qFont != inOut_mode.qRear)
        {
            send_uid_nfc_inout_iot_mode();
        }
        // else if (debug.send_debug)
        // {
        //     mqtt_publish(debug.topic_debug, 2, false, debug.msg_debug);

        //     debug.msg_debug = "";
        //     debug.send_debug = false;
        // }
    } // end if (oper.bGetDatetime)

    if (debug.send_debug)
    {
        mqtt_publish(debug.topic_debug, 2, false, debug.msg_debug);

        debug.msg_debug = "";
        debug.send_debug = false;
    }

    switch (mqttStep)
    {
    case SEND_DATA:
        FUNC_SEND_DATA();
        break;
    case SEND_SYSCTRL:
        FUNC_SEND_INFO();
        break;
    default:
        mqttStep = SEND_SYSCTRL;
        break;
    }

    statusChange = millis();
}
/////////////////////////////////////////////////////////////////////////////////////
void Fun_mqtt::FUNC_SEND_INFO()
{
    // publish - send status on and get datetime & config
    String msg = msgSendInfo("ON");
    mqttClient.publish("/SysCtrl", 2, false, msg.c_str());

    mqttStep = SEND_DATA;
}
/////////////////////////////////////////////////////////////////////////////////////
void Fun_mqtt::FUNC_SEND_DATA()
{
    // if (oper.len_plan)
    //     SEND_DATA_OUTPUT();

    if (!oper.len_plan)
        return;

    if (list_pubId.empty())
    {

        SEND_DATA_OUTPUT();
    }
    else
    {
        if (++ucCntCheckPubAck >= 10) // 10x2000ms = 20s
        {
            ucCntCheckPubAck = 0;

            String topic;
            String payload;

            topic = list_pubTopic.front();
            payload = list_pubJson.front();

            list_pubId.pop_front();
            list_pubTopic.pop_front();
            list_pubJson.pop_front();

            if (ucCntSendPayload < MAX_SEND_PAYLOAD)
            {
                ucCntSendPayload++;
                mqtt_publish_data(topic, 2, false, payload);

#ifdef ESP_DEBUG_MQTT
                Serial.printf("re-send: %s\r\n", payload.c_str());
#endif
            }
            else
            {
#ifdef ESP_DEBUG_MQTT
                Serial.printf("no receive ACK\r\n");
#endif
                ucCntSendPayload = 0;
                mqtt_publish(String("/Event/Cnt/NoRecACK"), 2, false, payload);
            }
        }
    }
}
/////////////////////////////////////////////////////////////////////////////////////
void Fun_mqtt::SEND_DATA_OUTPUT()
{
    if (e2p_data.read_16(E2PE_ADR_DATA_Q_FRONT) == e2p_data.read_16(E2PE_ADR_DATA_Q_REAR))
        return;

    uint8_t ucBuffData[E2PE_LEN_Q_DATA];
    oper.delQueue_product_data(ucBuffData); // get buffer
    oper.dev_IncQrearIdx();                 // increase Q_Rear

    String msg, topic;
    msg = createJsonCount(ucBuffData);
    if (msg.length() > 10)
    {
        topic = "/Event/Cnt";
        mqtt_publish_data(topic, 2, false, msg);
    }
}
/////////////////////////////////////////////////////////////////////////////////////
String Fun_mqtt::msgSendInfo(String sStatus)
{
    String data_json = "{";

    data_json.concat("\"IOT_MAC\":\"");
    data_json.concat(wifi.sMac);
    data_json.concat("\",\"AP_MAC\":\"");
    data_json.concat(wifi.sMacAP);
    data_json.concat("\",\"IOT_SPPL_CD\":\"");
    data_json.concat(Iot.sIotSupplier);
    data_json.concat("\",\"IOT_MODEL\":\"");
    data_json.concat(Iot.sIotModel);
    data_json.concat("\",\"MCIP\":\"");
    data_json.concat(wifi.sIP);
    data_json.concat("\",\"MCID\":\"");
    data_json.concat("-");
    data_json.concat("\",\"STS\":\"");
    data_json.concat(sStatus);
    data_json.concat("\",\"VER\":\"");
    data_json.concat(Iot.sVer);

    data_json.concat("\"}");

    return data_json;
}
/////////////////////////////////////////////////////////////////////////////////////
bool Fun_mqtt::check_reset_newday()
{
    ti.getSysTime();

    int old_day = e2p_info.read(E2PE_ADR_DAY_RESET);
    int old_mon = e2p_info.read(E2PE_ADR_MON_RESET);
    int old_year = ti.sysTime.tm_year;

    if (old_mon > 12)
        old_mon = ti.sysTime.tm_mon;
    if (old_mon > ti.sysTime.tm_mon)
        old_year--;

    tm a = {0, 0, 0, old_day, old_mon - 1, old_year - 1900};
    tm b = {0, 0, 0, ti.sysTime.tm_mday, ti.sysTime.tm_mon - 1, ti.sysTime.tm_year - 1900};
    int days_between = (int)ti.get_numberof_day(a, b);

    bool reset = false;

    if (days_between > 0)
    {
        if (days_between > 1)
            reset = true;
        else if (ti.sysTime.tm_hour >= TIME_HOUR_RST_NEW_DAY)
            reset = true;
    }

    if (bChkRstNewDay)
    {
        bChkRstNewDay = false;

        char buffer[100];
        sprintf(buffer, "%02d:%02d:%04d - %02d:%02d:%04d: %d at %02d:%02d:%02d, ret = %d",
                ti.sysTime.tm_mday, ti.sysTime.tm_mon, ti.sysTime.tm_year,
                old_day, old_mon, old_year,
                days_between,
                ti.sysTime.tm_hour, ti.sysTime.tm_min, ti.sysTime.tm_sec,
                reset);

        debug.msg_debug.concat(" --/ check_rst_new_day: ");
        debug.msg_debug.concat(buffer);
        debug.send_debug = true;
    }

    return reset;
}
/////////////////////////////////////////////////////////////////////////////////////
uint32_t u32_mil_rec_date_time;
void Fun_mqtt::getDateTime(String data)
{
    // if (oper.bGetDatetime)
    //     return;

    StaticJsonDocument<192 + 20> root;
    if (deserializeJson(root, data))
        return;

    if (millis() - u32_mil_rec_date_time < 10 * 1000 && u32_mil_rec_date_time) // 1minute
        return;

    // save for update time in EEPROM
    ti.getSysTime();
    ucHourCur = ti.sysTime.tm_hour;
    ucMinCur = ti.sysTime.tm_min;
    ucSecCur = ti.sysTime.tm_sec;

    debug.send_debug = true;

    ti.setSysTime(root["YEAR"], root["MON"], root["DAY"],
                  root["HOUR"], root["MIN"], root["SEC"]);

    ti.getSysTime();

    ti.u8_hourCur = ti.sysTime.tm_hour;

    updateTimeEEPROM();

    bChkRstNewDay = true;

    if (check_reset_newday())
        resetDataWhenNewDay(false);

    oper.check_data_machine = true;
    oper.bGetDatetime = true;

    lcd_refresh();

    u32_mil_rec_date_time = millis();

    debug.msg_debug.concat(" --/ getDateTime: ");
    debug.msg_debug.concat(ti.sysTime.tm_hour);
    debug.msg_debug.concat(":");
    debug.msg_debug.concat(ti.sysTime.tm_min);
    debug.msg_debug.concat(":");
    debug.msg_debug.concat(ti.sysTime.tm_sec);
    debug.msg_debug.concat(":");
    debug.send_debug = true;
}
/////////////////////////////////////////////////////////////////////////////////////
void Fun_mqtt::getPlan(String data)
{
    if (data.length() < 10)
        return;

    StaticJsonDocument<1000* 2> root;
    if (deserializeJson(root, data) || !root.size())
        return;

    oper.len_plan = root.size();
    if (oper.len_plan > LEN_JSON_OBJECT_PLAN)
        oper.len_plan = LEN_JSON_OBJECT_PLAN;

    String line = root[0]["LINE"];
    oper.line = line.substring(0, E2PE_LEN_LINE);
    e2p_info.StringToE2P(E2PE_ADR_LINE, E2PE_LEN_LINE, oper.line);

    String factory = root[0]["FACTORY"];
    oper.factory = factory.substring(0, E2PE_LEN_FACTORY);
    e2p_info.StringToE2P(E2PE_ADR_FACTORY, E2PE_LEN_FACTORY, oper.factory);

    String position = root[0]["POSITION"];
    oper.position = position.toInt();
    e2p_info.write_16(E2PE_ADR_POSITION, oper.position);

    for (uint8_t i = 0; i < oper.len_plan; i++)
    {
        if (i == LEN_JSON_OBJECT_PLAN)
            break;

        String TARGET = root[i]["TARGET"];
        uint16_t target = TARGET.toInt();
        e2p_data.write_32(oper.list_addr_output[i] + E2PE_LEN_OUTPUT + E2PE_LEN_DEFECT, target);

        String sWrkId = root[i]["WORKERID"];
        String sStyle = root[i]["STYLE"];
        String sProcess = root[i]["PROCESS"];
        String sProcess_id = root[i]["PROCESSID"];

        oper.plan_wrk_id[i] = sWrkId.substring(0, E2PE_LEN_WRK_ID);
        oper.plan_stlye[i] = sStyle.substring(0, E2PE_LEN_STYLE);
        oper.plan_process[i] = sProcess.substring(0, E2PE_LEN_PROC);
        oper.plan_process_id[i] = sProcess_id.substring(0, E2PE_LEN_PROC_ID);
    }

    // ucFlaDonSenTeaSeq = 1;     // check teaching sequence
    if (oper.current_idx_plan >= oper.len_plan)
    {
        oper.current_idx_plan = 0;
        e2p_info.write(E2PE_ADR_INDX_PLAN, oper.current_idx_plan);

        update_runTeaSeqWithCurrentIdxPlan();
        teqseq_restartRunMode();
        // e2p_info.commit_flag = true;
    }

    oper.update_product_current_process_id(); // update output, defect, target ...
    oper.update_config_current_process_id();  // update scale, handling time
    oper.bGetPlan = true;                     // for don't request plan again
    tea.bSendAllTeaSeq2Svr = true;            // send all teaching sequence to server

    lcd_refresh(); // update main lcd

    // // check fake || real plan by: target? real:fake
    // if (oper.target)
    // {
    //     oper.bRecRealPlan = true;
    //     if (oper.bAndonMode)
    //         Led.ucColor = uint8_t(LED_1_RED + (LED_1_RED << 3)); // Red
    //     else
    //         Led.ucColor = LED_OFF;
    // }
    // else
    // {
    //     oper.bRecRealPlan = false;
    //     Led.ucColor = (uint8_t)(LED_1_GREEN << 3);
    // }
    // Led.ucActive = 1;
    // Led.ulTimeTurnOn = 0;

    debug.msg_debug.concat(" --/ getPlan: ");
    debug.send_debug = true;
}
/////////////////////////////////////////////////////////////////////////////////////
void Fun_mqtt::getInfConfig(String data)
{
    StaticJsonDocument<700* 2> root;
    if (deserializeJson(root, data) || !root.size())
        return;

    uint8_t ucDayOutRst = root[0]["DAY_OUT_RST"];
    oper.daily_product_reset = ucDayOutRst;
    e2p_info.write(E2PE_ADR_DAI_OUT_RST, oper.daily_product_reset);

    for (size_t i = 0; i < root.size(); i++)
    {
        // handling time
        uint32_t handling_time = root[i]["HANDLING"];
        oper.plan_handling_time[i] = handling_time;

        // output scale
        uint16_t usOutScale = root[i]["SCALE"];
        if (usOutScale > 0)
            oper.plan_scale[i] = usOutScale;
    }

    oper.update_config_current_process_id();
}
/////////////////////////////////////////////////////////////////////////////////////
void Fun_mqtt::getOta(String data)
{
    StaticJsonDocument<300> root;
    if (deserializeJson(root, data))
        return;

    if (root["VER"] == Iot.sSwVer)
    {
        return;
    }

    String sUrl = root["FILE"];

    if (sUrl.startsWith("http://"))
    {
        wifi.bOtaByHost = true;
        wifi.sUrlByHost = sUrl;
    }
    else
    {
        wifi.sUrlOTA = String("http://") + sSvrAddr + ":" + String(usSvrPort) + "/" + sUrl;
        wifi.bUpdateOTA = true;
    }

#ifdef ESP_DEBUG_WIFI
    Serial.printf("getOta, data: %s\r\n", data.c_str());
    Serial.printf("wifi.bUpdateOTA: %d\r\n", wifi.bUpdateOTA);
    Serial.printf("wifi.sUrlOTA: %s\r\n", wifi.sUrlOTA.c_str());
#endif
}
/////////////////////////////////////////////////////////////////////////////////////
String createMesUpdateCnt(uint32_t usValue, uint8_t ucType, String sProcessId)
{
    String stType;
    if (ucType == Q_TYPE_OUTPUT)
        stType = String("OUTPUT");
    else
        stType = String("DEFECT");

    String data_json = "{";

    data_json.concat("\"YEAR\":");
    data_json.concat(ti.sysTime.tm_year);
    data_json.concat(",\"MON\":");
    data_json.concat(ti.sysTime.tm_mon);
    data_json.concat(",\"DAY\":");
    data_json.concat(ti.sysTime.tm_mday);
    data_json.concat(",\"HOUR\":");
    data_json.concat(ti.sysTime.tm_hour);
    data_json.concat(",\"MIN\":");
    data_json.concat(ti.sysTime.tm_min);
    data_json.concat(",\"SEC\":");
    data_json.concat(ti.sysTime.tm_sec);
    data_json.concat(",\"PROCESSID\":\"");
    data_json.concat(sProcessId);
    data_json.concat("\",\"TYPE\":\"");
    data_json.concat(stType);
    data_json.concat("\",\"DATA\":");
    data_json.concat(usValue);
    data_json.concat(",\"MAC\":\"");
    data_json.concat(wifi.sMac);
    data_json.concat(",\"UID\":\"");
    data_json.concat(nfc.sUidBlank);

    data_json.concat("\"}");

    return data_json;
}
/////////////////////////////////////////////////////////////////////////////////////
void Fun_mqtt::updateCount(String data)
{
    if (data.length() < 10)
        return;

    StaticJsonDocument<192 * 2> root;
    if (deserializeJson(root, data))
        return;
    if (!root.size())
        return;

    for (uint8_t i = 0; i < root.size(); i++)
    {
        if (i == LEN_JSON_OBJECT_PLAN)
            break;

        String sData = root[i]["DATA"];
        uint16_t usData = sData.toInt();
        String sProcessId = root[i]["PROCESSID"];
        String sType = root[i]["TYPE"];

        if (sProcessId == oper.process_id)
        { // save to eeprom
            if (sType == String("OUTPUT"))
            {
                oper.output = usData;
                oper.addQueue_product_data(Q_TYPE_OUTPUT, oper.output);
            }
            else if (sType == String("DEFECT"))
            {
                oper.defect = usData;
                oper.addQueue_product_data(Q_TYPE_DEFECT, oper.defect);
            }
        }
        else
        { // send to server
            String msg, topic;

            for (uint8_t i = 0; i < oper.len_plan; i++)
            {
                if (sProcessId == oper.plan_process_id[i])
                {
                    e2p_data.write_32(oper.list_addr_output[i], usData);
                    if (sType == String("OUTPUT"))
                        msg = createMesUpdateCnt(usData, Q_TYPE_OUTPUT, sProcessId);
                    else if (sType == String("DEFECT"))
                        msg = createMesUpdateCnt(usData, Q_TYPE_DEFECT, sProcessId);

                    topic = TOPIC_EVT_PRODUCT;
                    mqtt_publish(topic, 2, false, msg);
                }
            }
        }
    }
}
/////////////////////////////////////////////////////////////////////////////////////
uint8_t mqttGetIdTeaching(String data)
{
    if (data == "STI")
        return TEA_SEQ_SEN0_ID;
    else if (data == "TRI")
        return TEA_SEQ_SOL0_ID;
    else if (data == "DLY")
        return TEA_SEQ_DLY_ID;

    return 0x00;
}
/////////////////////////////////////////////////////////////////////////////////////
void Fun_mqtt::setTeaching(String data)
{
    StaticJsonDocument<512 * 2> root;
    if (deserializeJson(root, data))
        return;
    if (!root.size() || root.size() > LEN_JSON_OBJECT_PLAN)
        return;

    uint8_t ucCountPlan = oper.len_plan;

    if (root.size() < ucCountPlan)
        ucCountPlan = root.size();

    for (size_t i = 0; i < root.size(); i++)
    {
        String sProces_Id = root[i]["PROCESS_ID"];

        JsonArray jsArray = root[i]["DATA"];
        uint8_t ucIndxTea = 0;
        for (size_t j = 0; j < jsArray.size(); j++)
        {
            uint16_t usValue = jsArray[j]["SEQ_VALUE"];
            String sKey = jsArray[j]["SEQ_NAME"];
            uint8_t ucId = mqttGetIdTeaching(sKey);
            if (ucId != 0x00)
            {
                tea.ucTeaSeqStore[i][ucIndxTea++] = ucId;
                tea.ucTeaSeqStore[i][ucIndxTea++] = (uint8_t)(usValue >> 8);
                tea.ucTeaSeqStore[i][ucIndxTea++] = (uint8_t)(usValue);
                if (ucIndxTea >= 30)
                    break;
            }
        }
        tea.ucTeaSeqStore[i][ucIndxTea] = TEA_SEQ_END_ID; // end teaching
    }

    update_runTeaSeqWithCurrentIdxPlan();
    save_allTeachingSequence();
    teqseq_restartRunMode();
}
/////////////////////////////////////////////////////////////////////////////////////
void Fun_mqtt::debugInfo(String data)
{
    calDapot();

    uint32_t fActiveTime = (oper.handling_time + oper.motor_runtime) / 1000;
    String sLocation = oper.factory + "-" + oper.line + "-" + String(oper.position);

    String sMsg;
    sMsg += "\n\tVersion:\t" + Iot.sVer;
    sMsg += "\n\tMac addr:\t" + wifi.sMac;
    sMsg += "\n\tIP:\t\t" + wifi.sIP;
    sMsg += "\n\tLocation:\t" + sLocation;

    sMsg += "\n";
    sMsg += "\n\tMemory:\t\t" + String(ESP.getFreeHeap()) + "\t\t (bytes)";

    sMsg += "\n";
    sMsg += "\n\tOutput:\t\t" + String(oper.output);
    sMsg += "\n\tDefect:\t\t" + String(oper.defect);
    sMsg += "\n\tTarget:\t\t" + String(oper.target);

    sMsg += "\n";
    sMsg += "\n\tPower ontime:\t" + String(oper.power_ontime) + "\t\t (s)";
    sMsg += "\n\tActive time:\t" + String(fActiveTime) + "\t\t (s)";
    sMsg += "\n\tMot runtime:\t" + String(oper.motor_runtime / 1000) + "\t\t (s)";
    sMsg += "\n\tHandling time:\t" + String(oper.handling_time / 1000) + "\t\t (s)";
    sMsg += "\n\tSum cyc time:\t" + String(oper.cycle_time) + "\t\t (s)";
    sMsg += "\n\tCur cyc time:\t" + String(oper.current_cycle_time) + "\t\t (s)";
    sMsg += "\n\tSUM_STI:\t" + String(oper.sumof_stitch);
    sMsg += "\n\tSUM_TRI:\t" + String(oper.sumof_trim);
    sMsg += "\n\tHandle idx:\t" + String(oper.handling_time_idx_plan);

    sMsg += "\n";
    sMsg += "\n\tTeaching:";
    for (int i = 0; i < (E2PE_LEN_TEA_SEQ - 1) / 3; i++)
    {
        if (tea.ucTeaSeqStore[oper.current_idx_plan][i * 3] == TEA_SEQ_END_ID)
            break;
        String sName = getSeqName(tea.ucTeaSeqStore[oper.current_idx_plan][i * 3]);
        if (sName == "XXX")
            continue;
        uint16_t usValue;
        usValue = (uint16_t)(tea.ucTeaSeqStore[oper.current_idx_plan][i * 3 + 1] << 8);
        usValue += (uint16_t)(tea.ucTeaSeqStore[oper.current_idx_plan][i * 3 + 2]);

        sMsg += "\n\t\t" + sName + ":\t" + String(usValue);
    }

    sMsg += "\n";
    sMsg += "\n\tRssi:\t\t" + String(WiFi.RSSI());
    sMsg += "\n\tSsid:\t\t" + wifi.sSsid;
    sMsg += "\n\tPsk:\t\t" + wifi.sPassword;
    sMsg += "\n\tSvr:\t\t" + mqtt.sSvrAddr + "(" + String(mqtt.usSvrPort) + ")";

    sMsg += "\n";
    sMsg += "\n\tScalet:\t\t" + String(oper.scale);
    sMsg += "\n\tdelay sen0:\t" + String(tea.delay_sen0);
    sMsg += "\n\tgeneral mc:\t" + String(oper.ucNonSewingMc);
    sMsg += "\n\tdaily out_rst:\t" + String(oper.daily_product_reset);

    String sTopic = "/Debug/" + wifi.sMac + "/" + data;
    mqtt_publish(sTopic, 2, false, sMsg);
}
/////////////////////////////////////////////////////////////////////////////////////
void Fun_mqtt::getDebugOTA(String data)
{
    if (data.startsWith("http://"))
    {
        wifi.bOtaByHost = true;
        wifi.sUrlByHost = data;
    }
}
/////////////////////////////////////////////////////////////////////////////////////
void Fun_mqtt::checkPing(String data)
{
    bPing2Host = true;
    hostPing = data;
}
/////////////////////////////////////////////////////////////////////////////////////
void ping2Host()
{
    uint8_t ip_t[4];
    sscanf(hostPing.c_str(), "%hhu.%hhu.%hhu.%hhu", &ip_t[0], &ip_t[1], &ip_t[2], &ip_t[3]);
    IPAddress ip = IPAddress(ip_t);

    String json = wifi.sMac + ", " + wifi.sIP + " ping to " + hostPing + " ";
    if (Ping.ping(ip))
        json.concat("success");
    else
        json.concat("failled");

    mqtt.mqtt_publish("/Ping", 2, false, json);
}
/////////////////////////////////////////////////////////////////////////////////////
bool bGetCycleTime = false;
String Fun_mqtt::createJsonCount(uint8_t *ucQ_Data)
{
    String data_json = "{";
    if (ucQ_Data[Q_BUF_IDX_YEAR] + 2000 > 2020) // check save inside e2p datetime error
    {
        uint32_t usCountVal;
        String sPrcId, sType;

        // calculate current cycle time
        uint32_t ulTimeCurOutput = ucQ_Data[Q_BUF_IDX_HOUR] * 3600 + ucQ_Data[Q_BUF_IDX_MIN] * 60 + ucQ_Data[Q_BUF_IDX_SEC];
        if (!bGetCycleTime || (ulTimeCurOutput < ulTimeOutputOld))
        {
            oper.current_cycle_time = 0;
            bGetCycleTime = true;
        }
        else
        {
            oper.current_cycle_time = ulTimeCurOutput - ulTimeOutputOld;
            oper.cycle_time += oper.current_cycle_time;
        }
        ulTimeOutputOld = ulTimeCurOutput;

        usCountVal = (uint32_t)ucQ_Data[Q_BUF_IDX_DATA] << 24;
        usCountVal += (uint32_t)ucQ_Data[Q_BUF_IDX_DATA + 1] << 16;
        usCountVal += (uint32_t)ucQ_Data[Q_BUF_IDX_DATA + 2] << 8;
        usCountVal += (uint32_t)ucQ_Data[Q_BUF_IDX_DATA + 3];

        if (ucQ_Data[Q_BUF_IDX_CUR_PLAN] >= oper.len_plan)
            sPrcId = oper.plan_process_id[0];
        else
            sPrcId = oper.plan_process_id[ucQ_Data[Q_BUF_IDX_CUR_PLAN]];

        if (ucQ_Data[Q_BUF_IDX_TYPE] == Q_TYPE_OUTPUT)
            sType = String("OUTPUT");
        else if (ucQ_Data[Q_BUF_IDX_TYPE] == Q_TYPE_DEFECT)
            sType = String("DEFECT");
        else
            sType = String("UNKNOW");

        // String data_json = "{";

        // if (ucQ_Data[Q_BUF_IDX_YEAR] + 2000 > 2020) // check save inside e2p datetime error
        // {
        data_json.concat("\"YEAR\":");
        data_json.concat(ucQ_Data[Q_BUF_IDX_YEAR] + 2000);
        data_json.concat(",\"MON\":");
        data_json.concat(ucQ_Data[Q_BUF_IDX_MON]);
        data_json.concat(",\"DAY\":");
        data_json.concat(ucQ_Data[Q_BUF_IDX_DAY]);
        data_json.concat(",\"HOUR\":");
        data_json.concat(ucQ_Data[Q_BUF_IDX_HOUR]);
        data_json.concat(",\"MIN\":");
        data_json.concat(ucQ_Data[Q_BUF_IDX_MIN]);
        data_json.concat(",\"SEC\":");
        data_json.concat(ucQ_Data[Q_BUF_IDX_SEC]);
        data_json.concat(",\"PROCESSID\":\"");
        data_json.concat(sPrcId);
        data_json.concat("\",\"MAC\":\"");
        data_json.concat(wifi.sMac);
        data_json.concat("\",\"TYPE\":\"");
        data_json.concat(sType);
        data_json.concat("\",\"DATA\":");
        data_json.concat(usCountVal);
        data_json.concat(",\"CUR_CYCLE\":");
        data_json.concat(oper.current_cycle_time);
        data_json.concat(",\"SUM_CYCLE\":");
        data_json.concat(oper.cycle_time);
        data_json.concat(",\"UID\":\"");
        data_json.concat(nfc.sUidBlank);
        data_json.concat("\"");
    }
    else
    {
    }

    data_json.concat("}");

    return data_json;
}
/////////////////////////////////////////////////////////////////////////////////////
void Fun_mqtt::mqtt_publish(String topic, uint8_t qos, bool retain, String payload)
{
    mqttClient.publish(topic.c_str(), qos, retain, payload.c_str());
}
/////////////////////////////////////////////////////////////////////////////////////
void Fun_mqtt::mqtt_publish_data(String topic, uint8_t qos, bool retain, String payload)
{
    uint16_t packetId = mqttClient.publish(topic.c_str(), qos, retain, payload.c_str());

#ifdef ESP_DEBUG_MQTT
    Serial.printf("PacketId push, id: %d\r\n", packetId);
#endif

    list_pubId.push_front(packetId);
    list_pubTopic.push_front(topic);
    list_pubJson.push_front(payload);
}
/////////////////////////////////////////////////////////////////////////////////////
void Fun_mqtt::send_mc_time()
{
    calDapot();
    ti.getSysTime();

    String sTopic, sMsg;

    sMsg = "{";
    sMsg.concat("\"YEAR\":");
    sMsg.concat(ti.sysTime.tm_year);
    sMsg.concat(",\"MON\":");
    sMsg.concat(ti.sysTime.tm_mon);
    sMsg.concat(",\"DAY\":");
    sMsg.concat(ti.sysTime.tm_mday);
    sMsg.concat(",\"HOUR\":");
    sMsg.concat(ti.sysTime.tm_hour);
    sMsg.concat(",\"MIN\":");
    sMsg.concat(ti.sysTime.tm_min);
    sMsg.concat(",\"SEC\":");
    sMsg.concat(ti.sysTime.tm_sec);
    sMsg.concat(",\"MAC\":\"");
    sMsg.concat(wifi.sMac);
    sMsg.concat("\",\"POW_TIME\":");
    sMsg.concat(oper.power_ontime);
    sMsg.concat(",\"MOTO_TIME\":");
    sMsg.concat(oper.motor_runtime / 1000);
    sMsg.concat(",\"ACT_TIME\":");
    sMsg.concat((oper.handling_time + oper.motor_runtime) / 1000);
    sMsg.concat(",\"SUM_STI\":");
    sMsg.concat(oper.sumof_stitch);
    sMsg.concat(",\"SUM_TRI\":");
    sMsg.concat(oper.sumof_trim);
    sMsg.concat("}");

    mqtt_publish(TOPIC_EVT_MC_TIME, 2, false, sMsg);
}
/////////////////////////////////////////////////////////////////////////////////////
bool Fun_mqtt::check_get_data_from_server()
{
    if (!oper.bGetDatetime || !oper.bGetPlan)
    {
        mqttStep = SEND_SYSCTRL;
        return true;
    }

    return false;
}
/////////////////////////////////////////////////////////////////////////////////////
String Fun_mqtt::getSeqName(uint8_t ucId)
{
    String sId = "XXX"; // default

    if (ucId == TEA_SEQ_SEN0_ID)
        sId = "STI";
    else if (ucId == TEA_SEQ_SEN1_ID)
        sId = "STI_1";
    else if (ucId == TEA_SEQ_SOL0_ID)
        sId = "TRI";
    else if (ucId == TEA_SEQ_DLY_ID)
        sId = "DLY";

    return sId;
}
/////////////////////////////////////////////////////////////////////////////////////
void Fun_mqtt::send_idxTeachingSequence(uint8_t ucIndx)
{
    String sTopic, sMsg;

    sMsg = "{";
    sMsg.concat("\"MAC\":\"");
    sMsg.concat(wifi.sMac);
    sMsg.concat("\",\"PROCESS_ID\":\"");
    sMsg.concat(oper.plan_process_id[ucIndx]);
    sMsg.concat("\",\"DATA\":[");

    for (int i = 0; i < (E2PE_LEN_TEA_SEQ - 1) / 3; i++) // 0 - 9
    {
        if (tea.ucTeaSeqStore[ucIndx][i * 3] == TEA_SEQ_END_ID)
            break;

        String sName = getSeqName(tea.ucTeaSeqStore[ucIndx][i * 3]);
        if (sName == "XXX")
            continue;
        uint16_t usValue;
        usValue = (uint16_t)(tea.ucTeaSeqStore[ucIndx][i * 3 + 1] << 8);
        usValue += (uint16_t)(tea.ucTeaSeqStore[ucIndx][i * 3 + 2]);

        if (i != 0)
            sMsg.concat(",");

        sMsg.concat("{\"SEQ_NAME\":\"");
        sMsg.concat(sName);
        sMsg.concat("\",\"SEQ_VALUE\":");
        sMsg.concat(usValue);
        sMsg.concat("}");
    }

    sMsg.concat("]}");

    sTopic = "/Event/Teaching";
    mqtt_publish(sTopic, 2, false, sMsg);
}
/////////////////////////////////////////////////////////////////////////////////////
void Fun_mqtt::send_allTeachingSequence()
{
    for (int i = 0; i < oper.len_plan; i++)
    {
        send_idxTeachingSequence(i);
        delay(500);
    }
}
/////////////////////////////////////////////////////////////////////////////////////
void Fun_mqtt::sen_uidNfcBlank()
{
    ti.getSysTime();

    String sTopic, sMsg;
    sMsg = "{";

    sMsg.concat("\"YEAR\":");
    sMsg.concat(ti.sysTime.tm_year);
    sMsg.concat(",\"MON\":");
    sMsg.concat(ti.sysTime.tm_mon);
    sMsg.concat(",\"DAY\":");
    sMsg.concat(ti.sysTime.tm_mday);
    sMsg.concat(",\"HOUR\":");
    sMsg.concat(ti.sysTime.tm_hour);
    sMsg.concat(",\"MIN\":");
    sMsg.concat(ti.sysTime.tm_min);
    sMsg.concat(",\"SEC\":");
    sMsg.concat(ti.sysTime.tm_sec);
    sMsg.concat(",\"MAC\":\"");
    sMsg.concat(wifi.sMac);
    sMsg.concat("\",\"UID\":\"");
    sMsg.concat(nfc.sUidBlank);
    sMsg.concat("\"}");

    sTopic = "/Event/Nfc/Uid";
    mqtt_publish(sTopic, 2, false, sMsg);
}
/////////////////////////////////////////////////////////////////////////////////////
void Fun_mqtt::send_uid_nfc_inout_iot_mode()
{
    ti.getSysTime();

    String sTopic, sMsg;
    sMsg = "{";

    sMsg.concat("\"YEAR\":");
    sMsg.concat(ti.sysTime.tm_year);
    sMsg.concat(",\"MON\":");
    sMsg.concat(ti.sysTime.tm_mon);
    sMsg.concat(",\"DAY\":");
    sMsg.concat(ti.sysTime.tm_mday);

    if (inOut_mode.get_date_time[inOut_mode.qRear])
    {
        sMsg.concat(",\"HOUR\":");
        sMsg.concat(inOut_mode.hour[inOut_mode.qRear]);
        sMsg.concat(",\"MIN\":");
        sMsg.concat(inOut_mode.min[inOut_mode.qRear]);
        sMsg.concat(",\"SEC\":");
        sMsg.concat(inOut_mode.sec[inOut_mode.qRear]);
    }
    else
    {
        sMsg.concat(",\"HOUR\":");
        sMsg.concat(ti.sysTime.tm_hour);
        sMsg.concat(",\"MIN\":");
        sMsg.concat(ti.sysTime.tm_min);
        sMsg.concat(",\"SEC\":");
        sMsg.concat(ti.sysTime.tm_sec);
    }

    sMsg.concat(",\"MAC\":\"");
    sMsg.concat(wifi.sMac);

    sMsg.concat("\",\"MODE\":\"");
    if (inOut_mode.u8_inout_input_mode[inOut_mode.qRear])
        sMsg.concat("INPUT");
    else
        sMsg.concat("OUTPUT");

    sMsg.concat("\",\"UID\":\"");
    sMsg.concat(inOut_mode.uid[inOut_mode.qRear]);
    sMsg.concat("\"}");

    sTopic = "/Event/Nfc/InOutIoT";
    mqtt_publish(sTopic, 2, false, sMsg);

    inOut_mode.qRear = (inOut_mode.qRear + 1) % INOUT_Q_LEN_BUFF;
}
/////////////////////////////////////////////////////////////////////////////////////
void Fun_mqtt::send_andonMode()
{
    String topic, msg;
    uint16_t usModeBreak = e2p_info.read_16(E2PE_ADR_BREAK_MODE);

    msg = "{";

    msg.concat("\"YEAR\":");
    msg.concat(ti.sysTime.tm_year);
    msg.concat(",\"MON\":");
    msg.concat(ti.sysTime.tm_mon);
    msg.concat(",\"DAY\":");
    msg.concat(ti.sysTime.tm_mday);
    msg.concat(",\"HOUR\":");
    msg.concat(ti.sysTime.tm_hour);
    msg.concat(",\"MIN\":");
    msg.concat(ti.sysTime.tm_min);
    msg.concat(",\"SEC\":");
    msg.concat(ti.sysTime.tm_sec);
    msg.concat(",\"MAC\":\"");
    msg.concat(wifi.sMac);

    if (usModeBreak == MOD_BRK_MACHINE)
    {
        msg.concat("\",\"TYPE\":\"MACHINE\"");
        msg.concat(",\"EVENT\":\"ON\"");
    }
    else if (usModeBreak == MOD_BRK_PERSONAL)
    {
        msg.concat("\",\"TYPE\":\"PERSONAL\"");
        msg.concat(",\"EVENT\":\"ON\"");
    }
    else if (usModeBreak == MOD_BRK_MATERIAL)
    {
        msg.concat("\",\"TYPE\":\"MATERIAL\"");
        msg.concat(",\"EVENT\":\"ON\"");
    }
    else if (usModeBreak == MOD_BRK_ETC)
    {
        msg.concat("\",\"TYPE\":\"ETC\"");
        msg.concat(",\"EVENT\":\"ON\"");
    }
    else
    {
        msg.concat("\",\"TYPE\":\"BREAK\"");
        msg.concat(",\"EVENT\":\"OFF\"");
    }
    msg.concat("}");

    mqtt_publish(TOPIC_EVT_ANDON, 2, false, msg);
}
/////////////////////////////////////////////////////////////////////////////////////
String Fun_mqtt::create_jsonMcTime()
{
    calDapot();
    ti.getSysTime();

    String sMsg = "{";
    sMsg.concat("\"YEAR\":");
    sMsg.concat(ti.sysTime.tm_year);
    sMsg.concat(",\"MON\":");
    sMsg.concat(ti.sysTime.tm_mon);
    sMsg.concat(",\"DAY\":");
    sMsg.concat(ti.sysTime.tm_mday);
    sMsg.concat(",\"HOUR\":");
    sMsg.concat(ti.sysTime.tm_hour);
    sMsg.concat(",\"MIN\":");
    sMsg.concat(ti.sysTime.tm_min);
    sMsg.concat(",\"SEC\":");
    sMsg.concat(ti.sysTime.tm_sec);
    sMsg.concat(",\"MAC\":\"");
    sMsg.concat(wifi.sMac);
    sMsg.concat("\",\"POW_TIME\":");
    sMsg.concat(oper.power_ontime);
    sMsg.concat(",\"MOTO_TIME\":");
    sMsg.concat(oper.motor_runtime / 1000);
    sMsg.concat(",\"ACT_TIME\":");
    sMsg.concat((oper.handling_time + oper.motor_runtime) / 1000);
    sMsg.concat(",\"SUM_STI\":");
    sMsg.concat(oper.sumof_stitch);
    sMsg.concat(",\"SUM_TRI\":");
    sMsg.concat(oper.sumof_trim);
    sMsg.concat("}");

    return sMsg;
}
/////////////////////////////////////////////////////////////////////////////////////
String Fun_mqtt::create_jsonIdxTeacSeq(uint8_t ucIndx)
{
    String sMsg = "{";
    sMsg.concat("\"MAC\":\"");
    sMsg.concat(wifi.sMac);
    sMsg.concat("\",\"PROCESS_ID\":\"");
    sMsg.concat(oper.plan_process_id[ucIndx]);
    sMsg.concat("\",\"DATA\":[");

    for (int i = 0; i < (E2PE_LEN_TEA_SEQ - 1) / 3; i++) // 0 - 9
    {
        if (tea.ucTeaSeqStore[ucIndx][i * 3] == TEA_SEQ_END_ID)
            break;

        String sName = getSeqName(tea.ucTeaSeqStore[ucIndx][i * 3]);
        if (sName == "XXX")
            continue;
        uint16_t usValue;
        usValue = (uint16_t)(tea.ucTeaSeqStore[ucIndx][i * 3 + 1] << 8);
        usValue += (uint16_t)(tea.ucTeaSeqStore[ucIndx][i * 3 + 2]);

        if (i != 0)
            sMsg.concat(",");

        sMsg.concat("{\"SEQ_NAME\":\"");
        sMsg.concat(sName);
        sMsg.concat("\",\"SEQ_VALUE\":");
        sMsg.concat(usValue);
        sMsg.concat("}");
    }

    sMsg.concat("]}");

    return sMsg;
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

/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////