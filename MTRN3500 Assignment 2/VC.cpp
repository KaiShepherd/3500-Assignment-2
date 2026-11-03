#include "VC.h"

VehicleControl::VehicleControl(SM_ThreadManagement^ sm_tm, SM_VehicleControl^ sm_vehiclecontrol)
{
	SM_TM_ = sm_tm;
	SM_VehicleControl_ = sm_vehiclecontrol;
}

error_state VehicleControl::connect(String^ hostName, int portNumber)
{
	return SUCCESS;
}

error_state VehicleControl::communicate()
{
	return SUCCESS;
}

error_state VehicleControl::processSharedMemory()
{
	return SUCCESS; 
}

bool VehicleControl::getShutdownFlag()
{
	return (SM_TM_->shutdown & bit_VC) != 0;
}

void VehicleControl::threadFunction()
{
	Console::WriteLine("Vehicle Control thread is starting");

	if (connect(WEEDER_ADDRESS, VC_PORT) != SUCCESS)
	{
		Console::WriteLine("Connection error");
	}

	SM_TM_->ThreadBarrier->SignalAndWait();

	while (!getShutdownFlag())
	{
		Monitor::Enter(SM_TM_->lockObject);
		SM_TM_->heartbeat |= bit_VC;
		Monitor::Exit(SM_TM_->lockObject);

		//module work to be completed later

		error_state result = processSharedMemory();

		if (result == SUCCESS)
		{
			result = communicate();
		}
		if (result != SUCCESS)
		{
			printError(result);
		}
		Thread::Sleep(100);
	}

	Console::WriteLine("Vehicle Control thread is shutting down");
}