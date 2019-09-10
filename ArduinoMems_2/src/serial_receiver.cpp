#include "serial_receiver.h"
#include <Arduino.h>
#include <HardwareSerial.h>

Serial_Receiver::Serial_Receiver(){
    Init();
}

Serial_Receiver::~Serial_Receiver(){

}

int Serial_Receiver::Init(){
    Serial.begin(115200, SERIAL_8N1);
    // Timeout too short, packets get lost, timeout too long laaagg.
    Serial.setTimeout(20);
    return 0;
}

uint8_t Serial_Receiver::Get_Command(){
    bool done = false;
    while(!done){
        waiting_Bytes = Serial.available();
        if (waiting_Bytes > 0){
            recvd_Byte = Serial.read();

            if (
                (recvd_Byte == 'M') |
                (recvd_Byte == 'I') |
                (recvd_Byte == 'X') |
                (recvd_Byte == 'O')
            ){
                m_Cmd_Mode = mems;
            }
            else if (
                (recvd_Byte == 'm')
            ){
                m_Cmd_Mode = ioptron;
            }
            else{
                m_Cmd_Mode = undefined;
            }
            done = true;
        }
    }
    return recvd_Byte;
}

Mems_XY Serial_Receiver::Get_Mems_Move_Data(){
    bool done = false;
    while(!done){
        waiting_Bytes = Serial.available();
        if (waiting_Bytes >= mems_Buffer_Size){
            Serial.readBytes(mems_Buffer, mems_Buffer_Size);

            m_Mems_XY.x = ((uint16_t)mems_Buffer[0] << 8) + 
                        ((uint16_t)mems_Buffer[1]&0xFF);
            m_Mems_XY.y = ((uint16_t)mems_Buffer[2] << 8) +     
                        ((uint16_t)mems_Buffer[3]&0xFF);
            done = true;
        }
    }

    for(int i=0; i<mems_Buffer_Size; i++){
        mems_Buffer[i] = 0;
    }

    return m_Mems_XY;
}

Ioptron_XY Serial_Receiver::Get_Ioptron_Move_Data(){
    bool done = false;
    while(!done){
        waiting_Bytes = Serial.available();
        if (waiting_Bytes >= ioptron_Buffer_Size){
            Serial.readBytes(ioptron_Buffer, ioptron_Buffer_Size);
        
            m_Ioptron_XY.x = ((uint32_t)ioptron_Buffer[0] << 24) + 
                            ((uint32_t)ioptron_Buffer[1] << 16) +
                            ((uint32_t)ioptron_Buffer[2] << 8) +
                            ((uint32_t)ioptron_Buffer[3] & 0xFF);
            m_Ioptron_XY.y = ((uint32_t)ioptron_Buffer[4] << 24) + 
                            ((uint32_t)ioptron_Buffer[5] << 16) +
                            ((uint32_t)ioptron_Buffer[6] << 8) +
                            ((uint32_t)ioptron_Buffer[7] & 0xFF);
            done = true;
        }
    }

    for(int i=0; i<ioptron_Buffer_Size; i++){
        ioptron_Buffer[i] = 0;
    }

    return m_Ioptron_XY;
}

int Serial_Receiver::Read_Discard(){
    int num_Reads = 0;
    while(Serial.read() >= 0){
        num_Reads++;
    }
    return num_Reads;
}