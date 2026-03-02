//************************************************************************************************

#include "Ui_button.h"
#include "Ui_lcd.h"

#include "Fun_operation.h"
#include "Fun_nfc.h"
#include "Fun_wifi.h"

#include "Fun_e2p.h"


#include "Fun_teaSeq.h"
#include "Fun_mqtt.h"
#include "Fun_time.h"

#include "Fun_test.h"

// extern "C" uint32_t _EEPROM_start;
//************************************************************************************************
//************************************************************************************************
//************************************************************************************************
LED_RGB Led;
IoT_Sub Iot;
INFO Inf;
IN_OUT_MODE inOut_mode;
DEBUG_INFO debug;
//************************************************************************************************
// float esp_vcc;
// ADC_MODE(ADC_VCC);
// uint32_t ulVccScan;
// void esp_get_vcc()
// {
//   if (millis() - ulVccScan > 1000)
//   {
//     esp_vcc = ESP.getVcc() / 1000.00f;
//     ulVccScan = millis();
//   }
// }
//************************************************************************************************
void setup()
{
  ESP.wdtDisable();
  ESP.wdtEnable(WDTO_8S);

#ifdef ESP_ENABLE_DEBUG
  Serial.begin(115200);
  delay(1000);
  Serial.println();
#endif

  btn.init();
  btn.regCallback(lcd.btn_short, lcd.btn_long_1, lcd.btn_long_2, lcd.btn_long_3);

  wifi.sMac = String(WiFi.macAddress());

  init_etc();
  init_e2p();
  lcd.init();
  wifi.init();
  nfc.init();
  oper.init();
  tea.init();
  init_task();

  //
  delay(500);
  //
  chkAndonMode();

  // // test
  // test.init();
}
//************************************************************************************************
void loop()
{
  // function
  wifi.loop();
  nfc.loop();
  oper.loop();
  tea.loop();
  loop_etc();

  // ui
  lcd.loop();
  btn.loop();

  // // test
  // test.loop();

  yield();
}
//************************************************************************************************