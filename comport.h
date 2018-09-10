#ifndef COMPORT_H
#define COMPORT_H

#include <QString>
#include <memory>

#ifdef Q_OS_WIN

#include <Windows.h>

#else

#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

#endif

class COMPort
{
public:
	COMPort();
	~COMPort();
	bool OpenDevice(QString devname);
	bool CloseDevice();
	int ReadPacketBS(char * buf, int &len, std::shared_ptr<bool> breakRead);

private:
#ifdef Q_OS_WIN
	HANDLE hFD;
	OVERLAPPED ov;
#else
	int iFD;
#endif
};

#endif // COMPORT_H
