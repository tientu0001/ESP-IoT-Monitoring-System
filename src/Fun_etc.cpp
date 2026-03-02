

#include "config.h"

#include "Fun_operation.h"
#include "Fun_wifi.h"

#include "Fun_mqtt.h"
#include "Fun_time.h"
#include "Fun_nfc.h"

#include "Fun_e2p.h"

#include "Fun_test.h"

/////////////////////////////////////////////////////////////////////////////////////
#include <Ticker.h>
Ticker Timer_TeaSeq;
Ticker Timer_1;
Ticker Timer_10;
/////////////////////////////////////////////////////////////////////////////////////
uint8_t ucCntUidRs = 0;
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
#define TIME_ON_NFC_HANDLE 2000 // ms
/////////////////////////////////////////////////////////////////////////////////////
#define NOR_OUT_0 1
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
uint8_t cnt_60s;
uint8_t cnt_300s;
uint8_t cnt_600s;
/////////////////////////////////////////////////////////////////////////////////////
void control_led();
void control_e2p();
/////////////////////////////////////////////////////////////////////////////////////
void t_teaSeqCallback();
void t_1sCallback();
void t_10sCallback();
/////////////////////////////////////////////////////////////////////////////////////
void check_and_reset_data_machine();
/////////////////////////////////////////////////////////////////////////////////////
// void IRAM_ATTR timer1_ISR()
// {
//     timer1_write(50000); // 10ms
//     // timer1_write(500000); // 100ms
//     // SignalCtrl(&ma.Sig.Def);
//     teaSeq_detect_signal();
// }
/////////////////////////////////////////////////////////////////////////////////////
void init_task()
{
    Timer_TeaSeq.attach_ms(10, t_teaSeqCallback);
    Timer_1.attach(1, t_1sCallback);
    Timer_10.attach(10, t_10sCallback);
}
/////////////////////////////////////////////////////////////////////////////////////
void loop_task()
{
}
/////////////////////////////////////////////////////////////////////////////////////
void init_etc()
{
    // iot version
    Iot.sHwVer = "00NB";
    Iot.sSwVer = "0112";
    Iot.sVer = Iot.sHwVer + String("-") + Iot.sSwVer;

    Iot.sIotModel = "000B";
    Iot.sIotSupplier = "S096";

// led RGB
#ifndef ESP_ENABLE_DEBUG
    pinMode(NOR_OUT_0, FUNCTION_3);
    pinMode(NOR_OUT_0, OUTPUT);
    write_color(LED_OFF); // led RGB off
    // uint8_t color = (uint8_t)(LED_1_GREEN << 3);
    // write_color(color);
#endif

    // eeprom address
    oper.list_addr_output[0] = E2PE_ADR_DATA_OUTPUT_1;
    oper.list_addr_output[1] = E2PE_ADR_DATA_OUTPUT_2;
    oper.list_addr_output[2] = E2PE_ADR_DATA_OUTPUT_3;
    oper.list_addr_output[3] = E2PE_ADR_DATA_OUTPUT_4;

    // setup RSSI wifi
    wifi.arRssi[0] = -50;
    wifi.arRssi[1] = -65;
    wifi.arRssi[2] = -80;
    wifi.arRssi[3] = -100;

    // topic debug
    debug.topic_debug = String("/debug/") + wifi.sMac;
}
/////////////////////////////////////////////////////////////////////////////////////
void loop_etc()
{
    if (Inf.ucFlagESPRestart)
        ESP.restart();
    control_led(); // control led RGB
    // control_e2p(); // commit e2p
}
/////////////////////////////////////////////////////////////////////////////////////
void write_color(uint8_t ucRGB_Data)
{
#ifndef ESP_ENABLE_DEBUG
    uint8_t ucMask = 0x01;
    for (uint8_t i = 0; i <= 8; i++)
    {
        if (ucRGB_Data & ucMask)
        {
            digitalWrite(NOR_OUT_0, LOW);
            digitalWrite(NOR_OUT_0, HIGH);
            delayMicroseconds(1000);
        }
        else
        {
            digitalWrite(NOR_OUT_0, LOW);
            delayMicroseconds(1000);
            digitalWrite(NOR_OUT_0, HIGH);
            delayMicroseconds(2000);
        }
        ucMask = ucMask << 1;
        if (ucMask == 0x80)
            break; // move bit 1 to bit 8 and stop
    }
    digitalWrite(NOR_OUT_0, LOW);
    delayMicroseconds(1000);
    digitalWrite(NOR_OUT_0, HIGH);
    delayMicroseconds(2000);
#endif
}
/////////////////////////////////////////////////////////////////////////////////////
void active_led_nfc_success()
{
    Led.ucActive = 1;
    Led.ucColor = uint8_t(LED_1_GREEN + (LED_1_GREEN << 3)); // Green
    Led.ulTimeTurnOn = TIME_ON_NFC_HANDLE;
}
/////////////////////////////////////////////////////////////////////////////////////
void active_led_nfc_fail()
{
    Led.ucActive = 1;
    Led.ucColor = uint8_t(LED_1_RED + (LED_1_RED << 3)); // Red
    Led.ulTimeTurnOn = TIME_ON_NFC_HANDLE;
}
/////////////////////////////////////////////////////////////////////////////////////
void control_led()
{
    if (Led.ucActive == 1)
    {
        Led.ucActive = 0;

        write_color(Led.ucColor);

        if (Led.ulTimeTurnOn)
        {
            Led.ucChkTurnOffLedRGB = 1;
            Led.ulMilShowLedRGB = millis();
        }
    }
    else if (Led.ucChkTurnOffLedRGB == 1)
    {
        if (millis() - Led.ulMilShowLedRGB > Led.ulTimeTurnOn)
        {
            Led.ucChkTurnOffLedRGB = 0;

            write_color(LED_OFF);

            // if (oper.bRecRealPlan)
            //     write_color(LED_OFF);
            // else
            // {
            //     uint8_t color = (uint8_t)(LED_1_GREEN << 3);
            //     write_color(color);
            // }
        }
    }
}
/////////////////////////////////////////////////////////////////////////////////////
void control_e2p()
{
    if (e2p_info.commit_flag)
    {
        e2p_info.commit_flag = false;
        if (!e2p_info.commit())
        {
            debug.msg_debug.concat(" --/ e2p_info.commit() false");
            debug.send_debug = true;
        }
        // delay(500);
        return;
    }

    if (e2p_data.commit_flag)
    {
        e2p_data.commit_flag = false;
        if (!e2p_data.commit())
        {
            debug.msg_debug.concat(" --/ e2p_data.commit() false");
            debug.send_debug = true;
        }
        // delay(500);
        return;
    }

    if (e2p_network.commit_flag)
    {
        e2p_network.commit_flag = false;
        if (!e2p_network.commit())
        {
            debug.msg_debug.concat(" --/ e2p_network.commit() false");
            debug.send_debug = true;
        }
        // delay(500);
        return;
    }
}
/////////////////////////////////////////////////////////////////////////////////////
void t_teaSeqCallback()
{
    teaSeq_detect_signal();
}
/////////////////////////////////////////////////////////////////////////////////////
void t_1sCallback()
{
    teaseq_increaseDelayRunMode();

    control_e2p(); // commit e2p
}
/////////////////////////////////////////////////////////////////////////////////////
uint8_t cnt_check_get_data_from_svr = 0;
void t_60sCallback()
{
    oper.bSendMcTime = true; // send data machine time
    save_mc_time();          // save mc time

    e2p_info.commit_flag = true;
    e2p_data.commit_flag = true;

    if (++cnt_check_get_data_from_svr >= 3)
    {
        cnt_check_get_data_from_svr = 0;
        if (WiFi.status() == WL_CONNECTED)
        {
            if (mqtt.check_get_data_from_server())
            {
                if (++mqtt.u8_cnt_restart_mqtt_reason > 10)
                {
                    e2p_info.commit();
                    e2p_data.commit();

                    Inf.ucFlagESPRestart = 1;
                }

                if (mqtt.u8_cnt_restart_mqtt_reason == 2)
                {
                    mqtt.bMqttSubscribe = true;

                    mqtt.mqttSubscribe();

                    debug.msg_debug.concat(" --/ mqtt.mqttSubscribe() again ");
                    debug.send_debug = true;
                }
            }
        }
    }

    if (oper.check_data_machine)
    {
        oper.check_data_machine = false;
        check_and_reset_data_machine();
    }

    //////////////////////////////////
    // hansoll
    // check get nfc uid from e2p
    if (nfc.get_uid_blank_again)
    {
        nfc.get_uid_blank_again = false;

        if (++ucCntUidRs >= 5)
        {
            return;
        }

        get_uid_blank();
        check_uid_blank_problem();
    }
}
/////////////////////////////////////////////////////////////////////////////////////
void t_600sCallback()
{
}
/////////////////////////////////////////////////////////////////////////////////////
void t_300sCallback()
{
    if (mqtt.check_reset_newday() && oper.bGetDatetime)
    {
        resetDataWhenNewDay(true);
    }
}
/////////////////////////////////////////////////////////////////////////////////////
uint8_t ucOnTime;
void t_10sCallback()
{
    if (++cnt_60s >= 6) // 60s
    {
        cnt_60s = 0;
        t_60sCallback();

        if (++cnt_300s >= 5)
        {
            cnt_300s = 0;
            t_300sCallback();
        }

        if (++cnt_600s >= 10)
        {
            cnt_600s = 0;
            t_600sCallback();
        }
    }

    // only hansoll - output check and save every 10seconds
    // e2p_data.commit_flag = true;
}
/////////////////////////////////////////////////////////////////////////////////////
void check_and_reset_data_machine()
{
    if (ti.sysTime.tm_hour <= TIME_HOUR_RST_NEW_DAY)
        return;

    uint32_t delta_sec = (ti.sysTime.tm_hour - TIME_HOUR_RST_NEW_DAY) * 3600;
    delta_sec += ti.sysTime.tm_min * 60;
    delta_sec += ti.sysTime.tm_sec;
    if (oper.power_ontime > delta_sec) // wrong - reset again
    {
        debug.msg_debug.concat(" --/ check_and_reset_data_machine-true: ");
        debug.msg_debug.concat(oper.power_ontime);
        debug.msg_debug.concat("/");
        debug.msg_debug.concat(delta_sec);
        debug.send_debug = true;

        reset_production();
        reset_mc_time();

        debug.msg_debug.concat(" --/ rst_mctime_check_and_reset_data_machine");
        debug.send_debug = true;

        e2p_info.commit_flag = true;
        e2p_data.commit_flag = true;
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