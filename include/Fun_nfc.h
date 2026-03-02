#ifndef FUN_NFC_H
#define FUN_NFC_H
/////////////////////////////////////////////////////////////////////////////////////
#include "config.h"
/////////////////////////////////////////////////////////////////////////////////////
class Fun_nfc
{
private:
    void handle_nfc_blank();

public:
    Fun_nfc();
    ~Fun_nfc();

    void init();
    void loop();

    String getUidNfcTag_reverse();

    void ckTAG_ID_UL();
    void ckTAG_ID_M1K();
    void ckTAG_ID_PLX();

    uint8_t rfid_ReadTagData_ult();
    uint8_t rfid_WriteTagData_ult();
    uint8_t rfid_ReadTagData_m1k();
    uint8_t rfid_WriteTagData_m1k();

    void NFC_READ_TYPE_NETWORK();
    void NFC_WRITE_TYPE_NETWORK();
    void FUN_NFC_READ_OUTPUT();
    void NFC_WRITE_TYPE_OUTPUT();

    // variables
    uint32_t ulMillis;

    uint8_t ucMode;
    uint8_t ucWriteType;
    uint8_t ucReadType;
    uint8_t ucWriteEndFlag;
    uint8_t ucWriteTiOutCnt;
    uint8_t ucDataBuf[RFID_DATA_LEN];

    uint8_t ucChkErr;
    uint32_t ulMil;

    String sTitle;
    char *pcDispChar;

    uint8_t ucFlagDynamic;
    uint8_t ucResDynSuc;

    String sUid;
    String sUidBlank;
    uint8_t ucUidBlank[E2PE_LEN_UID_BLA_NFC];
    uint8_t ucLenUidBlank;
    uint8_t bSendUidNfcBlank2Svr;

    bool get_uid_blank_again;

    uint8_t bSendUidWithWifiConnected;

    uint8_t ucFlaShoBlaUid;
    uint8_t ucFlaSenBlaUid;

    uint8_t ucIncDecType;
    uint8_t ucInDecScale;

    uint8_t ucChkNfcReadWrite;

    uint8_t ucChkNfcWriteTagOutput;
    uint8_t ucWrtOutSuc;

    uint8_t ucReadHandleType;
    uint8_t ucWaitWrtType;

    uint8_t ucNfcNumProId;

    uint8_t ucCopyNfcData;

    char ucSsid[E2PE_LEN_SSID];
    char ucPassword[E2PE_LEN_PASS];
    char ucSvrAddr[E2PE_LEN_SVR_ADDR];

    bool bHanldeSuccess;
};

/////////////////////////////////////////////////////////////////////////////////////
extern Fun_nfc nfc;
#endif // FUN_NFC_H