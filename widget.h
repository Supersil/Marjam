#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTabWidget>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <memory>
#include <QShortcut>
#include <QPushButton>
#include <QGroupBox>

#include <QTcpServer>
#include <QTcpSocket>

#include "textlog.h"
#include <QtSerialPort/QSerialPort>

#define LISTEN_PORT 34123

class Widget : public QWidget
{
	Q_OBJECT
	
public:
	Widget(QWidget *parent = 0);
	~Widget();
	
private:
	QSerialPort serial;

	std::unique_ptr<QGridLayout> mainLayout;
	std::unique_ptr<QTabWidget> tabWidget;
	
	
	// first tab
	std::unique_ptr<QWidget> tabWelcome;
	std::unique_ptr<QGridLayout> welcomeLayout;
	std::unique_ptr<QLabel> lblGreeting;
	
	
	// second tab
	std::unique_ptr<QWidget> tabLogPass;
	std::unique_ptr<QGridLayout> logPassLayout;
	std::unique_ptr<QLabel> lblLogPass;
	
	std::unique_ptr<QGroupBox> authBox;
	std::unique_ptr<QGridLayout> authLayout;
	std::unique_ptr<QLabel> lblLogin;
	std::unique_ptr<QLineEdit> editLogin;
	std::unique_ptr<QLabel> lblPass;
	std::unique_ptr<QLineEdit> editPass;
	std::unique_ptr<QPushButton> btnCheck;
	
	//third tab
	std::unique_ptr<QWidget> tabPuzzle;
	std::unique_ptr<QGridLayout> puzzleLayout;
	std::unique_ptr<QLabel> lblPuzzle;
	
	
	std::unique_ptr<QShortcut> changeShortcut;
	
	
	std::unique_ptr<QTcpServer> tcpServer;
	bool bServerIsListening;
	
	textLog logFile;
	 
	QMap<int,QTcpSocket *> SClients;
	
private slots:
	void changeTab();
	void inputConnection();
	void slotReadClient();
	void serialReady();
};

#endif // WIDGET_H
