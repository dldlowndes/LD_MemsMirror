#include "LD_MemsMirror.h"

#include "rs232.h"
#include "LD_Util.h"

#include <iostream>
#include <random>
#include <unistd.h>

// Doesnt work;
// Try adding \n to the buffer (char10)
// Write a method for sending messages anyway
// Write a method for reading messages
//

namespace LD_MemsMirror{
    int ClipValue(float &value, float limit_hi, float limit_lo){
        if (value > limit_hi){
            value = limit_hi;
            return 1;
        }
        else if (value < limit_lo){
            value = limit_lo;
            return 1;
            }
        return 0;
    }

    int ClipValue(float &value){
        return ClipValue(value, 1, -1);
    }

    Mirror::Mirror(){
    }

    Mirror::Mirror(MirrorOptions my_Options){
        Init(my_Options);
    }

    Mirror::~Mirror(){
        Close();
    }

    int Mirror::Init(MirrorOptions my_Options){
        this->comport_Number = RS232_GetPortnr(my_Options.comport_Name.c_str());
        std::cout << "Com port: " << my_Options.comport_Name << " is number " << comport_Number << std::endl;
        RS232_OpenComport(comport_Number, 115200, "8n1");
        // Necessary. Arduino takes some time to set up afer connection?
        MySleep(2000);

        this->limit = my_Options.limit;

        // Send init command to MEMS;
        Set_HV_Driver(true);

        this->is_Initted = true;

        // Move to origin.
        Move(0, 0);

        std::cout << "Mirror ready" << "\n";
        return 0;
    }

    int Mirror::Set_HV_Driver(bool hv_On){
        if (hv_On){
            std::cout << "Toggle HV driver ON" << std::endl;
            this->outBuffer[0] = 'I';
        }
        else{
            this->outBuffer[0] = 'X';
            std::cout << "Toggle HV driver OFF" << std::endl;
        }
        SendCOM(outBuffer);
        MySleep(500);
        return 0;
    }

    int Mirror::SendCOM(std::vector<uint8_t> message){
        int bytes_Sent = RS232_SendBuf(comport_Number, message.data(), message.size());
        return bytes_Sent;
    }

    std::vector<char> Mirror::RecvCOM(int num_Bytes){
        // TODO: Implement.
        (void)num_Bytes;
        return inBuffer;
    }

    int Mirror::Move(float x, float y){
        if (is_Initted){
            // For safety and consistency. Make sure values in range -1, 1.
            ClipValue(x, limit, -limit);
            ClipValue(y, limit, -limit);

            // Convert into 0-65535 values.
            this->mems_X = (x+1)*32767;
            this->mems_Y = (y+1)*32767;

            this->outBuffer = {
                (uint8_t)('m'),
                (uint8_t)(mems_X >> 8),
                (uint8_t)(mems_X & 0xFF),
                (uint8_t)(mems_Y >> 8),
                (uint8_t)(mems_Y & 0xFF)
            };

            SendCOM(outBuffer);
            //std::cout << "Mirror set to " << mems_X << ", " << mems_Y << std::endl;

            this->mems_X_Current = this->mems_X;
            this->mems_Y_Current = this->mems_Y;

            return 0;
        }
        else{
            std::cout << "Mirror not initted. fail" << "\n";
            return 1;
        }
    }

    int Mirror::Close(){
        // Move to origin
        // Unclear if this is necessary but safest to assume that the mirror
        // probably doesn't like having it's bias voltages yanked out when it's
        // unbalanced somewhere. At least this puts it somewhere balanced in a
        // controlled manner.
        Move(0, 0);

        // Tell the driver to turn off the HV bias.
        Set_HV_Driver(false);

        // Disconnect
        RS232_CloseComport(comport_Number);
        // Add a sleep here? Serial seems to fall over when disconnecting?
        //MySleep(2000);
        return 0;
    }
}

int MirrorLoopTest(){
    LD_MemsMirror::MirrorOptions my_Options;
    my_Options.comport_Name = "ttyACM0";
    my_Options.limit = 0.95;

    LD_MemsMirror::Mirror my_Mirror(my_Options);

    std::cin.get();
    my_Mirror.Move(0.5, 0.5);
    std::cin.get();

    my_Mirror.Close();

    return 0;
}

int MirrorInitTest(){
    LD_MemsMirror::MirrorOptions my_Options;
    my_Options.comport_Name = "ttyACM0";
    my_Options.limit = 0.95;

    LD_MemsMirror::Mirror my_Mirror(my_Options);

    bool hv_Status = false;
    while(std::cin.get() != 'x'){
        my_Mirror.Set_HV_Driver(hv_Status);
        hv_Status = !hv_Status;
    }

    my_Mirror.Close();

    return 0;
}

int MirrorWiggle(){
    LD_MemsMirror::MirrorOptions my_Options;
    my_Options.comport_Name = "ttyACM0";
    my_Options.limit = 0.95;

    float increment = 0.01;
    int sleep_Time = 10000;

    LD_MemsMirror::Mirror my_Mirror(my_Options);

    my_Mirror.Set_HV_Driver(false);

    std::cout << "Press return to turn on driver and start pattern" << std::endl;
    std::cin.get();
    my_Mirror.Set_HV_Driver(true);

    int moves = 0;
    float i = 0;
    float range = 0.25;
    bool rising = true;
    do{
        my_Mirror.Move(i, i);
        if (rising){
            i += increment;
            if (i > range){
                rising = false;
            }
        }
        else{
            i-=increment;
            if (i < -range){
                rising = true;
            }
        }
        usleep(sleep_Time);
        std::cout << "Sleep" << std::endl;

    } while (true);

    std::cin.get();

    std::cout << moves << " moves" << std::endl;
}

int MirrorRandom(){
    LD_MemsMirror::MirrorOptions my_Options;
    my_Options.comport_Name = "ttyACM0";
    my_Options.limit = 0.80;

    int sleep_Time = 500;

    LD_MemsMirror::Mirror my_Mirror(my_Options);

    std::cin.get();

    float x = 0;
    float y = 0;
    std::default_random_engine generator;
    std::uniform_real_distribution<float> distribution(-0.01, 0.01);

    while(true){
        my_Mirror.Move(x, y);

        x += distribution(generator);
        y += distribution(generator);

        if (x > my_Options.limit){x = my_Options.limit;}
        if (x < -my_Options.limit){x = -my_Options.limit;}
        if (y > my_Options.limit){y = my_Options.limit;}
        if (y < -my_Options.limit){y = -my_Options.limit;}
        usleep(sleep_Time);
    }
    return 0;
}
