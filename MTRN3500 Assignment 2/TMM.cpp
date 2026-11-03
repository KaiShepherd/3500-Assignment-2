#include "TMM.h"

error_state ThreadManagement::setupSharedMemory()
{
    try {
        SM_TM_ = gcnew SM_ThreadManagement;
        SM_Laser_ = gcnew SM_Laser;
        SM_GPS_ = gcnew SM_GPS;
        SM_VehicleControl_ = gcnew SM_VehicleControl;

        SM_TM_->WatchList = gcnew array<Stopwatch^>(NUM_UNITS);
        for (int i = 0; i < NUM_UNITS; i++)
        {
            SM_TM_->WatchList[i] = gcnew Stopwatch;
        }

        SM_TM_->ThreadBarrier = gcnew Barrier(NUM_UNITS + 1);
    }
    catch (Exception^ e)
    {
        Console::WriteLine("Shared memory setup failed: " + e->Message);
        return ERR_SM;
    }

    return SUCCESS;
}

error_state ThreadManagement::processSharedMemory()
{
    int restartIndex = -1;
    bool criticalFailure = false;

    Monitor::Enter(SM_TM_->lockObject);
    try
    {
        // Tell the other modules TMM is alive
        SM_TM_->heartbeat |= bit_TM;

        for (int i = 0; i < NUM_UNITS; i++)
        {
            if (SM_TM_->heartbeat & BitList[i])
            {
                // Module has checked in: clear its bit and reset its timer
                SM_TM_->heartbeat &= ~BitList[i];
                SM_TM_->WatchList[i]->Restart();
            }
            else if (SM_TM_->WatchList[i]->ElapsedMilliseconds > TIMEOUT_MS)
            {
                if (BitList[i] & CRITICALMASK)
                {
                    // Critical thread has died: shut everything down
                    Console::WriteLine("Critical thread " + i + " failed, shutting down");
                    SM_TM_->shutdown = bit_ALL;
                    criticalFailure = true;
                    break;
                }
                else
                {
                    // Non-critical thread has died: restart it after unlocking
                    restartIndex = i;
                }
            }
        }
    }
    finally
    {
        Monitor::Exit(SM_TM_->lockObject);
    }

    if (criticalFailure)
    {
        return ERR_CRITICAL_FAILURE;
    }

    if (restartIndex != -1)
    {
        restartThread(restartIndex);
    }

    return SUCCESS;
}

void ThreadManagement::shutdownModules()
{
	Monitor::Enter(SM_TM_->lockObject);
	SM_TM_->shutdown = bit_ALL;
	Monitor::Exit(SM_TM_->lockObject);
}

bool ThreadManagement::getShutdownFlag()
{
	return (SM_TM_->shutdown & bit_TM) != 0;
}

void ThreadManagement::threadFunction()
{
	Console::WriteLine("Thread management thread is starting");

	ThreadList = gcnew array<Thread^>(NUM_UNITS);
	BitList = gcnew array<uint8_t>{ bit_LASER, bit_GPS, bit_VC, bit_DISPLAY, bit_CONTROLLER, bit_CRASHAVOIDANCE };
    NameList = gcnew array<String^>{ "LiDAR", "GNSS", "Vehicle Control", "Display", "Controller", "Crash Avoidance" };
	ThreadList[0] = gcnew Thread(gcnew ThreadStart(gcnew LiDAR(SM_TM_, SM_Laser_), &LiDAR::threadFunction));
	ThreadList[1] = gcnew Thread(gcnew ThreadStart(gcnew GNSS(SM_TM_, SM_GPS_), &GNSS::threadFunction));
	ThreadList[2] = gcnew Thread(gcnew ThreadStart(gcnew VehicleControl(SM_TM_, SM_VehicleControl_), &VehicleControl::threadFunction));
	ThreadList[3] = gcnew Thread(gcnew ThreadStart(gcnew Display(SM_TM_, SM_Laser_), &Display::threadFunction));
	ThreadList[4] = gcnew Thread(gcnew ThreadStart(gcnew Controller(SM_TM_, SM_VehicleControl_), &Controller::threadFunction));
	ThreadList[5] = gcnew Thread(gcnew ThreadStart(gcnew CrashAvoidance(SM_TM_, SM_Laser_, SM_VehicleControl_), &CrashAvoidance::threadFunction));

    for (int i = 0; i < NUM_UNITS; i++)
    {
        Console::WriteLine("TMM: starting thread " + NameList[i]);
        ThreadList[i]->Start();
    }

    SM_TM_->ThreadBarrier->SignalAndWait();

    for (int i = 0; i < NUM_UNITS; i++)
    {
        SM_TM_->WatchList[i]->Start();
    }

	while (!getShutdownFlag())
	{
		if (Console::KeyAvailable && Console::ReadKey(true).KeyChar == 'q')
		{
			break;
		}

        error_state result = processSharedMemory();
        if (result != SUCCESS)
        {
            printError(result);
        }

		Thread::Sleep(20);
	}

	shutdownModules();

	for (int i = 0; i < NUM_UNITS; i++)
	{
		ThreadList[i]->Join();
	}

	Console::WriteLine("Thread management thread is shutting down");
}

void ThreadManagement::restartThread(int index)
{
    // GNSS is the only non-critical thread
    if (BitList[index] == bit_GPS)
    {
        Console::WriteLine("GNSS thread failed, restarting");

        GNSS^ gnss = gcnew GNSS(SM_TM_, SM_GPS_);
        gnss->skipBarrier = true;   // barrier has already been passed by everyone else

        ThreadList[index] = gcnew Thread(gcnew ThreadStart(gnss, &GNSS::threadFunction));
        ThreadList[index]->Start();
        SM_TM_->WatchList[index]->Restart();
    }
}