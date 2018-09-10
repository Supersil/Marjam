#include "textlog.h"

#include <QDateTime>

QFile textLog::logFile;
QMutex textLog::logMutex;
QString textLog::strFilePath;
int textLog::objCount = 0;

textLog::textLog(QString strPath)
{
	if (!logFile.isOpen())
	{
		if (strPath.size() == 0)
			strFilePath = QString("ArduServer_").append(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss").append(".log"));
		else
			strFilePath = strPath;
	}
	objCount++;
}


textLog::~textLog()
{
	objCount--;
	
	if (!objCount)
	{
		if (logFile.isOpen())
		{
			logMutex.lock();
			logFile.flush();
			logFile.close();
			logMutex.unlock();
		}
	}
}


bool textLog::create()
{
	if ((strFilePath.size() == 0) || (logFile.isOpen()))
		return false;
	
	logFile.setFileName(strFilePath);
	
	return logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
}


void textLog::addLine(QString strLine, bool writeTimeStamp)
{
	if (!logFile.isOpen())
		return;
	
	if (writeTimeStamp)
		logFile.write(QTime::currentTime().toString("hh:mm:ss:zzz ").toLocal8Bit());
	
	logFile.write(strLine.toLocal8Bit());
	logFile.write("\n");
	logFile.flush();
	
}


void textLog::setFileName(QString strPath)
{
	if (logFile.isOpen())
		logFile.close();
	if (strPath.size() == 0)
		strFilePath = QString(DEF_FILE_NAME).append(QDateTime::currentDateTime().toString("_yyyyMMdd_hhmmss").append(".log"));
	else
		strFilePath = strPath;
}







