#ifndef MEMS_SPI_H
#define MEMS_SPI_H

#include <stdint.h>
#include <SPI.h>
#include "structs.h"

class Mems_SPI{
    public:
        Mems_SPI();
        Mems_SPI(Mems_Options mems_Opt, Pin_Numbers pin_Nos);
        ~Mems_SPI();

        int Init(Mems_Options mems_Opt, Pin_Numbers pin_Nos);
        int Set_HV_Driver(bool hv_On);
        int Move_Origin();
        int Move(uint16_t x_Diff_Value, uint16_t y_Diff_Value);

    private:
        int Init();
        int Send_SPI(byte cmd, byte addr, uint16_t data);

        uint16_t v_Diff_Max_DAC_Half;
        
        // 32bit because I'm doing some horrible stuff in move and it reduces
        // the amount of casting to/from 16bit :(
        int32_t v_Bias_DAC;

        int16_t x_Diff_Value;
        int16_t y_Diff_Value; 

        // The byte to send to AD5664 which contains 2 don't care bits, 3 cmd
        // bits and 3 addr bits.
        uint8_t spi_Cmd_Addr;
        // The data to send over SPI relevant to the cmd and addr.
        uint32_t spi_Buffer;

        Mems_Options m_Mems_Options;
        Pin_Numbers m_Pin_Nos;

        // AD5664 commands
        uint8_t WRITE_TO_DAC = 0b000;
        uint8_t UPDATE_DAC = 0b001;
        uint8_t WRITE_TO_DAC_UPDATE_ALL = 0b010;
        uint8_t WRITE_UPDATE_DAC = 0b011;
        uint8_t DAC_POWER_DOWN = 0b100;
        uint8_t DAC_RESET = 0b110;
        uint8_t LDAC_SETUP = 0b110;
        uint8_t INT_REF_SETUP = 0b111;
        // AD5664 addresses
        uint8_t DAC_A = 0b000;
        uint8_t DAC_B = 0b001;
        uint8_t DAC_C = 0b010;
        uint8_t DAC_D = 0b011;
        uint8_t DAC_ALL = 0b111;

};

#endif // MEMS_SPI_H