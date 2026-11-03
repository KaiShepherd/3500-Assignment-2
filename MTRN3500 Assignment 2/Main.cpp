#include "TMM.h"

int main(void)
{
    ThreadManagement^ tmm = gcnew ThreadManagement();

    // Create all shared memory; stop if it fails
    error_state result = tmm->setupSharedMemory();
    if (result != SUCCESS)
    {
        UGVModule::printError(result);
        Console::ReadKey();
        return -1;
    }

    // Starts all module threads and runs until shutdown
    tmm->threadFunction();

    Console::WriteLine("Program finished. Press any key to exit.");
    Console::ReadKey();
    return 0;
}