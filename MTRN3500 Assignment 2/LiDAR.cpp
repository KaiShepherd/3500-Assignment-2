#include "LiDAR.h"

LiDAR::LiDAR(SM_ThreadManagement^ sm_tm, SM_Laser^ sm_laser)
{
	SM_TM_ = sm_tm;
	SM_Laser_ = sm_laser;
    x_ = gcnew array<double>(STANDARD_LASER_LENGTH);
    y_ = gcnew array<double>(STANDARD_LASER_LENGTH);
}

error_state LiDAR::connect(String^ hostName, int portNumber)
{
    try
    {
        // Open the TCP connection and configure it
        Client = gcnew TcpClient(hostName, portNumber);
        Client->NoDelay = true;
        Client->ReceiveTimeout = 500;
        Client->SendTimeout = 500;
        Client->ReceiveBufferSize = 4096;
        Client->SendBufferSize = 1024;

        Stream = Client->GetStream();
        ReadData = gcnew array<unsigned char>(2500);

        // Authenticate
        array<unsigned char>^ authData = Encoding::ASCII->GetBytes("5476861\n");
        Stream->Write(authData, 0, authData->Length);

        // Read and check the server's reply
        int bytesRead = Stream->Read(ReadData, 0, ReadData->Length);
        String^ response = Encoding::ASCII->GetString(ReadData, 0, bytesRead);

        if (response->Trim() != "OK")
        {
            Console::WriteLine("LiDAR authentication failed, server replied: " + response);
            return ERR_CONNECTION;
        }

        Console::WriteLine("LiDAR connected and authenticated");
    }
    catch (SocketException^ e)
    {
        Console::WriteLine("LiDAR connection failed: " + e->Message);
        return ERR_CONNECTION;
    }
    catch (System::IO::IOException^ e)
    {
        Console::WriteLine("LiDAR authentication timed out: " + e->Message);
        return ERR_CONNECTION;
    }

    return SUCCESS;
}

error_state LiDAR::communicate()
{
    try
    {
        // Send scan request: <STX>sRN LMDscandata<ETX>
        array<unsigned char>^ body = Encoding::ASCII->GetBytes("sRN LMDscandata");
        array<unsigned char>^ request = gcnew array<unsigned char>(body->Length + 2);
        request[0] = 0x02;
        Array::Copy(body, 0, request, 1, body->Length);
        request[request->Length - 1] = 0x03;
        Stream->Write(request, 0, request->Length);

        // Read until the end byte (0x03) arrives, the reply may come in pieces
        int total = 0;
        do
        {
            int n = Stream->Read(ReadData, total, ReadData->Length - total);
            if (n == 0)
            {
                return ERR_CONNECTION;
            }
            total += n;
        } while (ReadData[total - 1] != 0x03 && total < ReadData->Length);

        if (ReadData[total - 1] != 0x03)
        {
            return ERR_INVALID_DATA;
        }

        // Split into fields, strip STX/ETX first
        String^ reply = Encoding::ASCII->GetString(ReadData, 0, total);
        reply = reply->Trim((wchar_t)0x02, (wchar_t)0x03);
        array<String^>^ fields = reply->Split(' ');

        // Validate the header
        if (fields->Length < 2 || (fields[0] != "sRA" && fields[0] != "sRN") || fields[1] != "LMDscandata")
        {
            return ERR_INVALID_DATA;
        }

        int idx = Array::IndexOf(fields, "DIST1");
        if (idx < 0 || fields->Length < idx + 6)
        {
            return ERR_INVALID_DATA;
        }

        // Read scan parameters (all hex)
        UInt32 scaleBits = Convert::ToUInt32(fields[idx + 1], 16);
        double scale = BitConverter::ToSingle(BitConverter::GetBytes(scaleBits), 0);  // IEEE float, usually 1.0
        double startAngle = Convert::ToInt32(fields[idx + 3], 16) / 10000.0;  // degrees
        double stepAngle = Convert::ToInt32(fields[idx + 4], 16) / 10000.0;  // degrees
        int numPoints = Convert::ToInt32(fields[idx + 5], 16);

        if (numPoints != STANDARD_LASER_LENGTH || fields->Length < idx + 6 + numPoints)
        {
            return ERR_INVALID_DATA;
        }

        // Convert to x-y coordinates
        for (int i = 0; i < numPoints; i++)
        {
            double range = Convert::ToInt32(fields[idx + 6 + i], 16) * scale / 1000.0;  // mm -> m
            double angle = (startAngle + i * stepAngle) * Math::PI / 180.0;  // radians

            x_[i] = range * Math::Sin(angle);
            y_[i] = -range * Math::Cos(angle);
        }

        // print
        for (int i = 0; i < numPoints; i++)
        {
            Console::WriteLine(String::Format("{0,3}: x = {1,7:F3} m, y = {2,7:F3} m", i, x_[i], y_[i]));
        }
    }
    catch (System::IO::IOException^)
    {
        return ERR_NO_DATA;
    }
    catch (FormatException^)
    {
        return ERR_INVALID_DATA;
    }
    catch (OverflowException^)
    {
        return ERR_INVALID_DATA;
    }

    return SUCCESS;
}

error_state LiDAR::processSharedMemory()
{
    Monitor::Enter(SM_Laser_->lockObject);
    try
    {
        Array::Copy(x_, SM_Laser_->x, STANDARD_LASER_LENGTH);
        Array::Copy(y_, SM_Laser_->y, STANDARD_LASER_LENGTH);
    }
    finally
    {
        Monitor::Exit(SM_Laser_->lockObject);
    }
    return SUCCESS;
}

bool LiDAR::getShutdownFlag()
{
	return (SM_TM_->shutdown & bit_LASER) != 0;
}

void LiDAR::threadFunction()
{
	Console::WriteLine("LiDAR thread starting");

	if (connect(WEEDER_ADDRESS, LIDAR_PORT) != SUCCESS)
	{
		Console::WriteLine("Communication error");
	}

	SM_TM_->ThreadBarrier->SignalAndWait();

	while (!getShutdownFlag())
	{
		Monitor::Enter(SM_TM_->lockObject);
		SM_TM_->heartbeat |= bit_LASER;
		Monitor::Exit(SM_TM_->lockObject);

		// module functionality to be implemented

		error_state result = communicate();

		if (result == SUCCESS)
		{
			result = processSharedMemory();
		}
		if (result != SUCCESS)
		{
			printError(result);
		}

		Thread::Sleep(1000);
	}

	Console::WriteLine("LiDAR thread is shutting down");
}