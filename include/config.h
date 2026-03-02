
#ifndef CONFIG_H
#define CONFIG_H

/******************************************************************************************************/
#include <Arduino.h>

/******************************************************************************************************/
/******************************************************************************************************/
// #define ESP_ENABLE_DEBUG

#ifdef ESP_ENABLE_DEBUG
// #define ESP_DEBUG_WIFI
// #define ESP_DEBUG_LCD
// #define ESP_DEBUG_MQTT
#define ESP_DEBUG_E2P
// #define ESP_DEBUG_NFC
// #define ESP_DEBUG_MESH
#define ESP_DEBUG_OPER
#define ESP_DEBUG_TEASEQ

#endif

/******************************************************************************************************/
/***************** **** *****************/
#define E2PE_INIT_CODE_INFO 0x86
#define E2PE_INIT_CODE_DATA 0x86
#define E2PE_INIT_CODE_NETWORK 0x86
/***************** *** *****************/
#define HANDLING_TIME_DEFAULT 60 // seconds
#define LEN_JSON_OBJECT_PLAN 4
/***************** PRODUCT *****************/
// type product
#define Q_TYPE_OUTPUT 0
#define Q_TYPE_DEFECT 1
#define Q_TYPE_ANDON 2
/***************** WIFI *****************/
#define WIFI_NO_ERR 0x00
#define WIFI_ERR_AP 0x01
#define WIFI_ERR_IP 0x02
#define WIFI_ERR_RSSI 0x03
#define WIFI_ERR_MQTT 0x04
/***************** NFC *****************/
#define NFC_TAG_TYPE_BLANK 0x40
#define NFC_TAG_TYPE_NETWORK 0x41
#define NFC_TAG_TYPE_OUTPUT 0x42

/***************** MESH *****************/
#define MESH_MODE_NODE 0x01
#define MESH_MODE_BRIDGE 0x00
/***************** MQTT *****************/
#define MQTT_SIG_DISCONNECTED 0x00
#define MQTT_SIG_CONNECTED 0x01
/***************** ETC *****************/
#define TIME_HOUR_RST_NEW_DAY 6 // 6 A.M

/***************** E2P INFO *****************/
#define E2PE_LEN_INIT 1

#define E2PE_LEN_OUTPUT 4
#define E2PE_LEN_DEFECT 4
#define E2PE_LEN_TARGET 4
//--
#define E2PE_LEN_DAY_RESET 1
#define E2PE_LEN_MON_RESET 1
//--
#define E2PE_LEN_DIS_ROTATE 1
#define E2PE_LEN_BREAK_MODE 2
#define E2PE_LEN_INDX_PLAN 1
// worker
#define E2PE_LEN_WRK_ID 8
#define E2PE_LEN_WRK_NA 16
#define E2PE_LEN_WRK_JOB_CD 8
#define E2PE_LEN_WRK_SKILL 1
#define E2PE_LEN_WRK_UID 20
// machine
#define E2PE_LEN_MC_ID 9
#define E2PE_LEN_MC_SUPY 10
// process
#define E2PE_LEN_STYLE 30
#define E2PE_LEN_PROC 20
#define E2PE_LEN_PROC_SEQ 2
#define E2PE_LEN_PROC_ID 3
// location
#define E2PE_LEN_COUNTRY 16
#define E2PE_LEN_FACTORY 4
#define E2PE_LEN_LINE 16
#define E2PE_LEN_POSITION 2
// network
#define E2PE_LEN_SSID 16
#define E2PE_LEN_PASS 16
#define E2PE_LEN_SVR_ADDR 32
#define E2PE_LEN_SVR_PORT 2
// machine time
#define E2PE_LEN_MC_DAPOT 4
#define E2PE_LEN_MC_HANLDE 4
#define E2PE_LEN_MC_DAMRT 4
#define E2PE_LEN_MC_NUM_STI 4
#define E2PE_LEN_MC_NUM_TRI 4
#define E2PE_LEN_MC_CT 4
// delay SEN_0
#define E2PE_LEN_TEA_DLY_SEN0 2
//...
// len teaching sequence
#define E2PE_LEN_TEA_SEQ 31
//----
#define E2PE_LEN_OUT_SCALE 2
#define E2PE_LEN_DAI_OUT_RST 1
//----
#define E2PE_LEN_NON_SEW_MC 1
//----
#define E2PE_LEN_ANDON_MODE 2
//---
#define E2PE_LEN_LEN_BLA_NFC 1
#define E2PE_LEN_UID_BLA_NFC 10
#define E2PE_LEN_SEN_BLA_NFC 1
#define E2PE_LEN_NFC_UID_STR 20
//---
#define E2PE_LEN_CHK_RST_OUT 1
//----
#define E2PE_LEN_HOUR_OLD 1
//----
#define E2PE_LEN_INOUT_MODE 1
#define E2PE_LEN_IGNORE_SEN0_TEA 1
#define E2PE_LEN_DIR_ANDON_MC 1
//----
// addr info begin
#define E2PE_ADR_INIT_INFO 0
// mc id
#define E2PE_ADR_MC_ID E2PE_ADR_INIT_INFO + E2PE_LEN_INIT
#define E2PE_ADR_MC_SUPY E2PE_ADR_MC_ID + E2PE_LEN_MC_ID
// factory
#define E2PE_ADR_FACTORY E2PE_ADR_MC_SUPY + E2PE_LEN_MC_SUPY
#define E2PE_ADR_LINE E2PE_ADR_FACTORY + E2PE_LEN_FACTORY
#define E2PE_ADR_POSITION E2PE_ADR_LINE + E2PE_LEN_LINE
// worker
#define E2PE_ADR_WRK_ID E2PE_ADR_POSITION + E2PE_LEN_POSITION
#define E2PE_ADR_WRK_NA E2PE_ADR_WRK_ID + E2PE_LEN_WRK_ID
#define E2PE_ADR_WRK_SKILL E2PE_ADR_WRK_NA + E2PE_LEN_WRK_NA
// process
#define E2PE_ADR_STYLE E2PE_ADR_WRK_SKILL + E2PE_LEN_WRK_SKILL
#define E2PE_ADR_PROC E2PE_ADR_STYLE + E2PE_LEN_STYLE
#define E2PE_ADR_PROC_SEQ E2PE_ADR_PROC + E2PE_LEN_PROC
//--
#define E2PE_ADR_INDX_PLAN E2PE_ADR_PROC_SEQ + E2PE_LEN_PROC_SEQ
#define E2PE_ADR_DAY_RESET E2PE_ADR_INDX_PLAN + E2PE_LEN_INDX_PLAN
#define E2PE_ADR_DIS_ROTATE E2PE_ADR_DAY_RESET + E2PE_LEN_DAY_RESET
#define E2PE_ADR_BREAK_MODE E2PE_ADR_DIS_ROTATE + E2PE_LEN_DIS_ROTATE
// addr teaching
//--teaching running
#define E2PE_ADR_TEA_SEQ E2PE_ADR_BREAK_MODE + E2PE_LEN_BREAK_MODE
//--teaching store - follow LEN_JSON_OBJECT_PLAN*31 = 124 address
#define E2PE_ADR_OUT_SEQ E2PE_ADR_TEA_SEQ + E2PE_LEN_TEA_SEQ
// delay SEN_0
#define E2PE_ADR_TEA_DLY_SEN0 E2PE_ADR_OUT_SEQ + (LEN_JSON_OBJECT_PLAN * E2PE_LEN_TEA_SEQ)
// output scale
#define E2PE_ADR_OUT_SCALE E2PE_ADR_TEA_DLY_SEN0 + E2PE_LEN_TEA_DLY_SEN0
// daily output reset
#define E2PE_ADR_DAI_OUT_RST E2PE_ADR_OUT_SCALE + E2PE_LEN_OUT_SCALE
// flag send uid blank nfc to server
#define E2PE_ADR_LEN_BLA_NFC E2PE_ADR_DAI_OUT_RST + E2PE_LEN_DAI_OUT_RST
#define E2PE_ADR_UID_BLA_NFC E2PE_ADR_LEN_BLA_NFC + E2PE_LEN_LEN_BLA_NFC
#define E2PE_ADR_SEN_BLA_NFC E2PE_ADR_UID_BLA_NFC + E2PE_LEN_UID_BLA_NFC
//--
#define E2PE_ADR_MON_RESET E2PE_ADR_SEN_BLA_NFC + E2PE_LEN_SEN_BLA_NFC
// in/out iot mode
#define E2PE_ADR_INOUT_MODE E2PE_ADR_MON_RESET + E2PE_LEN_MON_RESET
// ignore sen0 teaching sequence
#define E2PE_ADR_IGNORE_SEN0_TEA E2PE_ADR_INOUT_MODE + E2PE_LEN_INOUT_MODE
// dir. andon mc
#define E2PE_ADR_DIR_ANDON_MC E2PE_ADR_IGNORE_SEN0_TEA + E2PE_LEN_IGNORE_SEN0_TEA
// nfc_uid_str
#define E2PE_ADR_NFC_UID_STR E2PE_ADR_DIR_ANDON_MC + E2PE_LEN_DIR_ANDON_MC
//--
#define E2PE_ADR_BEGIN_INFO E2PE_ADR_NFC_UID_STR + E2PE_LEN_NFC_UID_STR
/***************** E2P DATA *****************/
#define LEN_BYTE_PRODUCT_DATA 4
//---
#define E2PE_LEN_Q_FRONT 2
#define E2PE_LEN_Q_REAR 2
//---
#define E2PE_LEN_Q_BUFF 300
#define E2PE_LEN_Q_DATA 12
//---
#define Q_BUF_IDX_YEAR 0      // 0
#define Q_BUF_IDX_MON 1       // 1
#define Q_BUF_IDX_DAY 2       // 2
#define Q_BUF_IDX_HOUR 3      // 3
#define Q_BUF_IDX_MIN 4       // 4
#define Q_BUF_IDX_SEC 5       // 5
#define Q_BUF_IDX_TYPE 6      // 6
#define Q_BUF_IDX_DATA 7      // 7-8-9-10
#define Q_BUF_IDX_CUR_PLAN 11 // 11
//---
#define E2PE_ADR_INIT_DATA E2PE_LEN_Q_BUFF *E2PE_LEN_Q_DATA      // 3600 bytes
#define E2PE_ADR_DATA_Q_FRONT E2PE_ADR_INIT_DATA + E2PE_LEN_INIT // address = 300*12 = 3600
#define E2PE_ADR_DATA_Q_REAR E2PE_ADR_DATA_Q_FRONT + E2PE_LEN_Q_FRONT

#define E2PE_ADR_DATA_OUTPUT_1 E2PE_ADR_DATA_Q_REAR + E2PE_LEN_Q_REAR
#define E2PE_ADR_DATA_DEFECT_1 E2PE_ADR_DATA_OUTPUT_1 + E2PE_LEN_OUTPUT
#define E2PE_ADR_DATA_TARGET_1 E2PE_ADR_DATA_DEFECT_1 + E2PE_LEN_DEFECT

#define E2PE_ADR_DATA_OUTPUT_2 E2PE_ADR_DATA_TARGET_1 + E2PE_LEN_TARGET
#define E2PE_ADR_DATA_DEFECT_2 E2PE_ADR_DATA_OUTPUT_2 + E2PE_LEN_OUTPUT
#define E2PE_ADR_DATA_TARGET_2 E2PE_ADR_DATA_DEFECT_2 + E2PE_LEN_DEFECT

#define E2PE_ADR_DATA_OUTPUT_3 E2PE_ADR_DATA_TARGET_2 + E2PE_LEN_TARGET
#define E2PE_ADR_DATA_DEFECT_3 E2PE_ADR_DATA_OUTPUT_3 + E2PE_LEN_OUTPUT
#define E2PE_ADR_DATA_TARGET_3 E2PE_ADR_DATA_DEFECT_3 + E2PE_LEN_DEFECT

#define E2PE_ADR_DATA_OUTPUT_4 E2PE_ADR_DATA_TARGET_3 + E2PE_LEN_TARGET
#define E2PE_ADR_DATA_DEFECT_4 E2PE_ADR_DATA_OUTPUT_4 + E2PE_LEN_OUTPUT
#define E2PE_ADR_DATA_TARGET_4 E2PE_ADR_DATA_DEFECT_4 + E2PE_LEN_DEFECT
//---
// mc time
#define E2PE_ADR_MC_DAPOT E2PE_ADR_DATA_TARGET_4 + E2PE_LEN_TARGET
#define E2PE_ADR_MC_DAMRT E2PE_ADR_MC_DAPOT + E2PE_LEN_MC_DAPOT
#define E2PE_ADR_MC_HANDLE E2PE_ADR_MC_DAMRT + E2PE_LEN_MC_DAMRT
#define E2PE_ADR_MC_CT E2PE_ADR_MC_HANDLE + E2PE_LEN_MC_HANLDE
#define E2PE_ADR_MC_NUM_STI E2PE_ADR_MC_CT + E2PE_LEN_MC_CT
#define E2PE_ADR_MC_NUM_TRI E2PE_ADR_MC_NUM_STI + E2PE_LEN_MC_NUM_STI
//---
#define E2PE_ADR_BEGIN_DATA E2PE_ADR_MC_NUM_TRI + E2PE_LEN_MC_NUM_TRI
/***************** E2P NETWORK *****************/
// addr network begin
#define E2PE_ADR_INIT_NETWORK 0
// network
#define E2PE_ADR_SSID E2PE_ADR_INIT_NETWORK + E2PE_LEN_INIT
#define E2PE_ADR_PASS E2PE_ADR_SSID + E2PE_LEN_SSID
#define E2PE_ADR_SVR_ADDR E2PE_ADR_PASS + E2PE_LEN_PASS
#define E2PE_ADR_SVR_PORT E2PE_ADR_SVR_ADDR + E2PE_LEN_SVR_ADDR
// non-sewing machine
#define E2PE_ADR_NON_SEW_MC E2PE_ADR_SVR_PORT + E2PE_LEN_SVR_PORT
//---
#define E2PE_ADR_BEGIN_NETWORK E2PE_ADR_NON_SEW_MC + E2PE_LEN_NON_SEW_MC
/***************** *** *****************/
// on, off break mode
#define MOD_BRK_OFF 0xFFFF
#define MOD_BRK_MACHINE 0x0001
#define MOD_BRK_PERSONAL 0x0010
#define MOD_BRK_MATERIAL 0x0100
#define MOD_BRK_ETC 0x1000

// #define MOD_BRK_OFF 0xFF
// #define MOD_BRK_MACHINE 1
// #define MOD_BRK_PERSONAL 2
// #define MOD_BRK_MATERIAL 3
// #define MOD_BRK_ETC 4

#define LEN_BREAK_REASON 4

// --lcd switch case-- //
#define LCD_SEQ_UPDATE 0x00 // case update for raw display
#define LCD_SEQ_WAIT 0x10   // case wait for show 1 second
// -- lcd page-- //
#define LCD_DISP_MODE_QC 0x00     // lcd display QC
#define LCD_DISP_MODE_WORKER 0x01 // lcd display Worker

#define LCD_DISP_MAIN 0x00    // lcd main
#define LCD_DISP_TEA_OUT 0x10 // lcd teaching sequence
#define LCD_DISP_NETWORK 0x24 // lcd network
// lcd break mode
#define LCD_DISP_ANDON_MENU 0x30 // menu for choose
#define LCD_DISP_ANDON 0x31      // show blink blink
// lcd nfc
#define LCD_DISP_NFC_WRT 0x40 // show nfc write mode
#define LCD_DISP_NFC_RED 0x41 // show nfc read mode
#define LCD_DISP_NFC_ERR 0x42 // show error when detect nfc
// another page
#define LCD_DISP_CONFIRM 0x50    // set default network factory
#define LCD_DISP_DEB_SENSOR 0x51 // debug sensor

#define LCD_DISP_GEN_MAC 0x60 // generate mac address

#define LCD_DISP_MENU_SETUP 0x70     // lcd disp menu setup
#define LCD_DISP_MENU_INFO 0x71      // lcd disp menu info.
#define LCD_DISP_MENU_SETTING 0x72   // lcd disp menu setting
#define LCD_DISP_MENU_NFC 0x73       // lcd disp menu nfc
#define LCD_DISP_MENU_AUTO_TEST 0x74 // lcd disp menu nfc
#define LCD_DISP_NFC_COPY 0x75       // lcd disp nfc copy increase output with scale

/***************** NFC *****************/
#define RFID_TAG_TYPE_INFO_PGE 4

#define RFID_TAG_TYPE_EARSE 0x00
#define RFID_TAG_TYPE_NETWORK 0x03
#define RFID_TAG_TYPE_OUTPUT 0x08

#define RFID_PGE_END_NETWORK_ULT 22
#define RFID_PGE_END_OUTPUT_ULT 1

#define RFID_PGE_END_EARSE_M1K 0
#define RFID_PGE_END_NETWORK_M1K 6
#define RFID_PGE_END_OUTPUT_M1K 1

#define RFID_MOD_READ 0x00
#define RFID_MOD_WRITE 0x01
#define RFID_MOD_RES_DYNAMIC 0x02
#define RFID_MOD_WAI_WRITE 0x03

#define NFC_HANDLE_READ_DATA 0x00
#define NFC_HANDLE_CHA_PRO_ID 0x01
#define NFC_HANDLE_SHO_MAC_QR 0x02

#define NFC_READ_WRITE_ANDON 0x00
#define NFC_READ_WRITE_OUTPUT 0x01
#define NFC_READ_WRITE_MAC_QR 0x02

#define RFID_WRT_TI_OUT_CNT 7
#define RFID_DATA_LEN 140 // 4*(39-4)

#define NFC_INCDEC_INCREASE 0
#define NFC_INCDEC_DECREASE 1
/***************** RGB *****************/
#define LED_1_OFF B00000000
#define LED_1_RED B00001000
#define LED_1_GREEN B00000100
#define LED_1_BLUE B00000010
#define LED_1_YELLOW B00001100
#define LED_1_PINK B00001010
#define LED_1_B_G B00000110
#define LED_1_WHITE B00001110
#define LED_OFF B00000000
//             0 000   000   0
//             0 LED_2 LED_1 0

typedef struct LED_RGB
{
    uint8_t ucFlagChangeColor;

    uint8_t ucColor;
    uint8_t ucColorLed[2];
    uint8_t ucBlink;
    uint16_t usInterval;

    uint32_t usTimeOld;

    uint8_t ucActive;
    uint8_t ucColorAct;

    uint8_t ucChkTurnOffLedRGB;
    uint32_t ulMilShowLedRGB;

    uint32_t ulTimeTurnOn;
} LED_RGB;

extern LED_RGB Led;
/******** Teaching sequence ***********/
#define TEA_SEQ_SEN0_ID 0x30 // same version 000C
#define TEA_SEQ_SEN1_ID 0x31
#define TEA_SEQ_SEN2_ID 0x32
#define TEA_SEQ_SOL0_ID 0x40
#define TEA_SEQ_DLY_ID 0x70
#define TEA_SEQ_END_ID 0xFF // flag end teaching
/******************************************************************************************************/
typedef struct IoT_Sub
{
    String sHwVer;
    String sSwVer;
    String sVer;

    String sIotSupplier;
    String sIotModel;
} IoT_Sub;
extern IoT_Sub Iot;
/////////////////////////////////////////////////////////////////////////////////////
typedef struct INFO
{
    // Info Worker
    String sWorkerId;
    String sWorkerName;
    String sSkill;

    // Info Machine
    String sMachineSupply;
    String sMachineId;

    // Info Process
    String sStype;
    String sProcess;
    String sProcessId;
    String sProcessSeq;

    uint8_t ucFlagESPRestart; // flag restart

    uint8_t ucDaiOutRst;   // daily output reset
    uint8_t ucRecDaiOutRs; // daily output reset default
    uint8_t ucFlagRsWhenNewDay;
    uint32_t ulMilDaiOutRs;

    uint8_t ucSumCylce;
    uint32_t ulMilOutOld;

    uint8_t ucOnTime;

    uint8_t ucFlagCalCycleTime;

} INFO;
extern INFO Inf;
/////////////////////////////////////////////////////////////////////////////////////
#define INOUT_Q_LEN_BUFF 10
typedef struct IN_OUT_MODE
{
    uint8_t qFont;
    uint8_t qRear;

    uint8_t hour[INOUT_Q_LEN_BUFF];
    uint8_t min[INOUT_Q_LEN_BUFF];
    uint8_t sec[INOUT_Q_LEN_BUFF];

    bool get_date_time[INOUT_Q_LEN_BUFF];

    bool u8_inout_input_mode[INOUT_Q_LEN_BUFF];

    String uid[INOUT_Q_LEN_BUFF];

} IN_OUT_MODE;
extern IN_OUT_MODE inOut_mode;
/////////////////////////////////////////////////////////////////////////////////////
typedef struct DEBUG_INFO
{
    bool send_debug;
    String topic_debug;
    String msg_debug;

} DEBUG_INFO;
extern DEBUG_INFO debug;
/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////
/******************************************************************************************************/
// oper
void reset_mc_time();
void save_mc_time();
void calDapot();
/******************************************************************************************************/
// teaching sequence
uint8_t get_statusSen0();
void teaSeq_detect_signal();
void update_storeTeachingSequence();
void rst_teachingSequenceRun();
void rst_idxTeachingSequence(uint8_t idx);
void rst_allTeachingSequence();
void read_allTeachingSequence();
void save_allTeachingSequence();
void teaseq_increaseDelayTeachMode();
void teaseq_increaseDelayRunMode();
void teaseq_startTeachMode();
void teaseq_endTeachMode();
void teqseq_restartRunMode();
void update_runTeaSeqWithCurrentIdxPlan();
/////////////////////////////////////////////////////////////////////////////////////
// eeprom infomation
uint8_t e2p_info_read_8(uint16_t usAddr);
uint16_t e2p_info_read_16(uint16_t usAddr);
uint32_t e2p_info_read_32(uint16_t usAddr);
String e2p_info_E2PtoString(uint16_t usAddr, uint8_t ucLen);
void e2p_info_write_8(uint16_t usAddr, uint8_t ucData);
void e2p_info_write_16(uint16_t usAddr, uint16_t ucData);
void e2p_info_write_32(uint16_t usAddr, uint32_t ucData);
void e2p_info_StringToE2P(uint16_t usAddr, uint8_t ucLen, String stStr);
// eeprom data
uint8_t e2p_data_read_8(uint16_t usAddr);
uint16_t e2p_data_read_16(uint16_t usAddr);
uint32_t e2p_data_read_32(uint16_t usAddr);
void e2p_data_write_8(uint16_t usAddr, uint8_t ucData);
void e2p_data_write_16(uint16_t usAddr, uint16_t ucData);
void e2p_data_write_32(uint16_t usAddr, uint32_t ucData);
// eeprom function
void init_e2p();
void E2P_ResetE2p_network();
void E2P_ResetE2p_data();
void E2P_ResetE2p_info();
void E2P_UpdateE2pVal();
void updateTimeEEPROM();
void resetDataWhenNewDay(bool timer);
void updateOutputValueEEPROM();
void chkAndonMode();
void get_uid_blank();
void check_uid_blank_problem();
/////////////////////////////////////////////////////////////////////////////////////
// operation
void reset_production();
void rst_Qfont_Qrear();
/////////////////////////////////////////////////////////////////////////////////////
// etc
void init_task();
void loop_task();
void init_etc();
void loop_etc();
void write_color(uint8_t ucRGB_Data);
void active_led_nfc_success();
void active_led_nfc_fail();
/////////////////////////////////////////////////////////////////////////////////////
// lcd
void lcd_refresh();
/////////////////////////////////////////////////////////////////////////////////////
// mesh
void init_mesh();
void loop_mesh();
/////////////////////////////////////////////////////////////////////////////////////
// mqtt
void mqtt_disconnect();
/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////

/******************************************************************************************************/
#endif // CONFIG_H