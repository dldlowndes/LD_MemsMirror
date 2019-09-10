#include "mems_spi.h"

Mems_SPI::Mems_SPI(){

}

Mems_SPI::Mems_SPI(Mems_Options mems_Opt, Pin_Numbers pin_Nos){
    m_Mems_Options = mems_Opt;
    m_Pin_Nos = pin_Nos;
    
    Init();
}

Mems_SPI::~Mems_SPI(){
    Set_HV_Driver(false);
}

int Mems_SPI::Init(Mems_Options mems_Opt, Pin_Numbers pin_Nos){
    m_Mems_Options = mems_Opt;
    m_Pin_Nos = pin_Nos;
    
    Init();
    return 0;
}

int Mems_SPI::Init(){
    // Init SPI.
    SPI.begin();
    // I think these settings will do?
    SPI.beginTransaction(SPISettings(24000000, MSBFIRST, SPI_MODE1));
    /*
    Mode, CPOL, CPHA, CKE
    0, 0, 0, 1
    1, 0, 1, 0
    2, 1, 0, 1
    3, 1, 1, 0
    */

    // Start with the HV drivers OFF.
    pinMode(m_Pin_Nos.mems_En_Pin, OUTPUT);
    digitalWrite(m_Pin_Nos.mems_En_Pin, LOW);

    // Start the sync pin high so it can go low when transfer starts.
    pinMode(m_Pin_Nos.mems_Sync_Pin, OUTPUT);
    digitalWrite(m_Pin_Nos.mems_Sync_Pin, HIGH);
    
    // All this does is tell an LED to shine if this program is running.
    pinMode(m_Pin_Nos.progd_Pin, OUTPUT);
    digitalWrite(m_Pin_Nos.progd_Pin, HIGH);

    // Filter clock frequency.
    tone(m_Pin_Nos.f_Clk_Pin, m_Mems_Options.f_Clk * 60);
    
    v_Bias_DAC = m_Mems_Options.v_Bias_Volts / (200.0 / 65535);
    // v_Difference_Max(volts) divided by the ratio of the maximum dac voltage
    // (200V) and the number of DAC units (65535) to give the number of DAC
    // units for the maximum v_Diff. In practice the more useful number is half
    // this since voltage is applied differentially.
    v_Diff_Max_DAC_Half = (m_Mems_Options.v_Difference_Max / (200.0 / 65535)) / 2;

    // delay?
    delayMicroseconds(100);

    // SPI Init strings from Picoamp datasheet.
    Send_SPI(0b101, 0b000, 0b1); // FULL RESET (0x280001)
    Send_SPI(0b111, 0b000, 0b1); // ENABLE INTERNAL REFERENCE (0x380001)
    Send_SPI(0b100, 0b000, 0b1111); // ENABLE ALL DAC CHANNELS (0x20000F)
    Send_SPI(0b110, 0b000, 0b0000); // SET LDAC MODE (0x300000)

    // Turn on the filter clock.
    // TODO: Add option and potential for separate X and Y clocks. This
    // is an option by chooing appropriate 0 ohm resistors on the board.

    // Set mems to origin.
    Set_HV_Driver(true);
    delayMicroseconds(100);
    Move_Origin();

    return 0;
}

int Mems_SPI::Send_SPI(byte cmd, byte addr, uint16_t data){
    // Pack the first byte of the SPI transfer
    spi_Cmd_Addr = ((cmd & 0x7) << 3) + (addr & 0x7);
    
    // Pull the sync pin low so the transfer is valid.
    digitalWrite(m_Pin_Nos.mems_Sync_Pin, LOW);
    //delayMicroseconds(100);

    // Send the command and address (8 bits) and then the data (16 bits)
    SPI.transfer(spi_Cmd_Addr);
    SPI.transfer16(data);
    //delayMicroseconds(100);

    // pull sync pin high to signify end of transmission.
    digitalWrite(m_Pin_Nos.mems_Sync_Pin, HIGH);

    return 0;
}

int Mems_SPI::Set_HV_Driver(bool hv_On){
    if (hv_On){
        digitalWrite(m_Pin_Nos.mems_En_Pin, HIGH);
    }
    else{
        Move_Origin();
        delayMicroseconds(100);
        digitalWrite(m_Pin_Nos.mems_En_Pin, LOW);
    }
    
    return 0;
}

int Mems_SPI::Move_Origin(){
    Send_SPI(WRITE_TO_DAC_UPDATE_ALL, DAC_ALL, v_Bias_DAC);
    return 0;
}

int Mems_SPI::Move(uint16_t x_Diff_Value, uint16_t y_Diff_Value){
    // fuck, this isn't right.
    // remember 32768 passed to this should lead to the DAC voltages being v_Bias.
    // 0 or 65535 should be clipped down such that V+ and V- are v_Diff_Max apart.

    // Is this clever or terrible? The values added/subtracted from the vbias
    // need to range from positive to negative values so store them in something
    // that can definitely hold them but then make sure that they're never too
    // big to over/underflow a uint16_t?
    int32_t x_DAC_Diff = ((uint32_t)x_Diff_Value - 32768);
    int32_t y_DAC_Diff = ((uint32_t)y_Diff_Value - 32768);

    // Disgusting but I can't think of a better way, at least the 2nd if only
    // gets evaluated when commanding extreme values... :(
    if (abs(x_DAC_Diff) > v_Diff_Max_DAC_Half){
        if (x_DAC_Diff > 0){
            x_DAC_Diff = v_Diff_Max_DAC_Half;
        }
        else{
            x_DAC_Diff = -v_Diff_Max_DAC_Half;
        }
    }
    if (abs(y_DAC_Diff) > v_Diff_Max_DAC_Half){
        if (y_DAC_Diff > 0){
            y_DAC_Diff = v_Diff_Max_DAC_Half;
        }
        else{
            y_DAC_Diff = -v_Diff_Max_DAC_Half;
        }
    }

    // I guess which channel is + and - for each channel is kind of arbitary?
    // Write to all the DACs. On the last write, instruct all the DACs to update.
    Send_SPI(WRITE_TO_DAC, DAC_A, (uint16_t)(v_Bias_DAC + x_DAC_Diff)); // X+
    Send_SPI(WRITE_TO_DAC, DAC_B, (uint16_t)(v_Bias_DAC - x_DAC_Diff)); // X-
    Send_SPI(WRITE_TO_DAC, DAC_C, (uint16_t)(v_Bias_DAC - y_DAC_Diff)); // Y-
    Send_SPI(WRITE_TO_DAC_UPDATE_ALL, DAC_D, (uint16_t)(v_Bias_DAC + y_DAC_Diff)); //Y+

    return 0;
}