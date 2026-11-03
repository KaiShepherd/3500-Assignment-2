#include "Controller.h"

Controller::Controller(SM_ThreadManagement^ sm_tm, SM_VehicleControl^ sm_vehiclecontrol)
{
	SM_TM_ = sm_tm;
	SM_VehicleControl_ = sm_vehiclecontrol;
	controller_ = new ControllerInterface(1, INPUT_MODE);
}


Controller::~Controller()
{
	delete controller_;
}


error_state Controller::processSharedMemory()
{
	return SUCCESS;
}


bool Controller::getShutdownFlag()
{
	return (SM_TM_->shutdown & bit_CONTROLLER) != 0;
}


void Controller::threadFunction()
{
	Console::WriteLine("Controller thread is starting");

	SM_TM_->ThreadBarrier->SignalAndWait();

	while (!getShutdownFlag())
	{
		Monitor::Enter(SM_TM_->lockObject);
		SM_TM_->heartbeat |= bit_CONTROLLER;
		Monitor::Exit(SM_TM_->lockObject);

		error_state result = processSharedMemory();
		
		if (result != SUCCESS)
		{
			printError(result);
		}

		Thread::Sleep(20);
	}

	Console::WriteLine("Controller thread shutting down");
}