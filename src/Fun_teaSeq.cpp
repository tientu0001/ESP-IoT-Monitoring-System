

#include "config.h"
#include "Fun_teaSeq.h"

#include <RingBuf.h>

#include "Fun_operation.h"
#include "Fun_e2p.h"

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
#define INT_IN_0 3  // sen0 - interrupt
#define NOR_IO_0 16 // sen1 - normal input
#define RLY_IN_0 A0 // sol0 - adc
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
#define LEN_BUFF_INT 200
typedef struct BUFF_ISR
{
    volatile uint8_t qFont;
    // uint8_t qFont;
    uint8_t qRear;
    uint32_t time_happened[LEN_BUFF_INT];
} BUFF_ISR;

BUFF_ISR sen0;
uint32_t ulMilSen0Old;

typedef struct IO_SUB
{
    uint8_t ucId;
    uint8_t ucStep;

} IO_SUB;

struct IO_SUB sen1;
struct IO_SUB adc;
/////////////////////////////////////////////////////////////////////////////////////
Fun_teaSeq tea;
/////////////////////////////////////////////////////////////////////////////////////
//---------//
#define CHK_SEN_DLY 0x0E  // 1110 - ignore delay signal
#define CHK_SEN_SEN0 0x0D // 1101 - ignore sen0 signal
#define CHK_SEN_SEN1 0x0B // 1011 - ignore sen1 signal
#define CHK_SEN_SOL0 0x07 // 0111 - ignore sol0 signal
#define CHK_SEN_ALL 0x0F  // 1111 - detect all signal

#define TEA_SEQ_RES_VALUE 0x00
#define TEA_SEQ_DETECT 0x01
#define OUT_SEQ_TEA_DLY 0x02
#define OUT_SEQ_TEA_SEN0 0x03
#define OUT_SEQ_TEA_SOL0 0x04
#define OUT_SEQ_TEA_SEN1 0x05
#define OUT_SEQ_FINISH 0x06
/////////////////////////////////////////////////////////////////////////////////////
#define TEA_SEQ_DLY_DELTA 0x01

#define TEA_SEQ_OUT_RUN 0x00
#define TEA_SEQ_OUT_TEA 0x10

#define TEA_SEQ_TEA_NONE 0x00
#define TEA_SEQ_TEA_START 0x01
#define TEA_SEQ_TEA_END 0x02

#define OUT_SEQ_RUN_START 0x00
#define OUT_SEQ_RUN_SET 0x10
#define OUT_SEQ_RUN_FINISH 0x20
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
SEQ_OUT_TEA OtTea;
SEQ_OUT_RUN OtRun;
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
void IRAM_ATTR detFalling()
{
    // sen0.time_happened[sen0.qFont] = millis();
    // sen0.qFont = (sen0.qFont + 1) % LEN_BUFF_INT;

    uint32_t time_falling = millis();
    if (time_falling - oper.ulTimeSen0HappnedFinal >= tea.delay_sen0)
    {
        if (!tea.bIgnoreSen0)
        {
            oper.sumof_stitch++;

            // stitch++ for teaseq. teaching - running
            if (!tea.ignore_sen_0_all)
                tea.CNT_TEA_SEN0++;
        }

        if (!oper.bMotorRun) // if not check motor, enable flag
        {
            oper.ulTimeSen0HappnedFirst = time_falling;
            oper.bMotorRun = true;
        }

        oper.ulTimeSen0HappnedFinal = time_falling;
    }
}
/////////////////////////////////////////////////////////////////////////////////////
void SignalCtrl(IO_SUB *pKey) // no interrupt
{
    switch (pKey->ucStep)
    {
    case 0:
    {
        if (digitalRead(pKey->ucId) == LOW)
        {
            // tea.CNT_TEA_SEN1++;
            // oper.increase_defect();
            if (!tea.bIgnoreSen0)
            {
                tea.CNT_TEA_SEN1++;
                oper.sumof_trim++;

                tea.ulIgnoreTime = millis(); // detect sol0 and ignore all sensor until 500ms
                tea.bIgnoreSen0 = true;
            }

            pKey->ucStep = pKey->ucStep + 1;
        }

        break;
    }
    case 1:
    {
        if (digitalRead(pKey->ucId) == HIGH)
            pKey->ucStep = 0;
        break;
    }
    default:
        pKey->ucStep = 0;
        break;
    }
}
/////////////////////////////////////////////////////////////////////////////////////
void AnalogCtrl(IO_SUB *pKey)
{
    switch (pKey->ucStep)
    {
    case 0:
    {
        if (map(analogRead(pKey->ucId), 0, 1024, 0, 3.3) > 1.5)
        {
            pKey->ucStep = pKey->ucStep + 1;
        }
        break;
    }
    case 1:
    {
        if (map(analogRead(pKey->ucId), 0, 1024, 0, 3.3) > 1.5)
        {
            if (!tea.bIgnoreSen0)
            {
                tea.CNT_TEA_SOL0++;
                oper.sumof_trim++;

                tea.ulIgnoreTime = millis(); // detect sol0 and ignore all sensor until 500ms
                tea.bIgnoreSen0 = true;
            }

            pKey->ucStep = pKey->ucStep + 1;
        }
        else
            pKey->ucStep = 0;

        break;
    }
    case 2:
    {
        if (map(analogRead(pKey->ucId), 0, 1024, 0, 3.3) < 0.2)
        {
            pKey->ucStep = 0;
        }

        break;
    }
    default:
        pKey->ucStep = 0;
        break;
    }
}
/////////////////////////////////////////////////////////////////////////////////////
uint8_t get_statusSen0()
{
    return digitalRead(INT_IN_0);
}
/////////////////////////////////////////////////////////////////////////////////////
void teaSeq_detect_signal() // 10ms
{
    SignalCtrl(&sen1);
    // AnalogCtrl(&adc);
}
/////////////////////////////////////////////////////////////////////////////////////
void update_storeTeachingSequence()
{
    for (int i = 0; i < E2PE_LEN_TEA_SEQ; i++)
        tea.ucTeaSeqStore[oper.current_idx_plan][i] = e2p_info.read(E2PE_ADR_TEA_SEQ + i);
}
/////////////////////////////////////////////////////////////////////////////////////
void update_runTeaSeqWithCurrentIdxPlan()
{
    for (int i = 0; i < E2PE_LEN_TEA_SEQ; i++)
        e2p_info.write(E2PE_ADR_TEA_SEQ + i, tea.ucTeaSeqStore[oper.current_idx_plan][i]);
}
/////////////////////////////////////////////////////////////////////////////////////
void rst_teachingSequenceRun()
{
    e2p_info.write(E2PE_ADR_TEA_SEQ, TEA_SEQ_END_ID);
}
/////////////////////////////////////////////////////////////////////////////////////
void rst_idxTeachingSequence(uint8_t idx)
{
    e2p_info.write(E2PE_ADR_OUT_SEQ + idx * E2PE_LEN_TEA_SEQ, TEA_SEQ_END_ID);
}
/////////////////////////////////////////////////////////////////////////////////////
void rst_allTeachingSequence()
{
    for (int i = 0; i < LEN_JSON_OBJECT_PLAN; i++)
        rst_idxTeachingSequence(i);

    rst_teachingSequenceRun();
}
/////////////////////////////////////////////////////////////////////////////////////
void read_allTeachingSequence()
{
    // read all teaching sequence
    for (int i = 0; i < LEN_JSON_OBJECT_PLAN; i++)
    {
        for (int j = 0; j < E2PE_LEN_TEA_SEQ; j++)
            tea.ucTeaSeqStore[i][j] = e2p_info.read(E2PE_ADR_OUT_SEQ + (i * E2PE_LEN_TEA_SEQ) + j);
    }
}
/////////////////////////////////////////////////////////////////////////////////////
void save_allTeachingSequence()
{
    // read all teaching sequence
    for (int i = 0; i < LEN_JSON_OBJECT_PLAN; i++)
    {
        for (int j = 0; j < E2PE_LEN_TEA_SEQ; j++)
            e2p_info.write(E2PE_ADR_OUT_SEQ + (i * E2PE_LEN_TEA_SEQ) + j, tea.ucTeaSeqStore[i][j]);
    }
}
/////////////////////////////////////////////////////////////////////////////////////
void teaseq_increaseDelayTeachMode()
{
    if (tea.ucMode == TEA_SEQ_OUT_TEA)
        tea.CNT_TEA_DLY++;
}
/////////////////////////////////////////////////////////////////////////////////////
void teaseq_increaseDelayRunMode()
{
    if (tea.ucMode == TEA_SEQ_OUT_RUN)
        tea.CNT_TEA_DLY++;
}
/////////////////////////////////////////////////////////////////////////////////////
void teaseq_startTeachMode()
{
    tea.ucStartEndTeaching = TEA_SEQ_TEA_START;
}
/////////////////////////////////////////////////////////////////////////////////////
void teaseq_endTeachMode()
{
    tea.ucStartEndTeaching = TEA_SEQ_TEA_END;
}
/////////////////////////////////////////////////////////////////////////////////////
void teqseq_restartRunMode()
{
    OtRun.ucMode = OUT_SEQ_RUN_START;
}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
void Fun_teaSeq::getListTeaSeq()
{
    uint16_t usAddr, usValue;
    usAddr = E2PE_ADR_TEA_SEQ;
    ucListCnt = 0;
    for (uint8_t i = 0; i < 10; i++)
    {
        if (e2p_info.read(usAddr + i * 3 + 0) == TEA_SEQ_END_ID)
            break;
        ucListName[i] = e2p_info.read(usAddr + i * 3 + 0);
        usValue = (uint16_t)e2p_info.read(usAddr + i * 3 + 1) << 8;
        usValue += (uint16_t)e2p_info.read(usAddr + i * 3 + 2);
        usListValue[i] = usValue;
        ucListCnt++;
    }
}
/////////////////////////////////////////////////////////////////////////////////////
Fun_teaSeq::Fun_teaSeq(/* args */)
{
}
/////////////////////////////////////////////////////////////////////////////////////
Fun_teaSeq::~Fun_teaSeq()
{
}
/////////////////////////////////////////////////////////////////////////////////////
void Fun_teaSeq::init()
{
    // signal - SEN0
    pinMode(INT_IN_0, FUNCTION_3);
    pinMode(INT_IN_0, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(INT_IN_0), detFalling, FALLING); // delay

    // signal - SEN1
    pinMode(NOR_IO_0, INPUT_PULLUP);
    sen1.ucId = NOR_IO_0;

    // signal ADC
    adc.ucId = RLY_IN_0;
}
/////////////////////////////////////////////////////////////////////////////////////
void Fun_teaSeq::loop()
{
    if (oper.bAndonMode || oper.in_out_iot_mode_enable)
        return;

    // sen0_interrupt();

    switch (ucMode)
    {
    case TEA_SEQ_OUT_RUN:
        seqOutRun(&OtRun);
        break;
    case TEA_SEQ_OUT_TEA:
        seqOutTeach(&OtTea);
        break;
    default:
        ucMode = TEA_SEQ_OUT_RUN;
        break;
    }

    if (bTeaSeqChanged)
    {
        bTeaSeqChanged = false;

        // update store teaching
        update_storeTeachingSequence();
        save_allTeachingSequence();
        e2p_info.commit_flag = true;

        bSendIdxTeaSeq2Svr = true;
    }

    if (bIgnoreSen0)
    {
        if (millis() - ulIgnoreTime > TEA_DLY_IGNORE_SEN0)
            bIgnoreSen0 = false;
    }
}
/////////////////////////////////////////////////////////////////////////////////////
void Fun_teaSeq::sen0_interrupt()
{
//     if (sen0.qFont != sen0.qRear)
//     {
//         // uint16_t cnt_return = 0;
//         // while (sen0.qFont != sen0.qRear) // return_1 : maybe return ~.~
//         // {
//         uint32_t time_falling = sen0.time_happened[sen0.qRear];

//         // stitch happen
//         if (time_falling - ulMilSen0Old >= delay_sen0)
//         {
//             // if (!bIgnoreSen0)
//             if (!bIgnoreSen0)
//             {
//                 oper.sumof_stitch++;

//                 // stitch++ for teaseq. teaching - running
//                 if (!tea.ignore_sen_0_all)
//                     CNT_TEA_SEN0++;
//             }

//             if (!oper.bMotorRun) // if not check motor, enable flag
//             {
//                 oper.ulTimeSen0HappnedFirst = time_falling;
//                 oper.bMotorRun = 1;

// #ifdef ESP_DEBUG_TEASEQ
//                 Serial.printf("motor start: %u \n", oper.ulTimeSen0HappnedFirst);
// #endif
//             }

//             oper.ulTimeSen0HappnedFinal = time_falling;
//             ulMilSen0Old = time_falling;
//             // ulMilSen0Old = millis();

//             oper.u32_stitch_happend = millis();
//         }

//         sen0.qRear = (sen0.qRear + 1) % LEN_BUFF_INT;

//         //     if (++cnt_return >= LEN_BUFF_INT) // return_2 : alway return
//         //         break;
//         // }
//     }
}
/////////////////////////////////////////////////////////////////////////////////////
void Fun_teaSeq::seqOutTeach(SEQ_OUT_TEA *pHdl)
{
    switch (pHdl->ucMode)
    {
    ///////////////////////////////////////////////////////////////////////////
    case TEA_SEQ_RES_VALUE:
    {
        CNT_TEA_DLY = 0;
        CNT_TEA_SEN0 = 0;
        CNT_TEA_SEN1 = 0;
        CNT_TEA_SOL0 = 0;

        CNT_TEA_DLY_OLD = 0;
        CNT_TEA_SEN0_OLD = 0;
        CNT_TEA_SOL0_OLD = 0;
        CNT_TEA_SEN1_OLD = 0;

        usE2pAddr = E2PE_ADR_TEA_SEQ;
        pHdl->ucMode = TEA_SEQ_DETECT;
        break;
    }
    ///////////////////////////////////////////////////////////////////////////
    case TEA_SEQ_DETECT:
    {
        SeqOutTeachCheckSensor(CHK_SEN_ALL, &pHdl->ucMode);
        break;
    }
    ///////////////////////////////////////////////////////////////////////////
    case OUT_SEQ_TEA_DLY:
    {
        if (CNT_TEA_DLY != CNT_TEA_DLY_OLD)
        {
            usListValue[ucListCnt - 1] = CNT_TEA_DLY;
            CNT_TEA_DLY_OLD = CNT_TEA_DLY;

            lcd_refresh();
        }
        SeqOutTeachCheckSensor(CHK_SEN_DLY, &pHdl->ucMode);
        break;
    }
    ///////////////////////////////////////////////////////////////////////////
    case OUT_SEQ_TEA_SEN0:
    {
        if (CNT_TEA_SEN0 != CNT_TEA_SEN0_OLD)
        {
            usListValue[ucListCnt - 1] = CNT_TEA_SEN0;
            CNT_TEA_SEN0_OLD = CNT_TEA_SEN0;

            lcd_refresh();
        }
        SeqOutTeachCheckSensor(CHK_SEN_SEN0, &pHdl->ucMode);
        break;
    }
    ///////////////////////////////////////////////////////////////////////////
    case OUT_SEQ_TEA_SEN1:
    {
        if (CNT_TEA_SEN1 != CNT_TEA_SEN1_OLD)
        {
            usListValue[ucListCnt - 1] = CNT_TEA_SEN1;
            CNT_TEA_SEN1_OLD = CNT_TEA_SEN1;

            lcd_refresh();
        }
        SeqOutTeachCheckSensor(CHK_SEN_SEN1, &pHdl->ucMode);
        break;
    }
    ///////////////////////////////////////////////////////////////////////////
    case OUT_SEQ_TEA_SOL0:
    {
        if (CNT_TEA_SOL0 != CNT_TEA_SOL0_OLD)
        {
            usListValue[ucListCnt - 1] = CNT_TEA_SOL0;
            CNT_TEA_SOL0_OLD = CNT_TEA_SOL0;

            lcd_refresh();
        }
        SeqOutTeachCheckSensor(CHK_SEN_SOL0, &pHdl->ucMode);
        break;
    }
    ///////////////////////////////////////////////////////////////////////////
    case OUT_SEQ_FINISH:
    {
        if (bDetectSensor)
        {
            bDetectSensor = false;

            // teaching changed
            e2p_info.write(usE2pAddr, TEA_SEQ_END_ID);

            // for check teaching changed & send to server
            bTeaSeqChanged = true;
            ulMilTeaSeqChanged = millis();
        }

        ucMode = TEA_SEQ_OUT_RUN;
        OtRun.ucMode = OUT_SEQ_RUN_START;

        tea.ignore_sen_0_all = false;
        break;
    }
    ///////////////////////////////////////////////////////////////////////////
    default:
        pHdl->ucMode = OUT_SEQ_FINISH;
        break;
    }
}
/////////////////////////////////////////////////////////////////////////////////////
void Fun_teaSeq::seqOutRun(SEQ_OUT_RUN *pHdl)
{
    switch (pHdl->ucMode)
    {
    ///////////////////////////////////////////////////////////////////////////
    case OUT_SEQ_RUN_START:
    {
        usE2pAddr = E2PE_ADR_TEA_SEQ;
        pHdl->ucMode = OUT_SEQ_RUN_SET;
        break;
    }
    ///////////////////////////////////////////////////////////////////////////
    case OUT_SEQ_RUN_SET:
    {
        pHdl->ucSenType = e2p_info.read(usE2pAddr++);
        pHdl->ucSenLim = (uint16_t)e2p_info.read(usE2pAddr++) << 8;
        pHdl->ucSenLim += (uint16_t)e2p_info.read(usE2pAddr++);
        switch (pHdl->ucSenType)
        {
        case TEA_SEQ_END_ID:
            pHdl->ucMode = pHdl->ucSenType;
            break;
        case TEA_SEQ_DLY_ID:
            pHdl->ucMode = pHdl->ucSenType;
            CNT_TEA_DLY = 0;
            break;
        case TEA_SEQ_SEN0_ID:
            pHdl->ucMode = pHdl->ucSenType;
            CNT_TEA_SEN0 = 0;
            break;
        case TEA_SEQ_SEN1_ID:
            pHdl->ucMode = pHdl->ucSenType;
            CNT_TEA_SEN1 = 0;
            break;
        case TEA_SEQ_SOL0_ID:
            pHdl->ucMode = pHdl->ucSenType;
            CNT_TEA_SOL0 = 0;
            break;
        default:
            pHdl->ucMode = TEA_SEQ_END_ID;
            break;
        }
        break;
    }
    ///////////////////////////////////////////////////////////////////////////
    case TEA_SEQ_END_ID:
        pHdl->ucMode = OUT_SEQ_RUN_START;
        break;
    ///////////////////////////////////////////////////////////////////////////
    case TEA_SEQ_DLY_ID:
        if (CNT_TEA_DLY >= pHdl->ucSenLim)
            pHdl->ucMode = OUT_SEQ_RUN_FINISH;
        break;
    case TEA_SEQ_SEN0_ID:
        if (CNT_TEA_SEN0 >= pHdl->ucSenLim)
            pHdl->ucMode = OUT_SEQ_RUN_FINISH;
        break;
    case TEA_SEQ_SEN1_ID:
        if (CNT_TEA_SEN1 >= pHdl->ucSenLim)
            pHdl->ucMode = OUT_SEQ_RUN_FINISH;
        break;
    case TEA_SEQ_SOL0_ID:
        if (CNT_TEA_SOL0 >= pHdl->ucSenLim)
            pHdl->ucMode = OUT_SEQ_RUN_FINISH;
        break;
    ///////////////////////////////////////////////////////////////////////////
    case OUT_SEQ_RUN_FINISH:
    {
        if (e2p_info.read(usE2pAddr) == TEA_SEQ_END_ID)
        {
            // increase output
            oper.increase_output();

            pHdl->ucMode = OUT_SEQ_RUN_START;
        }
        else
            pHdl->ucMode = OUT_SEQ_RUN_SET;
        break;
    }
    default:
        pHdl->ucMode = OUT_SEQ_RUN_START;
        break;
    }

    if (ucStartEndTeaching == TEA_SEQ_TEA_START)
    {
        ucMode = TEA_SEQ_OUT_TEA;
        OtTea.ucMode = TEA_SEQ_RES_VALUE;

        if (tea.ignore_sen0_teaseq)
            tea.ignore_sen_0_all = true;
    }
}
/////////////////////////////////////////////////////////////////////////////////////
void Fun_teaSeq::SeqOutTeachCheckSensor(uint8_t ucChkList, uint8_t *pMode)
{
    uint8_t ucSaveFlag = 0;
    // check dly signal
    if ((((ucChkList >> 0) & 0x01) == 0x01) && (CNT_TEA_DLY > 0))
    {
        bDetectSensor = true;
        ucSaveFlag = 1;
        *pMode = OUT_SEQ_TEA_DLY;
        if (ucChkList == CHK_SEN_ALL)
            ucListCnt = 1;
        else if (ucListCnt < 10)
            ucListCnt++;

        ucListName[ucListCnt - 1] = TEA_SEQ_DLY_ID;
    }
    // check sen0 signal
    else if ((((ucChkList >> 1) & 0x01) == 0x01) && (CNT_TEA_SEN0 > 0))
    {
        bDetectSensor = true;
        ucSaveFlag = 1;
        *pMode = OUT_SEQ_TEA_SEN0;
        if (ucChkList == CHK_SEN_ALL)
            ucListCnt = 1;
        else if (ucListCnt < 10)
            ucListCnt++;
        ucListName[ucListCnt - 1] = TEA_SEQ_SEN0_ID;
    }
    // check sen1 signal
    else if ((((ucChkList >> 2) & 0x01) == 0x01) && (CNT_TEA_SEN1 > 0))
    {
        bDetectSensor = true;
        ucSaveFlag = 1;
        *pMode = OUT_SEQ_TEA_SEN1;
        if (ucChkList == CHK_SEN_ALL)
            ucListCnt = 1;
        else if (ucListCnt < 10)
            ucListCnt++;
        ucListName[ucListCnt - 1] = TEA_SEQ_SEN1_ID;
    }
    // check sol0 signal
    else if ((((ucChkList >> 3) & 0x01) == 0x01) && (CNT_TEA_SOL0 > 0))
    {
        bDetectSensor = true;
        ucSaveFlag = 1;
        *pMode = OUT_SEQ_TEA_SOL0;
        if (ucChkList == CHK_SEN_ALL)
            ucListCnt = 1;
        else if (ucListCnt < 10)
            ucListCnt++;
        ucListName[ucListCnt - 1] = TEA_SEQ_SOL0_ID;
    }

    if (ucChkList == CHK_SEN_ALL)
        ucSaveFlag = 0;

    //  Finish teaching
    if (ucStartEndTeaching == TEA_SEQ_TEA_END)
    {
        ucSaveFlag = 1;
        *pMode = OUT_SEQ_FINISH;
    }

    if (ucSaveFlag == 1)
    {
        if (usE2pAddr >= (E2PE_ADR_TEA_SEQ + E2PE_LEN_TEA_SEQ - 1))
            usE2pAddr -= 3;
        switch (ucChkList)
        {
        case CHK_SEN_DLY:
            SeqSaveSenCnt(&usE2pAddr, TEA_SEQ_DLY_ID, &CNT_TEA_DLY, &CNT_TEA_DLY_OLD);
            break;
        case CHK_SEN_SEN0:
            SeqSaveSenCnt(&usE2pAddr, TEA_SEQ_SEN0_ID, &CNT_TEA_SEN0, &CNT_TEA_SEN0_OLD);
            break;
        case CHK_SEN_SEN1:
            SeqSaveSenCnt(&usE2pAddr, TEA_SEQ_SEN1_ID, &CNT_TEA_SEN1, &CNT_TEA_SEN1_OLD);
            break;
        case CHK_SEN_SOL0:
            SeqSaveSenCnt(&usE2pAddr, TEA_SEQ_SOL0_ID, &CNT_TEA_SOL0, &CNT_TEA_SOL0_OLD);
            break;
        default:
            break;
        }
    }
}
/////////////////////////////////////////////////////////////////////////////////////
void Fun_teaSeq::SeqSaveSenCnt(uint16_t *usAddress, uint8_t ucSeqId, uint16_t *usCnt, uint16_t *usCnt_Old)
{
    uint8_t ucValue;
    e2p_info.write(*usAddress, ucSeqId);
    *usAddress = *usAddress + 1;
    ucValue = (uint8_t)(*usCnt >> 8);
    e2p_info.write(*usAddress, ucValue);
    *usAddress = *usAddress + 1;
    ucValue = (uint8_t)(*usCnt);
    e2p_info.write(*usAddress, ucValue);
    *usAddress = *usAddress + 1;

    *usCnt = 0;
    *usCnt_Old = 0;
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