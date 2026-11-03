#pragma once

#include <ControllerInterface.h>
#include <UGVModule.h>
#include "Controller.h"
#include "CrashAvoidance.h"
#include "Display.h"
#include "GNSS.h"
#include "LiDAR.h"
#include "VC.h"

constexpr int NUM_UNITS = 6;
constexpr int TIMEOUT_MS = 2000;

ref class ThreadManagement : public UGVModule {
public:
    // Create shared memory objects
    error_state setupSharedMemory();

    // Send/Recieve data from shared memory structures
    error_state processSharedMemory() override;

    // Shutdown all modules in the software
    void shutdownModules();

    // Get Shutdown signal for module, from Thread Management SM
    bool getShutdownFlag() override;

    // Thread function for TMM
    void threadFunction() override;



private:
    // Add any additional data members or helper functions here
    SM_Laser^ SM_Laser_;
    SM_GPS^ SM_GPS_;
    SM_VehicleControl^ SM_VehicleControl_;
    array<Thread^>^ ThreadList;
    array<uint8_t>^ BitList;
    void restartThread(int index);
    array<String^>^ NameList;
};
