#pragma once
//inherits from UGVModule

#include <ControllerInterface.h>
#include <UGVModule.h>

#define XBOX_INPUT 0
#define KEYBOARD_INPUT 1
#define INPUT_MODE KEYBOARD_INPUT //change to XBOX_INPUT for the lab

ref class Controller : public UGVModule
{
public:
	Controller(SM_ThreadManagement^ sm_tm, SM_VehicleControl^ sm_vehiclecontrol);
	~Controller();
	error_state processSharedMemory() override;
	bool getShutdownFlag() override;
	void threadFunction() override;

private:
	SM_VehicleControl^ SM_VehicleControl_;
	ControllerInterface* controller_;
};
