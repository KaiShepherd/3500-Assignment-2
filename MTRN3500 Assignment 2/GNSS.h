#pragma once
//inherits from NetworkedModule

#include <NetworkedModule.h>

constexpr int GNSS_PORT = 24000;

ref class GNSS : public NetworkedModule
{
public:
    GNSS(SM_ThreadManagement^ sm_tm, SM_GPS^ sm_gps);

    error_state connect(String^ hostName, int portNumber) override;
    error_state communicate() override;
    error_state processSharedMemory() override;
    bool getShutdownFlag() override;
    void threadFunction() override;
    bool skipBarrier = false;

private:
    SM_GPS^ SM_GPS_;
};