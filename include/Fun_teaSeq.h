#ifndef FUN_SEQ_H
#define FUN_SEQ_H

#include <Arduino.h>

typedef struct SEQ_OUT_TEA
{
    uint8_t ucMode;
    uint8_t ucType;
} SEQ_OUT_TEA;

typedef struct SEQ_OUT_RUN
{
    uint8_t ucMode;
    uint8_t ucSenType;
    uint16_t ucSenLim;
} SEQ_OUT_RUN;

class Fun_teaSeq
{
public:
    static const uint16_t TEA_DEF_DLY_SEN0 = 10;   // default
    static const uint16_t TEA_MAX_DLY_SEN0 = 2000; // max delay sen0
    static const uint16_t INTERVAL_DLY_SEN0 = 100; // max delay sen0

    // functions
    Fun_teaSeq(/* args */);
    ~Fun_teaSeq();

    void init();
    void loop();

    void sen0_interrupt();

    void seqOutTeach(SEQ_OUT_TEA *pHdl);
    void seqOutRun(SEQ_OUT_RUN *pHdl);
    void getListTeaSeq();
    void SeqOutTeachCheckSensor(uint8_t ucChkList, uint8_t *pMode);
    void SeqSaveSenCnt(uint16_t *usAddress, uint8_t ucSeqId, uint16_t *usCnt, uint16_t *usCnt_Old);

    // variables
    uint16_t delay_sen0;

    uint8_t ucMode;

    uint16_t CNT_TEA_DLY;
    uint16_t CNT_TEA_SEN0;
    uint16_t CNT_TEA_SEN1;
    uint16_t CNT_TEA_SOL0;

    uint16_t CNT_TEA_DLY_OLD;
    uint16_t CNT_TEA_SEN0_OLD;
    uint16_t CNT_TEA_SEN1_OLD;
    uint16_t CNT_TEA_SOL0_OLD;

    uint8_t ucStartEndTeaching;

    uint8_t ucListCnt;
    uint8_t ucListName[(E2PE_LEN_TEA_SEQ - 1) / 3];
    uint16_t usListValue[(E2PE_LEN_TEA_SEQ - 1) / 3];

    uint8_t ucTeaSeqStore[LEN_JSON_OBJECT_PLAN][E2PE_LEN_TEA_SEQ]; // teaching all - 4 process_id. 4 teaching

    bool bSendIdxTeaSeq2Svr;
    bool bSendAllTeaSeq2Svr;

    uint32_t ulIgnoreTime;
    bool bIgnoreSen0;

    bool bTeaSeqChanged;

    bool ignore_sen0_teaseq;
    bool ignore_sen_0_all;

private:
    static const uint16_t TEA_DLY_IGNORE_SEN0 = 500; // ms- time noise cause trimmer happend

    uint16_t usE2pAddr;

    bool bDetectSensor;

    uint32_t ulMilTeaSeqChanged;
};

extern Fun_teaSeq tea;
#endif // FUN_SEQ_H