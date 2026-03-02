

//************************************************************************************************
#include "config.h"
#include "Ui_lcd.h"

#include "Fun_operation.h"
#include "Fun_time.h"
#include "Fun_nfc.h"
#include "Fun_wifi.h"
#include "Fun_mqtt.h"
#include "Fun_teaSeq.h"

#include "Fun_e2p.h"

// #include "Fun_test.h"

#include <SSD1306Wire.h> // legacy: #include "SSD1306.h"
#include <qrcode.h>
//************************************************************************************************
//************************************************************************************************
#define PAGE_ID_MAIN 0x00
//
#define PAGE_ID_MENU 0x10
//
#define PAGE_ID_ANDON 0x20
#define PAGE_ID_ANDON_MC 0x21
#define PAGE_ID_ANDON_PER 0x22
#define PAGE_ID_ANDON_MAT 0x23
#define PAGE_ID_ANDON_ETC 0x24
#define PAGE_ID_ANDON_SUB 0x25
//
#define PAGE_ID_CHG_PROC 0x30
//
#define PAGE_ID_INFO 0x40
#define PAGE_ID_INFO_NET 0x41
#define PAGE_ID_INFO_MC 0x42
#define PAGE_ID_INFO_PLAN 0x43
//
#define PAGE_ID_WEB_CFG 0x50
//
#define PAGE_ID_SETUP 0x60
#define PAGE_ID_SETUP_RST_TCH 0x61
#define PAGE_ID_SETUP_RST_OUTDEF 0x62
#define PAGE_ID_SETUP_RST_FAC 0x63
#define PAGE_ID_SETUP_ROT_DISP 0x64
#define PAGE_ID_SETUP_MAC_QR 0x65
#define PAGE_ID_SETUP_DLY_SEN0 0x66
#define PAGE_ID_SETUP_IGNORE_SEN0_TEASEQ 0x67
#define PAGE_ID_SETUP_INOUT_IOT_MODE 0x68
#define PAGE_ID_SETUP_DIR_AND_MC 0x69
//
#define PAGE_ID_NFC 0x70
#define PAGE_ID_NFC_COPY_OUTPUT 0x71
#define PAGE_ID_NFC_TAG_HANLDE 0x72
#define PAGE_ID_NFC_TAG_WRITE 0x73
#define PAGE_ID_NFC_TAG_READ 0x74
//
#define PAGE_ID_TEACH 0x80
//
#define PAGE_ID_SHOW_TEXT 0x90

////// len row
// nfc
#define PAGE_LEN_ROW_NFC 5
#define PAGE_LEN_ROW_COPY_OUTPUT 3
// info
#define PAGE_LEN_ROW_NETWORK 8
#define PAGE_LEN_ROW_MACHINE 4
#define PAGE_LEN_ROW_PLAN 8

#define QR_CANVAS_SIZE 64
#define QR_SCALE 2
#define QR_VERSION 2

#define NFC_TIME_WAIT_HANLDE 3 // s
#define NFC_TIME_WAIT_WRITE 7
#define NFC_TIME_WAIT_READ 3
/******************************************************************************************************/
#define LCD_SETUP_ROW_COUNT 11
/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
UiLcd lcd;
SSD1306Wire oled(0x3c, 5, 4); // ADDRESS, SDA, SCL  -  SDA and SCL usually populate automatically based on your board's pins_arduino.h e.g. https://github.com/esp8266/Arduino/blob/master/variants/nodemcu/pins_arduino.h
/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
String getClock(uint16_t usSecond)
{
  uint8_t ucHour, ucMinute, ucSecond;
  ucHour = usSecond / 3600;
  ucMinute = (usSecond - ucHour * 3600) / 60;
  ucSecond = usSecond - ucHour * 3600 - ucMinute * 60;

  String hour = String(ucHour);
  if (hour.length() == 1)
    hour = "0" + hour;

  String minute = String(ucMinute);
  if (minute.length() == 1)
    minute = "0" + minute;

  String second = String(ucSecond);
  if (second.length() == 1)
    second = "0" + second;

  return hour + ":" + minute + ":" + second;
}
//************************************************************************************************
void lcd_refresh()
{
  lcd.bRefresh = true;
}
//************************************************************************************************
// uint8_t test_lcd;
void UiLcd::btn_short()
{
  teaseq_increaseDelayTeachMode();

  if (lcd.ucPageCurr == PAGE_ID_MAIN)
  {
    if (oper.in_out_iot_mode_enable)
      oper.in_out_input_mode = !oper.in_out_input_mode;
    else if (oper.dir_andon_mc_mode)
    {
      // change to andon mc
      lcd.list_index = 0;
      lcd.disp_andon_sub();
    }
    else
      oper.increase_output();
  }
  else if (lcd.ucPageCurr == PAGE_ID_MENU)
  {
    if (lcd.list_index < 7)
      lcd.list_index++;
    else
      lcd.list_index = 0;
    lcd.disp_menu();
  }
  else if (lcd.ucPageCurr == PAGE_ID_ANDON)
  {
    if (lcd.list_index < 4)
      lcd.list_index++;
    else
      lcd.list_index = 0;
    lcd.disp_andon();
  }
  else if (lcd.ucPageCurr == PAGE_ID_ANDON_SUB)
  {
    // Execute ANDON off sequence here.

    e2p_info.write_16(E2PE_ADR_BREAK_MODE, MOD_BRK_OFF);
    e2p_info.commit_flag = true;

    oper.bAndonMode = false;
    oper.bSendAndon = true;

    // debug
    debug.msg_debug.concat(" --/ lcd_andon_mode: ");
    debug.msg_debug.concat(String(MOD_BRK_OFF, 16));
    debug.send_debug = true;

    teqseq_restartRunMode();

    lcd.disp_main();

    // LED OFF
    Led.ucActive = 1;
    Led.ucColor = LED_OFF;
    Led.ulTimeTurnOn = 0;
  }
  else if (lcd.ucPageCurr == PAGE_ID_INFO)
  {
    if (lcd.list_index < 3)
      lcd.list_index++;
    else
      lcd.list_index = 0;
    lcd.disp_info();
  }
  else if (lcd.ucPageCurr == PAGE_ID_INFO_NET)
  {
    lcd.list_index = (lcd.list_index + 1) % PAGE_LEN_ROW_NETWORK;
    lcd.disp_info_network();
  }
  else if (lcd.ucPageCurr == PAGE_ID_INFO_MC)
  {
    lcd.list_index = (lcd.list_index + 1) % PAGE_LEN_ROW_MACHINE;
    lcd.disp_info_machine();
  }
  else if (lcd.ucPageCurr == PAGE_ID_INFO_PLAN)
  {
    lcd.list_index = (lcd.list_index + 1) % PAGE_LEN_ROW_PLAN;
    lcd.disp_info_plan();
  }
  else if (lcd.ucPageCurr == PAGE_ID_SETUP)
  {
    if (lcd.list_index < LCD_SETUP_ROW_COUNT - 1)
      lcd.list_index++;
    else
      lcd.list_index = 0;
    lcd.disp_setup();
  }
  else if (lcd.ucPageCurr == PAGE_ID_SETUP_MAC_QR)
  {
    oled.normalDisplay();
    lcd.disp_main();
  }
  else if (lcd.ucPageCurr == PAGE_ID_NFC)
  {
    lcd.list_index = (lcd.list_index + 1) % PAGE_LEN_ROW_NFC;
    lcd.disp_nfc();
  }
  else if (lcd.ucPageCurr == PAGE_ID_NFC_COPY_OUTPUT)
  {
    lcd.list_index = (lcd.list_index + 1) % PAGE_LEN_ROW_COPY_OUTPUT;
    lcd.disp_nfc_copy_output();
  }
  else if (lcd.ucPageCurr == PAGE_ID_SETUP_DLY_SEN0)
  {
    if (lcd.list_index < 2)
      lcd.list_index++;
    else
      lcd.list_index = 0;
    lcd.disp_setup_setDlySen0();
  }
  else if (lcd.ucPageCurr == PAGE_ID_SETUP_INOUT_IOT_MODE)
  {
    if (lcd.list_index < 2)
      lcd.list_index++;
    else
      lcd.list_index = 0;
    lcd.disp_setup_inOut_mode();
  }
  else if (lcd.ucPageCurr == PAGE_ID_SETUP_IGNORE_SEN0_TEASEQ)
  {
    if (lcd.list_index < 2)
      lcd.list_index++;
    else
      lcd.list_index = 0;
    lcd.disp_setup_ignore_sen0_teaseq();
  }
  else if (lcd.ucPageCurr == PAGE_ID_SETUP_DIR_AND_MC)
  {
    if (lcd.list_index < 2)
      lcd.list_index++;
    else
      lcd.list_index = 0;
    lcd.disp_setup_dir_andon_mc();
  }
}
//-----------------------------------------------------------
void UiLcd::btn_long_1()
{

  if (lcd.ucPageCurr == PAGE_ID_MAIN)
  {
    lcd.list_index = 0;
    lcd.disp_menu();
  }
  else if (lcd.ucPageCurr == PAGE_ID_MENU)
  {
    if (lcd.list_index == 0)
      lcd.disp_andon();
    else if (lcd.list_index == 1)
      lcd.disp_changeProc();
    else if (lcd.list_index == 2)
    {
      lcd.list_index = 0;
      lcd.disp_info();
    }
    else if (lcd.list_index == 3)
    {
      lcd.disp_webConfig();
    }
    else if (lcd.list_index == 4)
    {
      lcd.list_index = 0;
      lcd.disp_setup();
    }
    else if (lcd.list_index == 5)
    {
      lcd.list_index = 0;
      lcd.disp_nfc();
    }
    else
      lcd.disp_main();
  }
  else if (lcd.ucPageCurr == PAGE_ID_ANDON)
  {
    if (lcd.list_index >= LEN_BREAK_REASON)
    {
      lcd.disp_main();
    }
    else
      lcd.disp_andon_sub();
  }
  else if (lcd.ucPageCurr == PAGE_ID_INFO)
  {
    if (lcd.list_index == 0)
    {
      lcd.list_index = 0;
      lcd.disp_info_network();
    }
    else if (lcd.list_index == 1)
    {
      lcd.list_index = 0;
      lcd.disp_info_machine();
    }
    else if (lcd.list_index == 2)
    {
      lcd.list_index = 0;
      lcd.disp_info_plan();
    }
    else if (lcd.list_index == 3)
      lcd.disp_main();
    else
      lcd.disp_main();
  }
  else if ((lcd.ucPageCurr > PAGE_ID_INFO) && (lcd.ucPageCurr < PAGE_ID_INFO + 0x10))
  {
    lcd.disp_main();
  }
  else if (lcd.ucPageCurr == PAGE_ID_WEB_CFG)
  {
    // Execute END Web server sequence here.
    wifi.bStartWebConfig = false;
    // mesh_network.bInit = false; // re-init

    // WiFi.softAPdisconnect(true);
    // WiFi.enableAP(0);

    lcd.disp_main();
  }
  else if (lcd.ucPageCurr == PAGE_ID_SETUP)
  {
    if (lcd.list_index == 0)
      lcd.disp_setup_resetTeach();
    else if (lcd.list_index == 1)
      lcd.disp_setup_resetOutDef();
    else if (lcd.list_index == 2)
      lcd.disp_setup_resetFacDefault();
    else if (lcd.list_index == 3)
      lcd.disp_setup_rotateDisp();
    else if (lcd.list_index == 4)
      lcd.disp_setup_iotMacQr();
    else if (lcd.list_index == 5)
    {
      lcd.list_index = 0;
      lcd.disp_setup_setDlySen0();
    }
    else if (lcd.list_index == 6)
    {
      lcd.list_index = 0;
      lcd.disp_setup_ignore_sen0_teaseq();
    }
    else if (lcd.list_index == 7)
    {
      lcd.list_index = 0;
      lcd.disp_setup_inOut_mode();
    }
    else if (lcd.list_index == 8)
    {
      lcd.list_index = 0;
      lcd.disp_setup_dir_andon_mc();
    }
    else if (lcd.list_index == 9)
    {
      reset_mc_time();
      e2p_data.commit_flag = true;

      debug.msg_debug.concat(" --/ rst_mctime_menu_setup");
      debug.send_debug = true;

      lcd.disp_main();
    }
    else
      lcd.disp_main();
  }
  else if (lcd.ucPageCurr == PAGE_ID_NFC)
  {
    if (lcd.list_index == 0)
    {
      lcd.list_index = 0;
      lcd.disp_nfc_copy_output();
    }
    else if (lcd.list_index == 1)
    {
      nfc.ucMode = RFID_MOD_WRITE;
      nfc.ucWriteType = RFID_TAG_TYPE_OUTPUT;

      nfc.ucIncDecType = NFC_INCDEC_DECREASE;

      lcd.auto_exit_cnt = 0;
      lcd.disp_nfc_tag_write();
    }
    else if (lcd.list_index == 2)
    {
      nfc.ucMode = RFID_MOD_WRITE;
      nfc.ucWriteType = RFID_TAG_TYPE_NETWORK;

      lcd.auto_exit_cnt = 0;
      lcd.disp_nfc_tag_write();
    }
    else if (lcd.list_index == 3)
    {
      nfc.ucMode = RFID_MOD_WRITE;
      nfc.ucWriteType = RFID_TAG_TYPE_EARSE;

      lcd.auto_exit_cnt = 0;
      lcd.disp_nfc_tag_write();
    }
    else
      lcd.disp_main();
  }
  else if (lcd.ucPageCurr == PAGE_ID_NFC_COPY_OUTPUT)
  {
    if (lcd.list_index == 0)
    {
      nfc.ucMode = RFID_MOD_WRITE;
      nfc.ucWriteType = RFID_TAG_TYPE_OUTPUT;

      nfc.ucIncDecType = NFC_INCDEC_INCREASE;
      nfc.ucInDecScale = 5;

      lcd.auto_exit_cnt = 0;
      lcd.disp_nfc_tag_write();
    }
    else if (lcd.list_index == 1)
    {
      nfc.ucMode = RFID_MOD_WRITE;
      nfc.ucWriteType = RFID_TAG_TYPE_OUTPUT;

      nfc.ucIncDecType = NFC_INCDEC_INCREASE;
      nfc.ucInDecScale = 1;

      lcd.auto_exit_cnt = 0;
      lcd.disp_nfc_tag_write();
    }
    else
      lcd.disp_main();
  }
  else if (lcd.ucPageCurr == PAGE_ID_TEACH)
  {
    teaseq_endTeachMode();
    lcd.disp_main();
  }
  else if (lcd.ucPageCurr == PAGE_ID_SETUP_DLY_SEN0)
  {
    if (lcd.list_index == 0)
    {
      tea.delay_sen0 = tea.delay_sen0 + Fun_teaSeq::INTERVAL_DLY_SEN0;
      if (tea.delay_sen0 > Fun_teaSeq::TEA_MAX_DLY_SEN0) // more than 2000ms
        tea.delay_sen0 = Fun_teaSeq::TEA_MAX_DLY_SEN0;

      lcd.disp_setup_setDlySen0();
    }
    else if (lcd.list_index == 1)
    {
      if (tea.delay_sen0 >= Fun_teaSeq::INTERVAL_DLY_SEN0)
        tea.delay_sen0 = tea.delay_sen0 - Fun_teaSeq::INTERVAL_DLY_SEN0;
      if (tea.delay_sen0 < Fun_teaSeq::TEA_DEF_DLY_SEN0) // default is 10ms
        tea.delay_sen0 = Fun_teaSeq::TEA_DEF_DLY_SEN0;

      lcd.disp_setup_setDlySen0();
    }
    else
    {
      e2p_info.write_16(E2PE_ADR_TEA_DLY_SEN0, tea.delay_sen0);
      e2p_info.commit_flag = true;

      lcd.disp_main();
    }
  }
  else if (lcd.ucPageCurr == PAGE_ID_SETUP_INOUT_IOT_MODE)
  {
    if (lcd.list_index == 0)
    {
      lcd.list_index = 0;
      // enable
      oper.in_out_iot_mode_enable = true;
      oper.in_out_input_mode = true;
      e2p_info.write(E2PE_ADR_INOUT_MODE, 1);
      e2p_info.commit_flag = true;

      lcd.disp_show_text("Enable", Open_Sans_Condensed_Bold_40);
    }
    else if (lcd.list_index == 1)
    {
      lcd.list_index = 0;
      // disable
      oper.in_out_iot_mode_enable = false;
      e2p_info.write(E2PE_ADR_INOUT_MODE, 0);
      e2p_info.commit_flag = true;

      lcd.disp_show_text("Disable", Open_Sans_Condensed_Bold_40);
    }
    else
      lcd.disp_main();
  }
  else if (lcd.ucPageCurr == PAGE_ID_SETUP_IGNORE_SEN0_TEASEQ)
  {
    if (lcd.list_index == 0)
    {
      // enable
      tea.ignore_sen0_teaseq = true;
      e2p_info.write(E2PE_ADR_IGNORE_SEN0_TEA, 1);
      e2p_info.commit_flag = true;

      lcd.disp_show_text("Enable", Open_Sans_Condensed_Bold_40);
    }
    else if (lcd.list_index == 1)
    {
      // disable
      tea.ignore_sen0_teaseq = false;
      e2p_info.write(E2PE_ADR_IGNORE_SEN0_TEA, 0);
      e2p_info.commit_flag = true;

      lcd.disp_show_text("Disable", Open_Sans_Condensed_Bold_40);
    }
    else
      lcd.disp_main();
  }
  else if (lcd.ucPageCurr == PAGE_ID_SETUP_DIR_AND_MC)
  {
    if (lcd.list_index == 0)
    {
      // enable
      oper.dir_andon_mc_mode = true;
      e2p_info.write(E2PE_ADR_DIR_ANDON_MC, 1);
      e2p_info.commit_flag = true;

      lcd.disp_show_text("Enable", Open_Sans_Condensed_Bold_40);
    }
    else if (lcd.list_index == 1)
    {
      // disable
      oper.dir_andon_mc_mode = false;
      e2p_info.write(E2PE_ADR_DIR_ANDON_MC, 0);
      e2p_info.commit_flag = true;

      lcd.disp_show_text("Disable", Open_Sans_Condensed_Bold_40);
    }
    else
      lcd.disp_main();
  }
}
//-----------------------------------------------------------
void UiLcd::btn_long_2()
{
  if (lcd.ucPageCurr == PAGE_ID_MENU)
  {
    teaseq_startTeachMode();
    tea.getListTeaSeq();

    lcd.disp_teaching();
  }
}
//-----------------------------------------------------------
void UiLcd::btn_long_3()
{
}

//************************************************************************************************
void UiLcd::disp_main()
{
  lcd.ucPageCurr = PAGE_ID_MAIN;

  oled.setFont(DejaVu_Sans_11);
  oled.clear();

  char buffer[20];

  //////////////////////////////////
  // wifi
  oled.fillTriangle(0, 3, 7, 3, 3, 9);
  if (wifi.ucErr == WIFI_NO_ERR)
  {
    oled.drawProgressBar(12, 2, 27, 8, wifi.rssi_percent);
  }
  else
  {
    if (wifi.ucErr == WIFI_ERR_AP)
      oled.drawString(15, 2, "AP");
    else if (wifi.ucErr == WIFI_ERR_IP)
      oled.drawString(15, 2, "IP");
    else if (wifi.ucErr == WIFI_ERR_RSSI)
      oled.drawString(15, 2, "RSSI");
    else if (wifi.ucErr == WIFI_ERR_MQTT)
      oled.drawString(15, 2, "SV");
  }

  // process_id
  String process_id = String("---");
  if (oper.len_plan)
    process_id = oper.process_id;
  oled.drawString(60, 0, process_id);

  // time
  ti.getSysTime();
  sprintf(buffer, "%02d:%02d", ti.sysTime.tm_hour, ti.sysTime.tm_min);
  oled.drawString(93, 0, buffer);

  // line - position
  String line = oper.line;
  if (line.length() > 3)
    line = line.substring(oper.line.length() - 3);
  sprintf(buffer, "%s:%03d", line.c_str(), oper.position);
  // String location = line + String(" : ") + String(oper.position);
  oled.drawString(78, 16, buffer);

  if (!oper.in_out_iot_mode_enable)
  {
    /////////////////////////
    // target
    oled.drawString(0, 16, "T: ");
    sprintf(buffer, "%05d", oper.target);
    oled.drawString(20, 16, buffer);

    /////////////////////////
    // defect
    oled.drawString(78, 35, "Defect");
    sprintf(buffer, "%04d", oper.defect);
    oled.drawString(78, 52, buffer);

    /////////////////////////
    // output
    oled.setFont(Open_Sans_Condensed_Bold_40);
    sprintf(buffer, "%04d", oper.output);
    oled.drawString(0, 21, buffer);
    /////////////////////////
  }
  else
  {
    /////////////////////////
    // in/out iot mode text
    oled.setFont(Open_Sans_Condensed_Bold_40);
    if (oper.in_out_input_mode)
      sprintf(buffer, "  INPUT");
    else
      sprintf(buffer, "OUTPUT");

    oled.drawString(0, 21, buffer);
  }

  oled.display();
}
//************************************************************************************************
void UiLcd::disp_menu()
{
  this->ucPageCurr = PAGE_ID_MENU;
  this->auto_exit_cnt = 0;

  String list[8];
  list[0] = "ANDON";
  list[1] = "Change process";
  list[2] = "Information";
  list[3] = "WEB cofig";
  list[4] = "Setup";
  list[5] = "NFC";
  list[6] = "Debug";
  list[7] = "Exit";

  oled.setFont(DejaVu_Sans_11);
  oled.clear();

  oled.drawString(0, 0, "MENU");
  oled.drawString(65, 0, Iot.sVer);
  oled.drawLine(0, 14, 127, 14);

  oled.fillCircle(8, 27, 2);
  oled.drawString(15, 20, list[this->list_index % 8]);
  oled.drawString(15, 36, list[(this->list_index + 1) % 8]);
  oled.drawString(15, 52, list[(this->list_index + 2) % 8]);

  oled.display();
}

//************************************************************************************************
void UiLcd::disp_teaching()
{
  this->ucPageCurr = PAGE_ID_TEACH;
  oled.setFont(DejaVu_Sans_11);

  oled.clear();
  oled.drawString(1, 8, "Teaching sequence");

  // draw signal
  if (tea.ucListCnt == 0)
    oled.drawString(10, 30, "Waiting signal...");
  else
  {
    uint8_t i, ucX, ucY;
    char buffer[6];
    for (i = 0; i < tea.ucListCnt; i++)
    {
      if (i < 5)
      {
        ucX = 1;
        ucY = 20 + (10 * i);
      }
      else
      {
        ucX = 68;
        ucY = 20 + (10 * (i - 5));
      }

      if (tea.ucListName[i] == TEA_SEQ_DLY_ID)
        oled.drawString(ucX, ucY, "DLY");
      else if (tea.ucListName[i] == TEA_SEQ_SEN0_ID)
        oled.drawString(ucX, ucY, "SEN0");
      else if (tea.ucListName[i] == TEA_SEQ_SEN1_ID)
        oled.drawString(ucX, ucY, "SEN1");
      else if (tea.ucListName[i] == TEA_SEQ_SOL0_ID)
        oled.drawString(ucX, ucY, "SOL0");
      else
        oled.drawString(ucX, ucY, "XXX");

      sprintf(buffer, "%04d", tea.usListValue[i]);
      oled.drawString(ucX + 32, ucY, buffer);
    }
  }

  oled.display();
}
//************************************************************************************************
void UiLcd::disp_andon()
{

  this->ucPageCurr = PAGE_ID_ANDON;

  String list[5];
  list[0] = "MACHINE";
  list[1] = "PERSONAL";
  list[2] = "MATERIAL";
  list[3] = "ETC";
  list[4] = "Exit";

  oled.setFont(DejaVu_Sans_11);

  oled.clear();

  oled.drawString(0, 0, "ANDON");
  oled.drawLine(0, 14, 127, 14);

  oled.fillCircle(8, 27, 2);
  oled.drawString(15, 20, list[this->list_index % 5]);
  oled.drawString(15, 36, list[(this->list_index + 1) % 5]);
  oled.drawString(15, 52, list[(this->list_index + 2) % 5]);

  oled.display();
}
//-----------------------------------------------------------
void UiLcd::disp_andon_machine()
{

  this->ucPageCurr = PAGE_ID_ANDON_MC;
  oled.setFont(DejaVu_Sans_11);
  oled.clear();

  oled.drawString(0, 0, "ANDON");
  oled.drawLine(0, 14, 127, 14);
  oled.setFont(Open_Sans_Condensed_Bold_40);
  oled.drawString(5, 17, "M/C");

  oled.display();
}
//-----------------------------------------------------------
void UiLcd::disp_andon_personal()
{

  this->ucPageCurr = PAGE_ID_ANDON_PER;
  oled.setFont(DejaVu_Sans_11);
  oled.clear();

  oled.drawString(0, 0, "ANDON");
  oled.drawLine(0, 14, 127, 14);
  oled.setFont(Open_Sans_Condensed_Bold_40);
  oled.drawString(5, 17, "PER.");

  oled.display();
}
//-----------------------------------------------------------
void UiLcd::disp_andon_material()
{

  this->ucPageCurr = PAGE_ID_ANDON_MAT;
  oled.setFont(DejaVu_Sans_11);
  oled.clear();

  oled.drawString(0, 0, "ANDON");
  oled.drawLine(0, 14, 127, 14);
  oled.setFont(Open_Sans_Condensed_Bold_40);
  oled.drawString(5, 17, "MAT.");

  oled.display();
}
//-----------------------------------------------------------
void UiLcd::disp_andon_etc()
{

  this->ucPageCurr = PAGE_ID_ANDON_ETC;
  oled.setFont(DejaVu_Sans_11);
  oled.clear();

  oled.drawString(0, 0, "ANDON");
  oled.drawLine(0, 14, 127, 14);
  oled.setFont(Open_Sans_Condensed_Bold_40);
  oled.drawString(5, 17, "ETC");

  oled.display();
}
//************************************************************************************************
void UiLcd::disp_andon_sub()
{
  this->ucPageCurr = PAGE_ID_ANDON_SUB;

  uint16_t usModeAndon = MOD_BRK_MACHINE;
  usModeAndon = usModeAndon << (list_index * 4); // 2^4 =16 : 1 byte hexa

  e2p_info.write_16(E2PE_ADR_BREAK_MODE, usModeAndon);
  e2p_info.commit_flag = true;

  oper.bAndonMode = true;
  oper.bSendAndon = true;

  // debug
  debug.msg_debug.concat(" --/ andon mode: ");
  debug.msg_debug.concat(String(usModeAndon, 16));
  debug.send_debug = true;

  String list[LEN_BREAK_REASON];
  list[0] = "M/C";
  list[1] = "PER.";
  list[2] = "MAT.";
  list[3] = "ETC";

  oled.setFont(DejaVu_Sans_11);
  oled.clear();

  oled.drawString(0, 0, "ANDON");
  oled.drawLine(0, 14, 127, 14);
  oled.setFont(Open_Sans_Condensed_Bold_40);
  oled.drawString(5, 17, list[list_index]);

  oled.display();

  // active 2LED RED color
  Led.ucActive = 1;
  Led.ucColor = uint8_t(LED_1_RED + (LED_1_RED << 3)); // Red
  Led.ulTimeTurnOn = 0;                                // active forever
}
//************************************************************************************************
void UiLcd::disp_changeProc()
{
  if (!oper.len_plan)
    this->disp_main();

  this->ucPageCurr = PAGE_ID_CHG_PROC;
  this->auto_exit_cnt = 0;

  oper.current_idx_plan = (oper.current_idx_plan + 1) % oper.len_plan;
  e2p_info.write(E2PE_ADR_INDX_PLAN, oper.current_idx_plan);
  oper.update_product_current_process_id();
  oper.update_config_current_process_id();
  update_runTeaSeqWithCurrentIdxPlan();
  teqseq_restartRunMode();
  e2p_info.commit_flag = true;

  oled.setFont(DejaVu_Sans_11);
  oled.clear();

  oled.drawString(0, 0, "Change process");
  oled.drawLine(0, 14, 127, 14);
  oled.setFont(Open_Sans_Condensed_Bold_40);
  oled.drawString(5, 17, oper.process_id);

  oled.display();
}

//************************************************************************************************
void UiLcd::disp_info()
{
  this->ucPageCurr = PAGE_ID_INFO;
  this->auto_exit_cnt = 0;

  String list[4];
  list[0] = "NETWORK";
  list[1] = "MACHINE";
  list[2] = "PLAN";
  list[3] = "Exit";

  oled.setFont(DejaVu_Sans_11);
  oled.clear();

  oled.drawString(0, 0, "Information");
  oled.drawLine(0, 14, 127, 14);

  oled.fillCircle(8, 27, 2);
  oled.drawString(15, 20, list[this->list_index % 4]);
  oled.drawString(15, 36, list[(this->list_index + 1) % 4]);
  oled.drawString(15, 52, list[(this->list_index + 2) % 4]);

  oled.display();
}

//-----------------------------------------------------------
void UiLcd::disp_info_network()
{
  this->ucPageCurr = PAGE_ID_INFO_NET;

  String list[PAGE_LEN_ROW_NETWORK];

  list[0] = "RSSI : " + String(wifi.lRssi) + String("dB");
  list[1] = "MAC : ";
  list[2] = "  " + wifi.sMac;
  list[3] = "IP : " + wifi.sIP;
  list[4] = "SSID : " + wifi.sSsid;
  list[5] = "PW : " + wifi.sPassword;
  list[6] = "SVR : " + mqtt.sSvrAddr;
  list[7] = "PORT : " + String(mqtt.usSvrPort);

  oled.setFont(DejaVu_Sans_11);
  oled.clear();

  oled.drawString(0, 0, "Info-Network");
  oled.drawLine(0, 14, 127, 14);

  oled.drawString(0, 20, list[this->list_index % PAGE_LEN_ROW_NETWORK]);
  oled.drawString(0, 36, list[(this->list_index + 1) % PAGE_LEN_ROW_NETWORK]);
  oled.drawString(0, 52, list[(this->list_index + 2) % PAGE_LEN_ROW_NETWORK]);

  oled.display();
}

//-----------------------------------------------------------
void UiLcd::disp_info_machine()
{
  calDapot();

  this->ucPageCurr = PAGE_ID_INFO_MC;

  String list[PAGE_LEN_ROW_MACHINE];
  list[0] = "Power - " + getClock(oper.power_ontime);
  list[1] = "Active - " + getClock((oper.handling_time + oper.motor_runtime) / 1000);
  list[2] = "Motor - " + getClock(oper.motor_runtime / 1000);
  list[3] = "ID - " + oper.machine_id;

  oled.setFont(DejaVu_Sans_11);
  oled.clear();

  oled.drawString(0, 0, "Info-Machine");
  oled.drawLine(0, 14, 127, 14);

  oled.drawString(0, 20, list[this->list_index % PAGE_LEN_ROW_MACHINE]);
  oled.drawString(0, 36, list[(this->list_index + 1) % PAGE_LEN_ROW_MACHINE]);
  oled.drawString(0, 52, list[(this->list_index + 2) % PAGE_LEN_ROW_MACHINE]);

  oled.display();
}

//-----------------------------------------------------------
void UiLcd::disp_info_plan()
{
  this->ucPageCurr = PAGE_ID_INFO_PLAN;

  String list[PAGE_LEN_ROW_PLAN];
  list[0] = "FAC : " + oper.factory;
  list[1] = "LIN : " + oper.line;
  list[2] = "POS : " + String(oper.position);
  list[3] = "STY : " + oper.style;
  list[4] = "PRO : " + oper.process;
  list[5] = "PID : " + oper.process_id;
  list[6] = "WID : " + oper.worker_id;
  list[7] = "UID : " + nfc.sUidBlank;

  oled.setFont(DejaVu_Sans_11);
  oled.clear();

  oled.drawString(0, 0, "Info-Plan");
  oled.drawLine(0, 14, 127, 14);

  oled.drawString(0, 20, list[this->list_index % PAGE_LEN_ROW_PLAN]);
  oled.drawString(0, 36, list[(this->list_index + 1) % PAGE_LEN_ROW_PLAN]);
  oled.drawString(0, 52, list[(this->list_index + 2) % PAGE_LEN_ROW_PLAN]);

  oled.display();
}

//************************************************************************************************
void UiLcd::disp_webConfig()
{

  this->ucPageCurr = PAGE_ID_WEB_CFG;
  // Execute START Web server sequence here.
  if (!wifi.bStartWebConfig)
  {
    wifi.bStartWebConfig = true;
    wifi.enable_webConfig();
  }

  oled.setFont(DejaVu_Sans_11);
  oled.clear();

  oled.drawString(0, 0, "Web config");
  oled.drawLine(0, 14, 127, 14);
  oled.setFont(Open_Sans_Condensed_Bold_40);
  oled.drawString(5, 17, "Web cfg");

  oled.display();
}

//************************************************************************************************
void UiLcd::disp_setup()
{
  this->ucPageCurr = PAGE_ID_SETUP;
  this->auto_exit_cnt = 0;

  String list[LCD_SETUP_ROW_COUNT];
  list[0] = "Reset Teaching";
  list[1] = "Reset OUT/DEF";
  list[2] = "Reset factory def.";
  list[3] = "Rotate display";
  list[4] = "IoT MAC QR";
  list[5] = "Set delay Sen0";
  list[6] = "Ignore Sen0 teaseq.";
  list[7] = "In/Out IoT mode";
  list[8] = "Dir. ANDON M/C";
  list[9] = "Reset mc runtime";
  list[10] = "Exit";

  oled.setFont(DejaVu_Sans_11);
  oled.clear();

  oled.drawString(0, 0, "Setup");
  oled.drawLine(0, 14, 127, 14);

  oled.fillCircle(8, 27, 2);
  oled.drawString(15, 20, list[this->list_index % LCD_SETUP_ROW_COUNT]);
  oled.drawString(15, 36, list[(this->list_index + 1) % LCD_SETUP_ROW_COUNT]);
  oled.drawString(15, 52, list[(this->list_index + 2) % LCD_SETUP_ROW_COUNT]);

  oled.display();
}

//-----------------------------------------------------------
void UiLcd::disp_setup_resetTeach()
{
  this->ucPageCurr = PAGE_ID_SETUP_RST_TCH;
  this->auto_exit_cnt = 0;

  rst_teachingSequenceRun();
  teqseq_restartRunMode();
  tea.bTeaSeqChanged = true;

  oled.setFont(DejaVu_Sans_11);
  oled.clear();

  oled.drawString(0, 0, "Setup-Reset teach");
  oled.drawLine(0, 14, 127, 14);

  oled.setFont(Open_Sans_Condensed_Bold_40);
  oled.drawString(5, 17, "Reset...");

  oled.display();

  debug.msg_debug.concat(" --/ disp_setup_resetTeach");
  debug.send_debug = true;
}
//-----------------------------------------------------------
void UiLcd::disp_setup_resetOutDef()
{

  this->ucPageCurr = PAGE_ID_SETUP_RST_OUTDEF;
  this->auto_exit_cnt = 0;

  // reset ouput, defect
  oper.reset_output();
  oper.reset_defect();

  oled.setFont(DejaVu_Sans_11);
  oled.clear();

  oled.drawString(0, 0, "Setup-Reset out/def");
  oled.drawLine(0, 14, 127, 14);

  oled.setFont(Open_Sans_Condensed_Bold_40);
  oled.drawString(5, 17, "Reset...");

  oled.display();

  debug.msg_debug.concat(" --/ disp_setup_resetOutDef");
  debug.send_debug = true;
}
//-----------------------------------------------------------
void UiLcd::disp_setup_resetFacDefault()
{
  this->ucPageCurr = PAGE_ID_SETUP_RST_FAC;
  this->auto_exit_cnt = 0;

  E2P_ResetE2p_info();
  E2P_ResetE2p_data();
  E2P_ResetE2p_network();
  E2P_UpdateE2pVal();

  oled.setFont(DejaVu_Sans_11);
  oled.clear();

  oled.drawString(0, 0, "Setup-Reset factory");
  oled.drawLine(0, 14, 127, 14);

  oled.setFont(Open_Sans_Condensed_Bold_40);
  oled.drawString(5, 17, "Reset...");

  oled.display();

  debug.msg_debug.concat(" --/ disp_setup_resetFacDefault");
  debug.send_debug = true;
}
//-----------------------------------------------------------
void UiLcd::disp_setup_rotateDisp()
{
  this->ucPageCurr = PAGE_ID_SETUP_ROT_DISP;
  this->auto_exit_cnt = 0;

  oled.setFont(DejaVu_Sans_11);
  oled.clear();

  oled.drawString(0, 0, "Setup-Rotate disp");
  oled.drawLine(0, 14, 127, 14);

  oled.setFont(Open_Sans_Condensed_Bold_40);
  oled.drawString(5, 17, "Rotating...");

  oled.display();
}
//-----------------------------------------------------------
void UiLcd::disp_setup_iotMacQr()
{
  this->ucPageCurr = PAGE_ID_SETUP_MAC_QR;

  QRCode qrcode;
  uint8_t qrcodeData[qrcode_getBufferSize(QR_VERSION)], offset;
  qrcode_initText(&qrcode, qrcodeData, QR_VERSION, ECC_QUARTILE, wifi.sMac.c_str());

  offset = (QR_CANVAS_SIZE - (qrcode.size * QR_SCALE)) / 2;

  oled.clear();
  oled.invertDisplay();

  for (uint8_t y = 0; y < qrcode.size; y++)
  {
    for (uint8_t x = 0; x < qrcode.size; x++)
    {
      if (qrcode_getModule(&qrcode, x, y))
      {
        for (uint8_t i = 0; i < QR_SCALE; i++)
        {
          for (uint8_t j = 0; j < QR_SCALE; j++)
          {
            oled.setPixel((x * QR_SCALE + j) + offset + 32, (y * QR_SCALE + i) + offset);
          }
        }
      }
    }
  }

  oled.display();
}
//-----------------------------------------------------------
void UiLcd::disp_setup_setDlySen0()
{
  this->ucPageCurr = PAGE_ID_SETUP_DLY_SEN0;

  String list[3];
  list[0] = "+";
  list[1] = "-";
  list[2] = "Save";

  oled.setFont(DejaVu_Sans_11);
  oled.clear();

  oled.drawString(0, 0, "Set delay Sen0");
  oled.drawLine(0, 14, 127, 14);

  oled.fillCircle(8, 27, 2);
  oled.drawString(15, 20, list[this->list_index % 3]);
  oled.drawString(15, 36, list[(this->list_index + 1) % 3]);
  oled.drawString(15, 52, list[(this->list_index + 2) % 3]);

  char buffer[10];
  oled.setFont(Open_Sans_Condensed_Bold_40);
  sprintf(buffer, "%04d", tea.delay_sen0);
  oled.drawString(44, 10, buffer);

  oled.display();
}
//-----------------------------------------------------------
void UiLcd::disp_setup_ignore_sen0_teaseq()
{
  this->ucPageCurr = PAGE_ID_SETUP_IGNORE_SEN0_TEASEQ;

  String list[3];
  list[0] = "Enable";
  list[1] = "Disable";
  list[2] = "Exit";

  oled.setFont(DejaVu_Sans_11);
  oled.clear();

  oled.drawString(0, 0, "Ignore Sen0 teaseq.");
  oled.drawLine(0, 14, 127, 14);

  oled.fillCircle(8, 27, 2);
  oled.drawString(15, 20, list[this->list_index % 3]);
  oled.drawString(15, 36, list[(this->list_index + 1) % 3]);
  oled.drawString(15, 52, list[(this->list_index + 2) % 3]);

  oled.display();
}
//-----------------------------------------------------------
void UiLcd::disp_setup_inOut_mode()
{
  this->ucPageCurr = PAGE_ID_SETUP_INOUT_IOT_MODE;

  String list[3];
  list[0] = "Enable";
  list[1] = "Disable";
  list[2] = "Exit";

  oled.setFont(DejaVu_Sans_11);
  oled.clear();

  oled.drawString(0, 0, "In/Out IoT mode");
  oled.drawLine(0, 14, 127, 14);

  oled.fillCircle(8, 27, 2);
  oled.drawString(15, 20, list[this->list_index % 3]);
  oled.drawString(15, 36, list[(this->list_index + 1) % 3]);
  oled.drawString(15, 52, list[(this->list_index + 2) % 3]);

  oled.display();
}
//-----------------------------------------------------------
void UiLcd::disp_setup_dir_andon_mc()
{
  this->ucPageCurr = PAGE_ID_SETUP_DIR_AND_MC;

  String list[3];
  list[0] = "Enable";
  list[1] = "Disable";
  list[2] = "Exit";

  oled.setFont(DejaVu_Sans_11);
  oled.clear();

  oled.drawString(0, 0, "Dir. ANDON MC");
  oled.drawLine(0, 14, 127, 14);

  oled.fillCircle(8, 27, 2);
  oled.drawString(15, 20, list[this->list_index % 3]);
  oled.drawString(15, 36, list[(this->list_index + 1) % 3]);
  oled.drawString(15, 52, list[(this->list_index + 2) % 3]);

  oled.display();
}
//************************************************************************************************
void UiLcd::disp_nfc()
{
  this->ucPageCurr = PAGE_ID_NFC;
  this->auto_exit_cnt = 0;

  String list[PAGE_LEN_ROW_NFC];
  list[0] = "Make OUTPUT inc. tag";
  list[1] = "Make OUTPUT dec. tag";
  list[2] = "Make NETWORK tag";
  list[3] = "Erase tag (classic)";
  list[4] = "Exit";

  oled.setFont(DejaVu_Sans_11);
  oled.clear();

  oled.drawString(0, 0, "NFC");
  oled.drawLine(0, 14, 127, 14);

  oled.fillCircle(8, 27, 2);
  oled.drawString(15, 20, list[this->list_index % PAGE_LEN_ROW_NFC]);
  oled.drawString(15, 36, list[(this->list_index + 1) % PAGE_LEN_ROW_NFC]);
  oled.drawString(15, 52, list[(this->list_index + 2) % PAGE_LEN_ROW_NFC]);

  oled.display();
}
//************************************************************************************************
void UiLcd::disp_nfc_copy_output()
{
  this->ucPageCurr = PAGE_ID_NFC_COPY_OUTPUT;

  String list[PAGE_LEN_ROW_COPY_OUTPUT];
  list[0] = "Scale 5";
  list[1] = "Scale 1";
  list[2] = "Exit";

  oled.setFont(DejaVu_Sans_11);
  oled.clear();

  oled.drawString(0, 0, "Icr. Output");
  oled.drawLine(0, 14, 127, 14);

  oled.fillCircle(8, 27, 2);
  oled.drawString(15, 20, list[this->list_index % PAGE_LEN_ROW_COPY_OUTPUT]);
  oled.drawString(15, 36, list[(this->list_index + 1) % PAGE_LEN_ROW_COPY_OUTPUT]);
  oled.drawString(15, 52, list[(this->list_index + 2) % PAGE_LEN_ROW_COPY_OUTPUT]);

  oled.display();
}
//************************************************************************************************
//************************************************************************************************
void UiLcd::disp_nfc_tag_handle()
{
  this->ucPageCurr = PAGE_ID_NFC_TAG_HANLDE;
  this->auto_exit_cnt = 0;

  oled.setFont(DejaVu_Sans_11);
  oled.clear();

  oled.drawString(0, 0, "NFC Handle");
  oled.drawLine(0, 14, 127, 14);

  if (nfc.bHanldeSuccess)
  {
    oled.drawString(15, 36, "SUCCESS");
  }
  else
  {
    oled.drawString(15, 36, "ERROR");
  }

  oled.display();
}
//************************************************************************************************
void UiLcd::disp_nfc_tag_write()
{
  this->ucPageCurr = PAGE_ID_NFC_TAG_WRITE;

  oled.setFont(DejaVu_Sans_11);
  oled.clear();

  oled.drawString(0, 0, "NFC Write");
  oled.drawLine(0, 14, 127, 14);

  oled.drawString(15, 36, "Waiting nfc (" + String(NFC_TIME_WAIT_WRITE - auto_exit_cnt) + ") ");

  oled.display();
}
//************************************************************************************************
void nfc_tag_nfc_network_sub()
{
  int len = 4;
  String list[len];
  list[0] = "SSID: " + wifi.sSsid;
  list[1] = "PSK: " + wifi.sPassword;
  list[2] = "SVR: " + mqtt.sSvrAddr;
  list[3] = "PORT: " + String(mqtt.usSvrPort);

  for (int i = 0; i < len; i++)
  {
    oled.drawString(0, 4 + i * 16, list[i]);
  }
}
//************************************************************************************************
//************************************************************************************************
//************************************************************************************************
//************************************************************************************************
void UiLcd::disp_nfc_tag_read()
{
  this->ucPageCurr = PAGE_ID_NFC_TAG_READ;
  this->auto_exit_cnt = 0;

  oled.setFont(DejaVu_Sans_11);
  oled.clear();

  if (nfc.ucReadType == NFC_TAG_TYPE_NETWORK)
  {
    nfc_tag_nfc_network_sub();
  }
  else if (nfc.ucReadType == NFC_TAG_TYPE_BLANK)
  {

    if (!oper.in_out_iot_mode_enable)
    {
      oled.drawString(0, 0, "BLANK NFC");
      oled.drawLine(0, 14, 127, 14);

      oled.setFont(ArialMT_Plain_16);
      oled.drawString(15, 30, nfc.sUid);
    }
    else
    {
      if (oper.in_out_input_mode)
        oled.drawString(0, 0, "INPUT IOT MODE");
      else
        oled.drawString(0, 0, "OUTPUT IOT MODE");

      oled.drawLine(0, 14, 127, 14);

      oled.setFont(ArialMT_Plain_16);
      oled.drawString(15, 30, nfc.sUid);
    }
  }
  else if (nfc.ucReadType == NFC_TAG_TYPE_OUTPUT)
  {
    oled.drawString(0, 0, "Inc/Dec");
    oled.drawLine(0, 14, 127, 14);

    oled.drawString(15, 36, "SUCCESS");
  }

  oled.display();
}
//************************************************************************************************
void UiLcd::disp_show_text(const char *text, const uint8_t *font)
{
  this->ucPageCurr = PAGE_ID_SHOW_TEXT;
  this->auto_exit_cnt = 0;

  oled.clear();

  oled.setFont(font);
  oled.drawString(5, 17, text);

  oled.display();
}
//************************************************************************************************
//************************************************************************************************
//************************************************************************************************
//************************************************************************************************
//************************************************************************************************
//************************************************************************************************
//************************************************************************************************
//************************************************************************************************
//************************************************************************************************
//************************************************************************************************
//************************************************************************************************
//************************************************************************************************
void UiLcd::init()
{
  oled.init();

  oled.flipScreenVertically();

  this->disp_main();

  ti.setSysTime(2002, 1, 1, 0, 0, 0);
}

//************************************************************************************************
#define LCD_TIME_REFRESH 1000 // ms

uint32_t check_ti, oneSecCheck;
// uint32_t timeDelta;
uint32_t time_hour, time_min, time_sec;
String stTime;
void UiLcd::loop()
{

  this->uiMillis_now = millis();

  if ((this->uiMillis_now - this->uiMillis_tick) > LCD_TIME_REFRESH)
  {
    this->uiMillis_tick = this->uiMillis_now;

    switch (this->ucPageCurr)
    {
    /////////////////////////////////////////////////////////////////////////////////////
    case PAGE_ID_MAIN:
    {
      this->bRefresh = true;
      if (this->bRefresh)
        this->disp_main();
      break;
    }
    /////////////////////////////////////////////////////////////////////////////////////
    case PAGE_ID_MENU:
    case PAGE_ID_SETUP:
      this->auto_exit_cnt++;
      if (this->auto_exit_cnt > 10)
        this->disp_main();
      break;
    /////////////////////////////////////////////////////////////////////////////////////
    case PAGE_ID_NFC:
      break;
    case PAGE_ID_NFC_TAG_HANLDE:
    {
      this->auto_exit_cnt++;
      if (this->auto_exit_cnt >= NFC_TIME_WAIT_HANLDE)
      {
        if (nfc.bHanldeSuccess)
        {
          lcd.auto_exit_cnt = 0;
          lcd.disp_nfc_tag_write();
        }
        else
          this->disp_main();
      }
      break;
    }
    case PAGE_ID_NFC_TAG_WRITE:
    {
      this->auto_exit_cnt++;
      if (this->auto_exit_cnt >= NFC_TIME_WAIT_WRITE)
      {
        nfc.ucMode = RFID_MOD_READ;
        this->disp_main();
      }
      else
      {
        disp_nfc_tag_write();
      }

      break;
    }
    case PAGE_ID_NFC_TAG_READ:
    {
      this->auto_exit_cnt++;
      if (this->auto_exit_cnt >= NFC_TIME_WAIT_READ)
      {
        if (wifi.bStartWebConfig)
          disp_webConfig();
        else
          this->disp_main();
      }

      break;
    }
    /////////////////////////////////////////////////////////////////////////////////////
    case PAGE_ID_CHG_PROC:
    case PAGE_ID_SETUP_RST_TCH:
    case PAGE_ID_SETUP_RST_OUTDEF:
    case PAGE_ID_SETUP_RST_FAC:
    case PAGE_ID_SETUP_ROT_DISP:
    case PAGE_ID_SHOW_TEXT:
    {
      this->auto_exit_cnt++;
      if (this->auto_exit_cnt > 2)
        this->disp_main();
      break;
    }
    /////////////////////////////////////////////////////////////////////////////////////
    case PAGE_ID_TEACH:
      if (this->bRefresh)
        this->disp_teaching();
      break;
    }
    this->bRefresh = false;
  }
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

/////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////