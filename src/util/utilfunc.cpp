#include "utilfunc.h"
#include <QList>
#include <QMessageBox>
#include <QFile>
#include <QStandardItemModel>
#include <QTextStream>
#include <QFileDialog>


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

QString qeutil::secsToString(int secs)
{
	if (secs % 60 == 0)
		return QObject::tr("%1分").arg(secs / 60);
	else
		return QObject::tr("%1分%2秒").arg(secs / 60).arg(secs % 60);
}

QString qeutil::secsToString(const QTime& tm1, const QTime& tm2)
{
	return secsToString(secsTo(tm1, tm2));
}

QString qeutil::secsToStringWithEmpty(int secs)
{
	if (secs)
		return secsToString(secs);
	else return "";
}

QString qeutil::minsToStringHM(int mins)
{
	return QString::asprintf("%d:%02d", mins / 60, mins % 60);
}

QString qeutil::secsDiffToString(int secs)
{
	if (secs >= 0) {
		return QString::asprintf("%d:%02d", secs / 60, secs % 60);
	}
	else {
		return QString::asprintf("-%d:%02d", (-secs) / 60, (-secs) % 60);
	}
}

const QString qeutil::fileFilter =
	QObject::tr("pyETRC运行图文件(*.pyetgr;*.json)\nETRC运行图文件(*.trc)\n所有文件(*.*)");

bool qeutil::tableToCsv(const QStandardItemModel* model, const QString& filename)
{
	QFile file(filename);
	file.open(QFile::WriteOnly);
	if (!file.isOpen())return false;
	QTextStream s(&file);

	//标题
	for (int i = 0; i < model->columnCount(); i++) {
		s << model->horizontalHeaderItem(i)->text() << ",";
	}
	s << Qt::endl;

	//内容
	for (int i = 0; i < model->rowCount(); i++) {
		for (int j = 0; j < model->columnCount(); j++) {
			s << model->item(i, j)->text() << ",";
		}
		s << Qt::endl;
	}
	file.close();
	return true;
}

bool qeutil::exportTableToCsv(const QStandardItemModel* model, QWidget* parent, const QString& initName)
{
	QString fn = QFileDialog::getSaveFileName(parent, QObject::tr("导出列车事件表"), initName,
		QObject::tr("逗号分隔文件 (*.csv)\n 所有文件 (*)"));
	if (fn.isEmpty())return false;
	bool flag = tableToCsv(model, fn);
	if (flag) {
		QMessageBox::information(parent, QObject::tr("提示"), QObject::tr("导出CSV文件成功"));
	}
	else {
		QMessageBox::warning(parent, QObject::tr("提示"), QObject::tr("导出CSV文件失败"));
	}
	return flag;
}

bool qeutil::ltIndexRow(const QModelIndex &idx1, const QModelIndex &idx2)
{
    return idx1.row()<idx2.row();
}

bool qeutil::timeInRange(const QTime& left, const QTime& right, const QTime& t)
{
	int tleft = left.msecsSinceStartOfDay(), tright = right.msecsSinceStartOfDay();
	int tt = t.msecsSinceStartOfDay();
	
	if (tright < tleft)
		tright += msecsOfADay;
	if (tleft <= tt && tt <= tright)
		return true;
	//考虑一次平移
	tt += msecsOfADay;
	if (tleft <= tt && tt <= tright)
		return true;
	return false;
}


bool qeutil::timeRangeIntersected(const QTime& start1, const QTime& end1, const QTime& start2,
	const QTime& end2)
{
	int xm1 = start1.msecsSinceStartOfDay(), xm2 = end1.msecsSinceStartOfDay();
	int xh1 = start2.msecsSinceStartOfDay(), xh2 = end2.msecsSinceStartOfDay();
	bool flag1 = (xm2 < xm1), flag2 = (xh2 < xh1);
	if (flag1)xm2 += msecsOfADay;
	if (flag2)xh2 += msecsOfADay;
	bool res1 = (std::max(xm1, xh1) <= std::min(xm2, xh2));   //不另加PBC下的比较
	if (res1 || flag1 == flag2) {
		// 如果都加了或者都没加PBC，这就是结果
		return res1;
	}
	//如果只有一边加了PBC，那么应考虑把另一边也加上PBC再试试
	if (flag1) {
		xh1 += msecsOfADay; xh2 += msecsOfADay;
	}
	else {
		xm1 += msecsOfADay; xm2 += msecsOfADay;
	}
	return (std::max(xm1, xh1) <= std::min(xm2, xh2));
}

bool qeutil::timeRangeIntersectedExcl(const QTime& start1, const QTime& end1, const QTime& start2,
	const QTime& end2)
{
	int xm1 = start1.msecsSinceStartOfDay(), xm2 = end1.msecsSinceStartOfDay();
	int xh1 = start2.msecsSinceStartOfDay(), xh2 = end2.msecsSinceStartOfDay();
	bool flag1 = (xm2 < xm1), flag2 = (xh2 < xh1);
	if (flag1)xm2 += msecsOfADay;
	if (flag2)xh2 += msecsOfADay;
	bool res1 = (std::max(xm1, xh1) < std::min(xm2, xh2));   //不另加PBC下的比较
	if (res1 || flag1 == flag2) {
		// 如果都加了或者都没加PBC，这就是结果
		return res1;
	}
	//如果只有一边加了PBC，那么应考虑把另一边也加上PBC再试试
	if (flag1) {
		xh1 += msecsOfADay; xh2 += msecsOfADay;
	}
	else {
		xm1 += msecsOfADay; xm2 += msecsOfADay;
	}
	return (std::max(xm1, xh1) < std::min(xm2, xh2));
}

bool qeutil::timeRangeIntersectedNoPBC(const QTime& start1, const QTime& end1, const QTime& start2,
	const QTime& end2)
{
	int xm1 = start1.msecsSinceStartOfDay(), xm2 = end1.msecsSinceStartOfDay();
	int xh1 = start2.msecsSinceStartOfDay(), xh2 = end2.msecsSinceStartOfDay();
	return (std::max(xm1, xh1) <= std::min(xm2, xh2));
}

bool qeutil::timeRangeIntersectedNoPBCExcl(const QTime& start1, const QTime& end1, const QTime& start2,
	const QTime& end2)
{
	int xm1 = start1.msecsSinceStartOfDay(), xm2 = end1.msecsSinceStartOfDay();
	int xh1 = start2.msecsSinceStartOfDay(), xh2 = end2.msecsSinceStartOfDay();
	return (std::max(xm1, xh1) < std::min(xm2, xh2));
}

QString qeutil::secsToStringHour(int secs)
{
    return QString::asprintf("%d:%02d:%02d",secs/3600,secs%3600/60,secs%60);
}
