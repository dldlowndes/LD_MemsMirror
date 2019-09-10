#ifndef STRUCTS_H
#define STRUCTS_H

#include <stdint.h>

enum Cmd_Mode {mems, ioptron, undefined};

struct Mems_Options{
    uint16_t f_Clk;
    float v_Difference_Max;
    float v_Bias_Volts;
};

struct Pin_Numbers{
    uint8_t mems_En_Pin;
    uint8_t mems_Sync_Pin;
    uint8_t f_Clk_Pin;
    uint8_t progd_Pin;
};

struct Mems_XY{
    uint16_t x;
    uint16_t y;
};

struct Ioptron_XY{
    uint32_t x;
    uint32_t y;
};

#endif // STRUCTS_H