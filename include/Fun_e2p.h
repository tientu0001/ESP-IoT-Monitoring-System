#ifndef FUN_E2P_H
#define FUN_E2P_H
/////////////////////////////////////////////////////////////////////////////////////
#include "EEPROM.h"

class Fun_e2p
{
private:
    EEPROMClass *EEPROM;

public:
    Fun_e2p(uint32_t sector);
    ~Fun_e2p();

    void begin(uint32_t addr);
    bool commit();
    uint8_t read(uint16_t usAddr);
    void write(uint16_t usAddr, uint8_t ucData);
    void write_16(uint16_t usAddr, uint16_t usData);
    uint16_t read_16(uint16_t usAddr);
    void write_32(uint16_t usAddr, uint32_t ulData);
    uint32_t read_32(uint16_t usAddr);
    void write_buff(uint16_t usAddr, char *buff, uint8_t ucLen);
    void read_buff(uint16_t usAddr, char *buff, uint8_t ucLen);
    void StringToE2P(uint16_t usAddr, uint8_t ucLen, String stStr);
    String E2PtoString(uint16_t usAddr, uint8_t ucLen);

    bool commit_flag;
};
/////////////////////////////////////////////////////////////////////////////////////
extern Fun_e2p e2p_info;
extern Fun_e2p e2p_data;
extern Fun_e2p e2p_network;
/////////////////////////////////////////////////////////////////////////////////////
#endif // FUN_E2P_H