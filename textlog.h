#ifndef TEXTLOG_H
#define TEXTLOG_H

#include <QFile>
#include <QString>
#include <QMutex>

#define DEF_FILE_NAME "ARDUServer"

class textLog
{
private:
	static QString strFilePath;
	static QFile logFile;
	static QMutex logMutex;
	
	static int objCount;
	
public:
	textLog(QString strPath = QString());
	~textLog();
	bool create();
	void addLine(QString strLine, bool writeTimeStamp = true);
	void setFileName(QString strPath);

};

#endif // TEXTLOG_H