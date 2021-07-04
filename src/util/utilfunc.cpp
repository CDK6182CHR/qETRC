#include "utilfunc.h"
#include <QList>
#include <QMessageBox>



//QTime qeutil::parseTime(const QString& tm)
//{
//	auto s = tm.split(":");
//	if (s.length() == 3) {
//		return QTime(s.at(0).toInt(), s.at(1).toInt(), s.at(2).toInt());
//	}
//	else if (s.length() == 2) {
//		return QTime(s.at(0).toInt(), s.at(1).toInt(), 0);
//	}
//	return QTime();
//}

QTime qeutil::parseTime(const QString& tm)
{
	int sub[3]{ 0,0,0 };
	int j = 0;
	for (int i = 0; i < tm.length() && j < 3; ++i) {
		const QChar& ch = tm.at(i);
		if (ch == ':')++j;
		else if (ch.isDigit())
			sub[j] = sub[j] * 10 + ch.digitValue();
	}
	if(j==1||j==2)
		return QTime(sub[0], sub[1], sub[2]);
	return QTime();
}

const QString qeutil::fileFilter =
	QObject::tr("pyETRC运行图文件(*.pyetgr;*.json)\nETRC运行图文件(*.trc)\n所有文件(*.*)");
