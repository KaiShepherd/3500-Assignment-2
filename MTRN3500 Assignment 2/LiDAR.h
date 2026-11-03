#pragma once
//inherits from NetworkedModule

#include <NetworkedModule.h>

constexpr int LIDAR_PORT = 23000;

ref class LiDAR : public NetworkedModule
{
public:
    LiDAR(SM_ThreadManagement^ sm_tm, SM_Laser^ sm_laser);

    error_state connect(String^ hostName, int portNumber) override;
    error_state communicate() override;
    error_state processSharedMemory() override;
    bool getShutdownFlag() override;
    void threadFunction() override;

private:
    SM_Laser^ SM_Laser_;
    array<double>^ x_;
    array<double>^ y_;
};