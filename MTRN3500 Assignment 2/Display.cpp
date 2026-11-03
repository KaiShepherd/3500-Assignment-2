#include "Display.h"

Display::Display(SM_ThreadManagement^ sm_tm, SM_Laser^ sm_laser)
{
	SM_TM_ = sm_tm;
	SM_Laser_ = sm_laser;

}

error_state Display::connect(String^ hostName, int portNumber)
{
	return SUCCESS;
}

error_state Display::communicate()
{
	return SUCCESS;
}
error_state Display::processSharedMemory()
{
	return SUCCESS;
}
bool Display::getShutdownFlag()
{
	return (SM_TM_->shutdown & bit_DISPLAY) != 0;
}
void Display::threadFunction()
{
	Console::WriteLine("Display thread is starting");

	if (connect(DISPLAY_ADDRESS, DISPLAY_PORT) != SUCCESS)
	{
		Console::WriteLine("Connection error");
	}

	SM_TM_->ThreadBarrier->SignalAndWait();

	while (!getShutdownFlag())
	{
		Monitor::Enter(SM_TM_->lockObject);
		SM_TM_->heartbeat |= bit_DISPLAY;
		Monitor::Exit(SM_TM_->lockObject);

		// thread functionality to be implemented

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
	Console::WriteLine("Display thread is shutting down");
}