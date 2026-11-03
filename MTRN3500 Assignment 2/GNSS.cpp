#include "GNSS.h"

GNSS::GNSS(SM_ThreadManagement^ sm_tm, SM_GPS^ sm_gps)
{
	SM_TM_ = sm_tm;
	SM_GPS_ = sm_gps;
}

error_state GNSS::connect(String^ hostName, int portNumber)
{
	return SUCCESS;
}

error_state GNSS::communicate()
{
	return SUCCESS;
}

error_state GNSS::processSharedMemory()
{
	return SUCCESS;
}

bool GNSS::getShutdownFlag()
{
	return (SM_TM_->shutdown & bit_GPS) != 0;
}

void GNSS::threadFunction()
{
	Console::WriteLine("GNSS thread is starting");

	if (connect(WEEDER_ADDRESS, GNSS_PORT) != SUCCESS)
	{
		Console::WriteLine("Connection error");
	}

	if (!skipBarrier)
	{
		SM_TM_->ThreadBarrier->SignalAndWait();
	}

	while (!getShutdownFlag())
	{
		Monitor::Enter(SM_TM_->lockObject);
		SM_TM_->heartbeat |= bit_GPS;
		Monitor::Exit(SM_TM_->lockObject);

		//module functionality to be implemented

		error_state result = communicate();

		if (result == SUCCESS)
		{
			result = processSharedMemory();
		}
		if (result != SUCCESS)
		{
			printError(result);
		}

		Thread::Sleep(100);
	}

	Console::WriteLine("GNSS thread shutting down");
}