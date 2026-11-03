#pragma once
//inherits form NetworkedModule

#include <NetworkedModule.h>

constexpr int VC_PORT = 25000;

ref class VehicleControl : public NetworkedModule
{
public:
    VehicleControl(SM_ThreadManagement^ sm_tm, SM_VehicleControl^ sm_vehiclecontrol);

    error_state connect(String^ hostName, int portNumber) override;
    error_state communicate() override;
    error_state processSharedMemory() override;
    bool getShutdownFlag() override;
    void threadFunction() override;

private:
    SM_VehicleControl^ SM_VehicleControl_;
};