//************************************************************************************************
#include "config.h"
#include "Fun_operation.h"
#include "Fun_mqtt.h"

#include "Fun_e2p.h"
#include "Fun_time.h"
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
#define INTERVAL_TIME_MOTOR_STOP 500     // ms
#define INTERVAL_SCAN_STS_GENERAL_MC 200 // ms
/////////////////////////////////////////////////////////////////////////////////////
FunOperation oper;
//************************************************************************************************
void FunOperation::init()
{
    oper.handling_time_idx_plan = HANDLING_TIME_DEFAULT * 1000;
}
//************************************************************************************************
void FunOperation::loop()
{
    uint32_t ulMil = millis();

    //     // calculate motor runtime
    //     if (!ucNonSewingMc) // sewing mc
    //     {
    //         if (bMotorRun)
    //         {
    //             // if (ulMil - ulTimeSen0HappnedFinal > INTERVAL_TIME_MOTOR_STOP) // motor stop
    //             if (ulMil - oper.u32_stitch_happend > 1000)
    //             {
    //                 ulCurMotRunMs = ulTimeSen0HappnedFinal - ulTimeSen0HappnedFirst;
    //                 motor_runtime += ulCurMotRunMs;

    //                 bMotorRun = false; // motor stop

    //                 // start time up handling time
    //                 // ulHanTimeTmp = ulMil;
    //                 ulHanTimeTmp = ulTimeSen0HappnedFinal + 1000;
    //                 bCalHandlingTime = true;

    // #ifdef ESP_DEBUG_OPER
    //                 Serial.printf("-------ulCurMotRunMs: %u\r\n", ulCurMotRunMs);
    // #endif
    //             }
    //         }
    //     }
    //     else // general mc
    //     {
    //         if (bMotorRun && (ulMil - ulGenScanOld > INTERVAL_SCAN_STS_GENERAL_MC))
    //         {
    //             if (get_statusSen0() == HIGH) // stop
    //             {
    //                 ulCurMotRunMs = ulMil - ulTimeSen0HappnedFirst;
    //                 motor_runtime += ulCurMotRunMs;

    //                 bMotorRun = false; // motor stop

    //                 // start time up handling time
    //                 ulHanTimeTmp = ulMil;
    //                 bCalHandlingTime = true;

    // #ifdef ESP_DEBUG_OPER
    //                 Serial.printf("-------stop motor_run11111: %u\r\n", ulCurMotRunMs);
    // #endif
    //             }

    //             ulGenScanOld = ulMil;
    //         }
    //     }

    //     // calculate handling time
    //     if (bCalHandlingTime)
    //     {
    //         if (bMotorRun) // if motor run, get this time - ulHanTimeTmp and check
    //         {
    //             if (ulMil > ulHanTimeTmp)
    //             {
    //                 ulCurHanTime = (ulMil - ulHanTimeTmp);
    //                 handling_time += ulCurHanTime;
    //             }
    // #ifdef ESP_DEBUG_OPER
    //             Serial.printf("--incr handle: %u\r\n", ulCurHanTime);
    // #endif

    //             bCalHandlingTime = 0;
    //         }
    //         else // if motor stop, check when at this time - ulHanTimeTmp > time in Plan <= from svr, so add time in Plan
    //         {
    //             if ((ulMil - ulHanTimeTmp) > (handling_time_idx_plan))
    //             {
    //                 ulCurHanTime = handling_time_idx_plan;
    //                 handling_time += ulCurHanTime;
    //                 bCalHandlingTime = 0;

    // #ifdef ESP_DEBUG_OPER
    //                 Serial.printf("--incr handle111: %u\r\n", ulCurHanTime);
    // #endif
    //             }
    //         }
    //     }

    // calculate motor runtime
    if (!ucNonSewingMc) // sewing mc
    {
        if (bMotorRun)
        {
            if (ulMil - ulTimeSen0HappnedFinal > INTERVAL_TIME_MOTOR_STOP) // motor stop
            {
                if (ulTimeSen0HappnedFinal > ulTimeSen0HappnedFirst)
                {
                    ulCurMotRunMs = ulTimeSen0HappnedFinal - ulTimeSen0HappnedFirst;
                    motor_runtime += ulCurMotRunMs;

                    // debug.msg_debug.concat(", M1: ");
                    // debug.msg_debug.concat(ulCurMotRunMs);

#ifdef ESP_DEBUG_OPER
                    Serial.printf("-------ulCurMotRunMs: %u\r\n", ulCurMotRunMs);
#endif
                }

                bMotorRun = false; // motor stop

                // start time up handling time
                ulHanTimeTmp = ulMil;
                // ulHanTimeTmp = ulTimeSen0HappnedFinal + 200;
                bCalHandlingTime = true;
            }
        }
    }
    else // general mc
    {
        if (bMotorRun && (ulMil - ulGenScanOld > INTERVAL_SCAN_STS_GENERAL_MC))
        {
            if (get_statusSen0() == HIGH) // stop
            {
                if (ulMil > ulTimeSen0HappnedFirst)
                {
                    ulCurMotRunMs = ulMil - ulTimeSen0HappnedFirst;
                    motor_runtime += ulCurMotRunMs;

                    // debug.msg_debug.concat(", M2: ");
                    // debug.msg_debug.concat(ulCurMotRunMs);

#ifdef ESP_DEBUG_OPER
                    Serial.printf("-------stop motor_run11111: %u\r\n", ulCurMotRunMs);
#endif
                }

                bMotorRun = false; // motor stop

                // start time up handling time
                ulHanTimeTmp = ulMil;
                bCalHandlingTime = true;
            }

            ulGenScanOld = ulMil;
        }
    }

    // calculate handling time
    if (bCalHandlingTime)
    {
        if (bMotorRun) // if motor run, get this time - ulHanTimeTmp and check
        {
            if (ulMil > ulHanTimeTmp)
            {
                ulCurHanTime = (ulMil - ulHanTimeTmp);
                handling_time += ulCurHanTime;

                // debug.msg_debug.concat(", H1: ");
                // debug.msg_debug.concat(ulCurHanTime);

#ifdef ESP_DEBUG_OPER
                Serial.printf("--incr handle: %u\r\n", ulCurHanTime);
#endif
            }

            bCalHandlingTime = 0;
        }
        else // if motor stop, check when at this time - ulHanTimeTmp > time in Plan <= from svr, so add time in Plan
        {
            if ((ulMil - ulHanTimeTmp) > (handling_time_idx_plan))
            {
                ulCurHanTime = handling_time_idx_plan;
                handling_time += ulCurHanTime;
                bCalHandlingTime = 0;

                // debug.msg_debug.concat(", H2: ");
                // debug.msg_debug.concat(ulCurHanTime);

#ifdef ESP_DEBUG_OPER
                Serial.printf("--incr handle111: %u\r\n", ulCurHanTime);
#endif
            }
        }
    }
}
//************************************************************************************************
void FunOperation::increase_output()
{
    output = output + scale;
    addQueue_product_data(Q_TYPE_OUTPUT, output);

    lcd_refresh();
}
//************************************************************************************************
void FunOperation::increase_defect()
{
    defect = defect + scale;
    addQueue_product_data(Q_TYPE_DEFECT, defect);

    lcd_refresh();
}
//************************************************************************************************
void FunOperation::reset_output()
{
    output = 0;
    addQueue_product_data(Q_TYPE_OUTPUT, output);

    lcd_refresh();
}
//************************************************************************************************
void FunOperation::reset_defect()
{
    defect = 0;
    addQueue_product_data(Q_TYPE_DEFECT, defect);

    lcd_refresh();
}
//************************************************************************************************
void FunOperation::addQueue_product_data(uint8_t ucType, uint32_t ulData)
{
    uint16_t usQfront, usQfrontAddr;

    usQfront = e2p_data.read_16(E2PE_ADR_DATA_Q_FRONT);
    usQfrontAddr = usQfront * E2PE_LEN_Q_DATA;

    ti.getSysTime();

    e2p_data.write(usQfrontAddr + Q_BUF_IDX_YEAR, ti.sysTime.tm_year - 2000);
    e2p_data.write(usQfrontAddr + Q_BUF_IDX_MON, ti.sysTime.tm_mon);
    e2p_data.write(usQfrontAddr + Q_BUF_IDX_DAY, ti.sysTime.tm_mday);
    e2p_data.write(usQfrontAddr + Q_BUF_IDX_HOUR, ti.sysTime.tm_hour);
    e2p_data.write(usQfrontAddr + Q_BUF_IDX_MIN, ti.sysTime.tm_min);
    e2p_data.write(usQfrontAddr + Q_BUF_IDX_SEC, ti.sysTime.tm_sec);
    e2p_data.write(usQfrontAddr + Q_BUF_IDX_TYPE, ucType);

    for (int i = 0; i < LEN_BYTE_PRODUCT_DATA; i++)
        e2p_data.write(usQfrontAddr + Q_BUF_IDX_DATA + i, (ulData >> (8 * (LEN_BYTE_PRODUCT_DATA - 1 - i))));

    e2p_data.write(usQfrontAddr + Q_BUF_IDX_CUR_PLAN, current_idx_plan);

    // Save to main memory
    switch (ucType)
    {
    case Q_TYPE_OUTPUT:
    {
        e2p_data.write_32(list_addr_output[current_idx_plan], ulData);
        break;
    }
    case Q_TYPE_DEFECT:
    {
        e2p_data.write_32(list_addr_output[current_idx_plan] + E2PE_LEN_OUTPUT, ulData);
        break;
    }
    }

    dev_IncQfontIdx();
}
//************************************************************************************************
void FunOperation::delQueue_product_data(uint8_t *ucOutBuff)
{
    uint16_t usQrear, usQrearAddr;

    usQrear = e2p_data.read_16(E2PE_ADR_DATA_Q_REAR);
    usQrear = usQrear % E2PE_LEN_Q_BUFF;
    usQrearAddr = usQrear * E2PE_LEN_Q_DATA;

    for (uint8_t i = 0; i < E2PE_LEN_Q_DATA; i++)
        ucOutBuff[i] = e2p_data.read(usQrearAddr + i);

    // dev_IncQrearIdx(); // increase Q_Rear
}
//************************************************************************************************
void FunOperation::dev_IncQfontIdx()
{
    uint16_t usQfront = (e2p_data.read_16(E2PE_ADR_DATA_Q_FRONT) + 1) % E2PE_LEN_Q_BUFF;
    e2p_data.write_16(E2PE_ADR_DATA_Q_FRONT, usQfront);

    uint16_t usQrear = e2p_data.read_16(E2PE_ADR_DATA_Q_REAR);
    if (usQfront == usQrear)
    {
        usQrear = (usQrear + 1) % E2PE_LEN_Q_BUFF;
        e2p_data.write_16(E2PE_ADR_DATA_Q_REAR, usQrear);
    }
}
//************************************************************************************************
void FunOperation::dev_IncQrearIdx()
{
    uint16_t valQrear = (e2p_data.read_16(E2PE_ADR_DATA_Q_REAR) + 1) % E2PE_LEN_Q_BUFF;
    e2p_data.write_16(E2PE_ADR_DATA_Q_REAR, valQrear);
}
//************************************************************************************************
void FunOperation::update_product_current_process_id()
{
    output = e2p_data.read_32(list_addr_output[current_idx_plan]);
    defect = e2p_data.read_32(list_addr_output[current_idx_plan] + E2PE_LEN_OUTPUT);
    target = e2p_data.read_32(list_addr_output[current_idx_plan] + E2PE_LEN_OUTPUT + E2PE_LEN_DEFECT);

    worker_id = plan_wrk_id[current_idx_plan];
    style = plan_stlye[current_idx_plan];
    process = plan_process[current_idx_plan];
    process_id = plan_process_id[current_idx_plan];
}
//************************************************************************************************
void FunOperation::update_config_current_process_id()
{
    scale = plan_scale[current_idx_plan];
    if (!scale)
        scale = 1;
    e2p_info.write_16(E2PE_ADR_OUT_SCALE, scale);

    handling_time_idx_plan = plan_handling_time[current_idx_plan] * 1000;
    if (!handling_time_idx_plan)
        handling_time_idx_plan = HANDLING_TIME_DEFAULT * 1000;
}
//************************************************************************************************

//************************************************************************************************
/////////////////////////////////////////////////////////////////////////////////////
void reset_production()
{
    // reset ouput, defect, target
    oper.output = 0;
    oper.defect = 0;
    oper.target = 0;

    for (uint8_t i = 0; i < LEN_JSON_OBJECT_PLAN; i++)
    {
        e2p_data.write_32(oper.list_addr_output[i], oper.output);
        e2p_data.write_32(oper.list_addr_output[i] + E2PE_LEN_OUTPUT, oper.defect);
        e2p_data.write_32(oper.list_addr_output[i] + E2PE_LEN_OUTPUT + E2PE_LEN_DEFECT, oper.target);
    }
    rst_Qfont_Qrear();
}
/////////////////////////////////////////////////////////////////////////////////////
void rst_Qfont_Qrear()
{
    e2p_data_write_16(E2PE_ADR_DATA_Q_REAR, 0);
    e2p_data_write_16(E2PE_ADR_DATA_Q_FRONT, 0);
}
/////////////////////////////////////////////////////////////////////////////////////
void reset_mc_time()
{
    // reset m/c time
    oper.power_ontime = 0;
    oper.motor_runtime = 0;
    oper.handling_time = 0;
    oper.cycle_time = 0;
    oper.sumof_stitch = 0;
    oper.sumof_trim = 0;

    oper.power_time_line = oper.power_ontime;
    oper.power_time_reset = millis();

    save_mc_time();
}
/////////////////////////////////////////////////////////////////////////////////////
void save_mc_time()
{
    calDapot();

    e2p_data.write_32(E2PE_ADR_MC_DAPOT, oper.power_ontime);
    e2p_data.write_32(E2PE_ADR_MC_DAMRT, oper.motor_runtime);
    e2p_data.write_32(E2PE_ADR_MC_HANDLE, oper.handling_time);
    e2p_data.write_32(E2PE_ADR_MC_CT, oper.cycle_time);
    e2p_data.write_32(E2PE_ADR_MC_NUM_STI, oper.sumof_stitch);
    e2p_data.write_32(E2PE_ADR_MC_NUM_TRI, oper.sumof_trim);
}
/////////////////////////////////////////////////////////////////////////////////////
void calDapot()
{
    oper.power_ontime = oper.power_time_line + ((millis() - oper.power_time_reset) / 1000);
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