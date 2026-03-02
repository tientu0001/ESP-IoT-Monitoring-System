#ifndef FUN_MQTT_H
#define FUN_MQTT_H
/////////////////////////////////////////////////////////////////////////////////////
#include <RingBuf.h>
#include <AsyncMqttClient.h>

#define LEN_QUEUE_PUBLISH 5

class Fun_mqtt
{
private:
    String create_jsonMcTime();

public:
    static const uint8_t SEND_SYSCTRL = 0x00;
    static const uint8_t SEND_DATA = 0x01;
    uint8_t mqttStep;

    static const uint8_t SEND_INFOMATION = 0x00; // don't check ACK from bridge
    static const uint8_t SEND_PRODUCT = 0x01;    // check ACK from bridge
    uint8_t ucSendType;

    // functions
    Fun_mqtt();
    ~Fun_mqtt();

    void init();
    void loop();

    void connectToMqtt();
    void mqttSubscribe();
    void mqttMessage(char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total);
    void mqttCommunicateSvr();

    void mqtt_publish(String topic, uint8_t qos, bool retain, String payload);
    void mqtt_publish_data(String topic, uint8_t qos, bool retain, String payload);

    void FUNC_SEND_INFO();
    void FUNC_SEND_DATA();
    void SEND_DATA_OUTPUT();
    void send_mc_time();

    bool check_get_data_from_server();
    String msgSendInfo(String sStatus);

    bool bChkRstNewDay;
    bool check_reset_newday();

    void getDateTime(String data);
    void getPlan(String data);
    void getInfConfig(String data);
    void getOta(String data);
    void updateCount(String data);
    void setTeaching(String data);
    //--
    void debugInfo(String data);
    void getDebugOTA(String data);
    void checkPing(String data);

    String createJsonCount(uint8_t *ucQ_Data);
    String create_jsonIdxTeacSeq(uint8_t ucIndx);

    String getSeqName(uint8_t ucId);
    void send_idxTeachingSequence(uint8_t ucIndx);
    void send_allTeachingSequence();
    void sen_uidNfcBlank();
    void send_andonMode();

    void send_uid_nfc_inout_iot_mode();

    // debug
    bool b_send_mqtt_disconnected;
    uint8_t u8_mqtt_dis_reason;

    ////////////////////////////////////////////////////
    bool bConnected;
    bool bDisconnected;

    bool bMqttSubscribe;
    ////////////////////////////////////////////////////
    // mesh
    bool mqttEmitSignal;
    uint8_t mqttSignal;

    ////////////////////////////////////////////////////
    // variables
    uint32_t statusChange;
    uint32_t ulMilTryConnect;
    uint8_t u8_cnt_restart_mqtt_reason;

    String sSvrAddr;
    uint16_t usSvrPort;

    uint8_t ucConnected;
    uint8_t ucDisconnected;
    uint8_t ucChkSendLocation;

    uint16_t usLenMsg;
    String sMsg;

    String sSubTopicPlan;      // get plan
    String sSubTopicConfig;    // get config
    String sSubTopicOta;       // ota when server active
    String sSubTopicOtaAuto;   // ota when iot power on
    String sEventUpdate;       // update output & defect
    String sSubTopLedRGB;      // control LED RGB
    String sSubTopSetMachine;  // set machine id
    String sSubTopSetTeaching; // set teaching
    String sSubTopGetConfig;   // get info config(worker teaching enable)
    String sSubWokerInfo;      // get worker info

    String sDebugInfo; // get info
    String debugOTA;   // get info

    String debugPing; // get info

    uint8_t ucFlaDonSenTeaSeq;
    //////////////////////////////////////////////////////
    RingBuf<uint16_t, LEN_QUEUE_PUBLISH> usPacketId; // buffer packetId
    RingBuf<String, LEN_QUEUE_PUBLISH> sTopic;       // buffer topic
    RingBuf<String, LEN_QUEUE_PUBLISH> sPayload;     // buffer payload
    //////////////////////////////////////////////////////
};
/////////////////////////////////////////////////////////////////////////////////////
extern Fun_mqtt mqtt;
#endif // FUN_MQTT_H