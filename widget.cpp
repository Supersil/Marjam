#include "widget.h"

#include <QFont>
#include <QMessageBox>
#include <QDateTime>

Widget::Widget(QWidget *parent)
	: QWidget(parent)
{
	mainLayout = std::unique_ptr<QGridLayout>(new QGridLayout);
	tabWidget = std::unique_ptr<QTabWidget>(new QTabWidget);
	tabWidget->setTabBarAutoHide(true);
	
	QFont font("notperfect regular",20);
	
	tabWelcome = std::unique_ptr<QWidget>(new QWidget);
	tabWidget->addTab(tabWelcome.get(),"Welcome");
	welcomeLayout = std::unique_ptr<QGridLayout>(new QGridLayout);
	tabWelcome->setLayout(welcomeLayout.get());
	lblGreeting = std::unique_ptr<QLabel>(new QLabel("Hello world"));
	welcomeLayout->addWidget(lblGreeting.get(),0,0);
	lblGreeting->setFont(font);
	lblGreeting->setText(
"Привет Марьяшка, мы - волшебные создания Земли давно наблюдаем за тобой.\n\
Нам нравится твое намерение нести свет этому миру и мы хотим вознаградить тебя!\n\
Но награда должна быть заслужена, поэтому мы подготовили несколько заданий\n\
для проверки твоих способностей. \n\
Сперва сконцентрируй все лучи добра в одной точке на панели.");
	lblGreeting->setAlignment(Qt::AlignCenter);
	
	
	tabLogPass = std::unique_ptr<QWidget>(new QWidget);
	tabWidget->addTab(tabLogPass.get(),"Auth");
	logPassLayout = std::unique_ptr<QGridLayout>(new QGridLayout);
	tabLogPass->setLayout(logPassLayout.get());
	lblLogPass = std::unique_ptr<QLabel>(new QLabel());
	lblLogPass->setFont(font);
	lblLogPass->setText("Молодец, а теперь следуй зову музыки.");
	
	authBox = std::unique_ptr<QGroupBox>(new QGroupBox("Введите аутентификационные данные"));
	authLayout = std::unique_ptr<QGridLayout>(new QGridLayout);
	lblLogin = std::unique_ptr<QLabel>(new QLabel("Логин: "));
	editLogin = std::unique_ptr<QLineEdit>(new QLineEdit);
	lblPass = std::unique_ptr<QLabel>(new QLabel("Пароль: "));
	editPass = std::unique_ptr<QLineEdit>(new QLineEdit);
	btnCheck = std::unique_ptr<QPushButton>(new QPushButton("Проверить данные"));
	
	QFont bold_font("Times New Roman",20);
	bold_font.setBold(true);
	authBox->setFont(bold_font);
	lblLogin->setFont(bold_font);
	lblPass->setFont(bold_font);
	editLogin->setEchoMode(QLineEdit::Password);
	editPass->setEchoMode(QLineEdit::Password);
	
	authLayout->addWidget(lblLogin.get(),0,0,Qt::AlignCenter);
	authLayout->addWidget(editLogin.get(),0,1,Qt::AlignCenter);
	authLayout->addWidget(lblPass.get(),1,0,Qt::AlignCenter);
	authLayout->addWidget(editPass.get(),1,1,Qt::AlignCenter);
	authLayout->addWidget(btnCheck.get(),2,0,1,2,Qt::AlignCenter);
	authBox->setLayout(authLayout.get());
	
	logPassLayout->addWidget(lblLogPass.get(),0,0);
	logPassLayout->addWidget(authBox.get(),1,0);
	
	tabPuzzle = std::unique_ptr<QWidget>(new QWidget);
	tabWidget->addTab(tabPuzzle.get(),"Puzzle");
	puzzleLayout = std::unique_ptr<QGridLayout>(new QGridLayout);
	tabPuzzle->setLayout(puzzleLayout.get());
	lblPuzzle = std::unique_ptr<QLabel>(new QLabel());
	puzzleLayout->addWidget(lblPuzzle.get(),0,0);
	
	lblPuzzle->setFont(font);
	lblPuzzle->setText(
"ООО Да это же сильнейший артефакт\n\
\"Волшебная палочка, отпирающая запоры\"!\n\
Сохрани ее, она ещё может пригодиться.\n\n\
Раздвигаем мы его, чтоб гостям поспать\n\
Загляни под правый край, попробуй порыскать!");
	lblPuzzle->setAlignment(Qt::AlignCenter);

			
	
	mainLayout->addWidget(tabWidget.get(),0,0);
	
	
	setLayout(mainLayout.get());
	
	changeShortcut = std::unique_ptr<QShortcut>(new QShortcut(QKeySequence("Ctrl+E"),this));
	connect(changeShortcut.get(),SIGNAL(activated()),this,SLOT(changeTab()));
	
	tabWidget->setTabEnabled(1,false);
	tabWidget->setTabEnabled(2,false);
	
	
	logFile.create();
	
	tcpServer = std::unique_ptr<QTcpServer>(new QTcpServer(this));
	bServerIsListening = false;
	connect(tcpServer.get(), SIGNAL(newConnection()), this, SLOT(inputConnection()));
	if (!tcpServer->listen(QHostAddress::Any, LISTEN_PORT) && (bServerIsListening == false))
	{
		logFile.addLine("Cant start server");
		QMessageBox::critical(this,"Ошибка","Перезапустите программу");
	}
	else 
	{
		bServerIsListening = true;
		logFile.addLine("Server started");
	}
	serial.setPortName("COM4");
	if(!serial.open(QIODevice::ReadWrite))
		logFile.addLine("Can't open COM port COM4");
	connect(&serial,SIGNAL(readyRead()),this,SLOT(serialReady()));
	
}


Widget::~Widget()
{
	
}

void Widget::changeTab()
{
	tabWidget->setTabEnabled(tabWidget->currentIndex(),false);
	tabWidget->setTabEnabled(tabWidget->currentIndex(),true);
}


void Widget::inputConnection()
{
	if (bServerIsListening == true)
	{
		logFile.addLine("New connection");
		QTcpSocket * clientSocket = tcpServer->nextPendingConnection();
		int idusersocs = clientSocket->socketDescriptor();
		QTextStream os(clientSocket);
		os.setAutoDetectUnicode(true);
		
		os << "Server is UP!\n"	<< QDateTime::currentDateTime().toString() << "\n";
		
		SClients[idusersocs]=clientSocket;
		connect(SClients[idusersocs],SIGNAL(readyRead()),this, SLOT(slotReadClient()));
	}
}


void Widget::slotReadClient()
{
	QTcpSocket* clientSocket = (QTcpSocket*)sender();
	QByteArray clientData = clientSocket->readAll();
	logFile.addLine(clientData);
	if (clientData.contains("change"))
		changeTab();
}

void Widget::serialReady()
{
	QByteArray buf;

	if (serial.canReadLine())
		buf = serial.readLine();
	else
		return;

	if (buf.contains("Success"))
	{
		qDebug() << buf;

		if (tabWidget->currentIndex() == 0)
			changeTab();
	}
}
