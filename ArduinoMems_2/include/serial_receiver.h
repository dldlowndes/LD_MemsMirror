#ifndef SERIAL_RECV_H
#define SERIAL_RECV_H

#include "structs.h"
#include <stdint.h>

#define mems_Buffer_Size 4
#define ioptron_Buffer_Size 8

class Serial_Receiver{
    public:
        Serial_Receiver();
        ~Serial_Receiver();

        uint8_t Get_Command();
        Mems_XY Get_Mems_Move_Data();
        Ioptron_XY Get_Ioptron_Move_Data();

        int Read_Serial();
        int Read_Discard();

    private:
        int Init();
        
        uint8_t mems_Buffer[mems_Buffer_Size];
        uint8_t ioptron_Buffer[ioptron_Buffer_Size];

        int waiting_Bytes;
        int recvd_Count;
        int recvd_Byte;
        uint8_t cmd_Byte;
        Cmd_Mode m_Cmd_Mode;

        Mems_XY m_Mems_XY;
        Ioptron_XY m_Ioptron_XY;
};

#endif // SERIAL_RECV_H