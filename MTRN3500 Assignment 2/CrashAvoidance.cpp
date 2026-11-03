#include "CrashAvoidance.h"

CrashAvoidance::CrashAvoidance(SM_ThreadManagement^ sm_tm, SM_Laser^ sm_laser, SM_VehicleControl^ sm_vehiclecontrol)
{
	SM_TM_ = sm_tm;
	SM_Laser_ = sm_laser;
	SM_VehicleControl_ = sm_vehiclecontrol;
}

error_state CrashAvoidance::processSharedMemory()
{
	return SUCCESS;
}

bool CrashAvoidance::getShutdownFlag()
{
	return (SM_TM_->shutdown & bit_CRASHAVOIDANCE) != 0;
}

void CrashAvoidance::threadFunction()
{
	Console::WriteLine("Crash Avoidance thread is starting");

	SM_TM_->ThreadBarrier->SignalAndWait();

	while (!getShutdownFlag())
	{
		Monitor::Enter(SM_TM_->lockObject);
		SM_TM_->heartbeat |= bit_CRASHAVOIDANCE;
		Monitor::Exit(SM_TM_->lockObject);

		error_state result = processSharedMemory();

		if (result != SUCCESS)
		{
			printError(result);
		}

		Thread::Sleep(20);
	}

	Console::WriteLine("Crash Avoidance thread is shutting down");
}