#ifndef LD_MEMSMIRROR_H
#define LD_MEMSMIRROR_H

#include <string>
#include <vector>

namespace LD_MemsMirror{
    int ClipValue(float &value, float limit_hi, float limit_lo);
    int ClipValue(float &value);

    struct MirrorOptions{
        std::string comport_Name;
        float limit = 0.95;
    };

    class Mirror
    {
        public:
            Mirror();
            Mirror(MirrorOptions my_Options);
            ~Mirror();

            int Init(MirrorOptions my_Options);
            int Set_HV_Driver(bool hv_On);
            int Move(float x, float y);
            int Close();

        private:
            int comport_Number;
            float limit;

            bool is_Initted = false;

            uint16_t mems_X;
            uint16_t mems_X_Current;
            uint16_t mems_Y;
            uint16_t mems_Y_Current;

            std::vector<uint8_t> outBuffer = std::vector<uint8_t>(5,0);
            std::vector<char> inBuffer = std::vector<char>(5,0);

            int SendCOM(std::vector<uint8_t> message);
            std::vector<char> RecvCOM(int num_Bytes=0);

    };
} // namespace LD_MEMSMIRROR_H

int MirrorLoopTest();
int MirrorInitTest();
int MirrorWiggle();
int MirrorRandom();

#endif // LD_MEMSMIRROR_H
