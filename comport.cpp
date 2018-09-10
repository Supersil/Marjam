#include "comport.h"
#include <Windows.h>

#define sleep(x) Sleep(x)

#ifdef Q_OS_WIN

#define writeLog(x,...) 

COMPort::COMPort()
{
	memset(&ov,0,sizeof(ov));
	hFD = INVALID_HANDLE_VALUE;
}

COMPort::~COMPort()
{
	CloseDevice();
}

bool COMPort::OpenDevice(QString devname)
{
	DCB dcb;
	bool fSuccess;

	CloseDevice();

	hFD = CreateFileA(devname.toLocal8Bit().data(),	// device name
		GENERIC_READ | GENERIC_WRITE,	// O_RDWR
		0,		// not shared
		NULL,	// default value for object security ?!?
		OPEN_EXISTING, // file (device) exists
		FILE_FLAG_OVERLAPPED,	// asynchron handling
		NULL); // no more handle flags
	//
	if(hFD == INVALID_HANDLE_VALUE) {
		return false;
	}
	// create event for overlapped I/O
	// we need a event object, which inform us about the
	// end of an operation (here reading device)
	ov.hEvent = CreateEvent(NULL,// LPSECURITY_ATTRIBUTES lpsa
		false, // bool fManualReset
		false, // bool fInitialState
		NULL); // LPTSTR lpszEventName
	//
	if(ov.hEvent == INVALID_HANDLE_VALUE) {
		return false;
	}

	// Build on the current configuration, and skip setting the size
	// of the input and output buffers with SetupComm.
	SecureZeroMemory(&dcb, sizeof(DCB));
	dcb.DCBlength = sizeof(DCB);
	fSuccess = GetCommState(hFD, &dcb);

	if (!fSuccess) 
	{
		// Handle the error.
		writeLog(QString().sprintf("GetCommState failed with error %d.\n", GetLastError()));
		return false;
	}

	dcb.BaudRate = CBR_9600;		 // set the baud rate
	dcb.ByteSize = 8;						 // data size, xmit, and rcv
	dcb.Parity = NOPARITY;				// no parity bit
	dcb.StopBits = ONESTOPBIT;		// one stop bit
	dcb.fDtrControl = DTR_CONTROL_ENABLE;
	dcb.fRtsControl = RTS_CONTROL_ENABLE;

	fSuccess = SetCommState(hFD, &dcb);

	if (!fSuccess) 
	{
		// Handle the error.
		writeLog(QString().sprintf("SetCommState failed with error %d.\n", GetLastError()));
		return false;
	}

	//
	//* THIS IS OBSOLETE!!!
	// event should be triggered, if there are some received data
	if(!SetCommMask(hFD,EV_BREAK | EV_RXCHAR))
	return false;
	//*/

	COMMTIMEOUTS cto = {MAXDWORD,0,0,0,0};
	if(!::SetCommTimeouts(hFD,&cto))
	 return false;

	// for a better performance with win95/98 I increased the internal
	// buffer to wxSERIALPORT_BUFSIZE (normal size is 1024, but this can
	// be a little bit to small, if you use a higher baudrate like 115200)
	if(!SetupComm(hFD,1024,1024))
	 return false;

	//PurgeComm(hFD, PURGE_RXCLEAR); 
	//PurgeComm(hFD, PURGE_TXCLEAR);

	return true;
}

bool COMPort::CloseDevice()
{
	if(hFD != INVALID_HANDLE_VALUE)
	{
		//PurgeComm(hFD, PURGE_RXCLEAR); 
		//PurgeComm(hFD, PURGE_TXCLEAR);

		CloseHandle(hFD);
		CloseHandle(ov.hEvent);
		hFD = INVALID_HANDLE_VALUE;
	}
	return true;
}


int COMPort::ReadPacketBS(char* buf,int &len,std::shared_ptr<bool> pbBreakRead)
{
	COMSTAT		csStat;
	DWORD dwEvtMask, dwError, dwBytes;
	int iRetLen = 0;
	DWORD i,iInQue;
	
	writeLog("ReadPacket..");
	
	ClearCommBreak(hFD);
	
	PurgeComm(hFD, PURGE_RXABORT | PURGE_TXABORT);
	PurgeComm(hFD, PURGE_RXCLEAR | PURGE_TXCLEAR); 
	
	while(true)
	{
		ResetEvent(ov.hEvent);

		if(pbBreakRead.get() && (*pbBreakRead))
		{
			len = iRetLen;
			break;
		}

		dwEvtMask = 0;

		if (!WaitCommEvent(hFD, &dwEvtMask, &ov))
		{
			dwError = ::GetLastError();
			writeLog(QString().sprintf("\n WaitCommEvent dwError = 0x%08X; dwEvtMask = 0x%08X",dwError,dwEvtMask));
			sleep(100);
		}
		else
			writeLog("\nWaitCommEvent - true");

		if (dwEvtMask & EV_BREAK)
		{
			len = 0;
			writeLog("\ndwEvtMask & EV_BREAK");
			break;
		}
		else 
			if (dwEvtMask & EV_RXCHAR)
			{
				writeLog(QString().sprintf("\ndwEvtMask = 0x%08X",dwEvtMask));
				dwError = 0;
				csStat.cbInQue = 0;
				ClearCommError(hFD,&dwError, &csStat);
				iInQue = csStat.cbInQue;
				writeLog(QString().sprintf("\ncsStat.cbInQue = %d",iInQue));
				if(iInQue == 0)//(csStat.cbInQue == 0)
				{
					writeLog("\n(csStat.cbInQue == 0)");
					continue;
				}

				if(iInQue > len)//(csStat.cbInQue > len)
				{
					writeLog(QString().sprintf("\ncsStat.cbInQue > len (%d,%d)",iInQue,len));
					len = iInQue;//csStat.cbInQue;
					break;
				}

				ResetEvent(ov.hEvent);

				dwBytes = (DWORD)len;

				if (!ReadFile(hFD, buf, iInQue, &dwBytes, &ov))
				{
					DWORD err = GetLastError();
					if(err != ERROR_IO_PENDING )
					{
						len = 0;
						writeLog("\n!ReadFile");
						break;
					}
				}
				DWORD err = GetLastError();
				if (err == ERROR_IO_PENDING)
					dwBytes = iInQue;
				
				for (i = 0; i < dwBytes; i++)
				{
					if(buf[i]==0x0D)
					{
						buf[i] = 0;
						iRetLen = len = (int)i;
						break;
					}
				}
				
				if(i >= dwBytes)//0x0D not found
				{
					len = 0;
					writeLog(QString().sprintf("\ni >= csStat.cbInQue (%d)",i));
				}

				break;

			}				
	}

	return iRetLen;
}

#else

COMPort::COMPort()
{
	iFD = 0;
}


COMPort::~COMPort()
{
	CloseDevice();
}


bool COMPort::OpenDevice(QString devname)
{
	bool bSuccess;
	
	CloseDevice();
	QString COMPath("/dev/");
	iFD = open(COMPath.append(devname).toLocal8Bit().data(), O_RDWR | O_NOCTTY);
	
	if (iFD < 0)
		return false;

	struct termios settings;
	memset(&settings, 0, sizeof(struct termios));

	if (tcgetattr (iFD, &settings) != 0)
		return false;

	cfsetispeed(&settings, B9600);
	cfsetospeed(&settings, B9600);


	settings.c_cflag = (settings.c_cflag & ~CSIZE) | CS8;		 // 8-bit chars
	// disable IGNBRK for mismatched speed tests; otherwise receive break
	// as \000 chars
	settings.c_iflag &= ~IGNBRK;				 // disable break processing
	settings.c_lflag = 0;								// no signaling chars, no echo,
	// no canonical processing
	settings.c_oflag = 0;								// no remapping, no delays
	settings.c_cc[VMIN]	= 0;						// read doesn't block
	settings.c_cc[VTIME] = 5;						// 0.5 seconds read timeout

	settings.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

	settings.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
	// enable reading
	settings.c_cflag &= ~(PARENB | PARODD);			// shut off parity
	settings.c_cflag |= 0;
	settings.c_cflag &= ~CSTOPB;
	settings.c_cflag &= ~CRTSCTS;

	if (tcsetattr (iFD, TCSANOW, &settings) != 0)
		return false;


	return true;
	
}


bool COMPort::CloseDevice()
{
	int iRet;
	if (iFD)
	{
		iRet = close(iFD);
		iFD = 0;
		if (iRet != 0)
			return false;
	}
	return true;	
}


int COMPort::ReadPacketBS(char *buf, int &len, std::shared_ptr<bool> breakRead)
{
	if (iFD <= 0)
		return 0;

	int iRetLen = 0;
	sleep(2); //required to make flush work, for some reason
	tcflush(iFD,TCIOFLUSH);

	while(true)
	{
		if (*breakRead)
		{
			len = iRetLen;
			break;
		}

		sleep(10);

		int iInQue = len;
		iInQue = read(iFD,buf,iInQue);

		if (iInQue < 0)
			return -1;

		if (iInQue == 0)
			continue;

		if (iInQue > len)
		{
			len = iInQue;
			break;
		}

		len = iInQue;

		int i, size = len;
		for(i = 0; i < len; i++)
		{
			if (buf[i] == 0x0D)
			{
				buf[i] = 0;
				iRetLen = len = i;
				break;
			}
		}
		if (i >= size)
		{
			len = 0;

		}
		break;

	}
	
	return iRetLen;
}


//int COMPort::ReadPacket(char *buf, int &len)
//{

//	if (iFD <= 0)
//		return 0;

//	int i;
//	int rb = 0; 
//	for(i = 0; i < len; i++)
//	{
//		sleep(50);
//		if((rb = read(iFD,buf+i,1) ) == -1)
//		{
//			len = i;
//			break;
//		}
//		else
//		{
//			if ((buf[i] == 0x05) || (buf[i]==0x03) ||
//				 ((i>0) && (buf[i] == 0)))
//			{
//				len = i+1;
//				break;
//			}
//			// 0 - passes
//			if (i == 0)
//				if (buf[0] == 0)
//					i--;
//		}
//	}
	
//	return len;
//}


//int COMPort::SendPacket(char *buf, int len)
//{
//	int iWrite = 0;
	
//	iWrite = write(iFD,buf,len);
//	sleep(2); //required to make flush work, for some reason
//	tcflush(iFD,TCIOFLUSH);

//	return iWrite;
//}

#endif
