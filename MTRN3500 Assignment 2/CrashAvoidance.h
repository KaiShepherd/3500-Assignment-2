#pragma once
//inherits from UGVModule

#include <UGVModule.h>

ref class CrashAvoidance : public UGVModule
{
public:
	CrashAvoidance(SM_ThreadManagement^ sm_tm, SM_Laser^ sm_laser, SM_VehicleControl^ sm_vehiclecontrol);
	error_state processSharedMemory() override;
	bool getShutdownFlag() override;
	void threadFunction() override;

private:
	SM_Laser^ SM_Laser_;
	SM_VehicleControl^ SM_VehicleControl_;
};