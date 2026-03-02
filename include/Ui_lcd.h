

#ifndef UI_LCD_H
#define UI_LCD_H
//************************************************************************************************
// #include "main.h"

#include <stdint.h>

//************************************************************************************************
class Fun_wifi;
//************************************************************************************************
class UiLcd
{
public:
  void init();
  void loop();

  static void btn_short();
  static void btn_long_1();
  static void btn_long_2();
  static void btn_long_3();

  static void disp_main();
  void disp_menu();
  // teaching
  void disp_teaching();
  // andon
  void disp_andon();
  void disp_andon_machine();
  void disp_andon_personal();
  void disp_andon_material();
  void disp_andon_etc();
  void disp_andon_sub();

  void disp_changeProc();
  // info
  void disp_info();
  void disp_info_network();
  void disp_info_machine();
  void disp_info_plan();
  void disp_webConfig();
  // setup
  void disp_setup();
  void disp_setup_resetTeach();
  void disp_setup_resetOutDef();
  void disp_setup_resetFacDefault();
  void disp_setup_rotateDisp();
  void disp_setup_iotMacQr();
  void disp_setup_setDlySen0();
  void disp_setup_ignore_sen0_teaseq();
  void disp_setup_inOut_mode();
  void disp_setup_dir_andon_mc();
  // nfc
  void disp_nfc();
  void disp_nfc_copy_output();
  void disp_nfc_tag_handle();
  void disp_nfc_tag_write();
  void disp_nfc_tag_read();
  // show "enable/disable"
  void disp_show_text(const char *text, const uint8_t *font);

  uint8_t ucPageCurr;
  uint8_t list_index;

  bool bRefresh;

  uint32_t auto_exit_cnt;

private:
  uint32_t uiMillis_now, uiMillis_tick;
};

//************************************************************************************************
extern UiLcd lcd;

//************************************************************************************************
#endif // UI_LCD_H