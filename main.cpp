#include "widget.h"
#include <QApplication>

//commit try

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	Widget w;
	w.show();
	
	return a.exec();
}
