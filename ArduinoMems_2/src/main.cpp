#include <Arduino.h>
#include "structs.h"
#include "mems_spi.h"
#include "serial_receiver.h"

const byte num_Chars = 5;
uint8_t received_Chars[num_Chars];
uint8_t echo_Chars[num_Chars];
byte recvd_Count;
byte recvd_Byte;
byte i;
char arg0;
uint16_t arg1, arg2;

Mems_Options m_Mems_Options;
Pin_Numbers m_Pin_Nos;

//uint8_t serial_Cmd;
//Mems_XY m_Mems_XY;
//Ioptron_XY m_Ioptron_XY;
//Serial_Receiver my_Serial = Serial_Receiver();

Mems_SPI my_Mems = Mems_SPI();

void setup() {
  // Read these values from the specific MEMS device datasheet.
  // Probably trim down the v_Difference_Max by a few volts just
  // to be safe.
  m_Mems_Options.f_Clk = 220;
  m_Mems_Options.v_Bias_Volts = 80;
  m_Mems_Options.v_Difference_Max = 159;

  m_Pin_Nos.f_Clk_Pin = 5;
  m_Pin_Nos.mems_En_Pin = 3;
  m_Pin_Nos.mems_Sync_Pin = 4;
  m_Pin_Nos.progd_Pin = 7;

  my_Mems.Init(m_Mems_Options, m_Pin_Nos);

  Serial.begin(115200, SERIAL_8N1);
  Serial.setTimeout(200);
}

void loop() {
  // put your main code here, to run repeatedly:
  if(Serial.available() > 0){
    if(Serial.readBytes(received_Chars, num_Chars) == num_Chars){
      arg0 = received_Chars[0];
      arg1 = ((uint16_t)received_Chars[1] << 8) +
             ((uint16_t)received_Chars[2]&0xFF);
      arg2 = ((uint16_t)received_Chars[3] << 8) +
             ((uint16_t)received_Chars[4]&0xFF);

      if (arg0 == 'm'){
        my_Mems.Move(arg1, arg2);
      }
      else if(arg0 == 'I'){
        my_Mems.Set_HV_Driver(true);
      }
      else if(arg0 == 'X'){
        my_Mems.Set_HV_Driver(false);
      }

    }

  }

  for(int i = 0; i < num_Chars; i++){
    received_Chars[i] = 0;
  }
}