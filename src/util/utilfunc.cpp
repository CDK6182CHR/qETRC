#include "utilfunc.h"
#include <QList>
#include <QMessageBox>
#include <QFile>
#include <QStandardItemModel>
#include <QTextStream>
#include <QFileDialog>
#include <cmath>
#include <QModelIndex>
#include <QTime>
#include <QStandardItem>
#include <QTableView>
#include <QStyledItemDelegate>

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

TrainTime qeutil::parseTrainTime(const QString& tm)
{
	int sub[3]{ 0,0,0 };
	int j = 0;
	for (int i = 0; i < tm.length() && j < 3; ++i) {
		const QChar& ch = tm.at(i);
		if (ch == ':')++j;
		else if (ch.isDigit())
			sub[j] = sub[j] * 10 + ch.digitValue();
	}
	if (j == 1 || j == 2)
		return TrainTime(sub[0], sub[1], sub[2]);
	return TrainTime{};
}

int qeutil::secsToStrict(const QTime& tm1, const QTime& tm2, int addDays)
{
	int secs = tm1.secsTo(tm2);
	secs += addDays * 3600 * 24;
	return secs;
}

int qeutil::secsToStrict(const TrainTime& tm1, const TrainTime& tm2, int addDays, int period)
{
	int secs = tm1.secsTo(tm2);
	secs += addDays * 3600 * period;
	return secs;
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

QString qeutil::secsToString(const TrainTime& tm1, const TrainTime& tm2, int period)
{
	return secsToString(secsTo(tm1, tm2, period));
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

bool qeutil::tableToCsv(const QStandardItemModel* model, const QTableView* table, const QString& filename)
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
			auto* dele = table->itemDelegateForColumn(j);
			if (auto* delest = qobject_cast<QStyledItemDelegate*>(dele)) {
				s << delest->displayText(model->item(i, j)->data(Qt::DisplayRole), QLocale{}) << ",";
			}
			else {
				s << model->item(i, j)->text() << ",";
			}
		}
		s << Qt::endl;
	}
	file.close();
	return true;
}

bool qeutil::exportTableToCsv(const QStandardItemModel* model, const QTableView* table, QWidget* parent, const QString& initName)
{
	QString fn = QFileDialog::getSaveFileName(parent, QObject::tr("导出CSV数据"), initName,
		QObject::tr("逗号分隔文件 (*.csv)\n 所有文件 (*)"));
	if (fn.isEmpty())return false;
	bool flag = tableToCsv(model, table, fn);
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

std::set<int> qeutil::indexRows(const QList<QModelIndex>& lst)
{
	std::set<int> ret{};
	foreach(const auto & idx, lst) {
		ret.emplace(idx.row());
	}
	return ret;
}

bool qeutil::timeInRange(const TrainTime& left, const TrainTime& right, const TrainTime& t, int period)
{
	int secsOfPeriod = 3600 * period;
	int tleft = left.secondsSinceStart(), tright = right.secondsSinceStart();
	int tt = t.secondsSinceStart();

	if (tright < tleft)
		tright += secsOfPeriod;
	if (tleft <= tt && tt <= tright)
		return true;
	//考虑一次平移
	tt += secsOfPeriod;
	if (tleft <= tt && tt <= tright)
		return true;
	return false;
}


bool qeutil::timeRangeIntersected(const TrainTime& start1, const TrainTime& end1, const TrainTime& start2,
	const TrainTime& end2, int period_hours)
{
	int periodSecs = period_hours * 3600;
	int xm1 = start1.secondsSinceStart(), xm2 = end1.secondsSinceStart();
	int xh1 = start2.secondsSinceStart(), xh2 = end2.secondsSinceStart();
	bool flag1 = (xm2 < xm1), flag2 = (xh2 < xh1);
	if (flag1)xm2 += periodSecs;
	if (flag2)xh2 += periodSecs;
	bool res1 = (std::max(xm1, xh1) <= std::min(xm2, xh2));   //不另加PBC下的比较
	if (res1 || flag1 == flag2) {
		// 如果都加了或者都没加PBC，这就是结果
		return res1;
	}
	//如果只有一边加了PBC，那么应考虑把另一边也加上PBC再试试
	if (flag1) {
		xh1 += periodSecs; xh2 += periodSecs;
	}
	else {
		xm1 += periodSecs; xm2 += periodSecs;
	}
	return (std::max(xm1, xh1) <= std::min(xm2, xh2));
}

bool qeutil::timeRangeIntersectedExcl(const TrainTime& start1, const TrainTime& end1, const TrainTime& start2,
	const TrainTime& end2, int period_hours)
{
	int periodSecs = period_hours * 3600;
	int xm1 = start1.secondsSinceStart(), xm2 = end1.secondsSinceStart();
	int xh1 = start2.secondsSinceStart(), xh2 = end2.secondsSinceStart();
	bool flag1 = (xm2 < xm1), flag2 = (xh2 < xh1);
	if (flag1)xm2 += period_hours;
	if (flag2)xh2 += period_hours;
	bool res1 = (std::max(xm1, xh1) < std::min(xm2, xh2));   //不另加PBC下的比较
	if (res1 || flag1 == flag2) {
		// 如果都加了或者都没加PBC，这就是结果
		return res1;
	}
	//如果只有一边加了PBC，那么应考虑把另一边也加上PBC再试试
	if (flag1) {
		xh1 += periodSecs; xh2 += periodSecs;
	}
	else {
		xm1 += periodSecs; xm2 += periodSecs;
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

int qeutil::iround(double x, int m)
{
	double r = std::fmod(x, m);
	int flr = ifloor(x, m);
	if (r >= m / 2.0) {
		return flr + m;
	}
	else {
		return flr;
	}
}

int qeutil::ifloor(double x, int m)
{
	return static_cast<int>(std::floor(x / m)) * m;
}

int qeutil::iceil(double x, int m)
{
	return static_cast<int>(std::ceil(x / m)) * m;
}

QString qeutil::msgTypeToString(QtMsgType type)
{
	switch (type)
	{
	case QtDebugMsg: return "Debug";
		break;
	case QtWarningMsg: return "Warning";
		break;
	case QtCriticalMsg: return "Critical";
		break;
	case QtFatalMsg: return "Fatal";
		break;
	case QtInfoMsg: return "Info";
		break;
	default:
		return "UNKNOWN";
	}
}

QString qeutil::timeToStringNullable(const TrainTime& tm, TrainTime::TimeFormat fmt)
{
	return !tm.isNull() ? tm.toString(fmt) : TrainTime(0, 0).toString(fmt);
}


bool qeutil::timeCompare(const TrainTime& tm1, const TrainTime& tm2, int period_hours)
{
	int periodSecs = 3600 * period_hours;
	int secs = tm1.secsTo(tm2);
	bool res = (secs > 0);
	if (std::abs(secs) > periodSecs / 2)
		return !res;
	return res;
}


bool qeutil::timeCrossed(const TrainTime& start1, const TrainTime& start2,
	const TrainTime& end1, const TrainTime& end2, int period_hours)
{
	if (start1 == start2 && end1 == end2)
		return true;
	else
		return timeCompare(start1, start2, period_hours) != timeCompare(end1, end2, period_hours)
			&& ((start1 != start2) == (end1 != end2));
}

QString qeutil::secsToStringHour(int secs)
{
    return QString::asprintf("%d:%02d:%02d",secs/3600,secs%3600/60,secs%60);
}

QColor qeutil::inversedColor(const QColor& color)
{
	return QColor(255 - color.red(), 255 - color.green(), 255 - color.blue(), color.alpha());
}

QColor qeutil::inversedColorIf(const QColor& color, bool stat)
{
	return stat ? color : inversedColor(color);
}

void qeutil::inverseColorIf(QColor& color, bool on)
{
	if (on) {
		color.setRed(255 - color.red());
		color.setGreen(255 - color.green());
		color.setBlue(255 - color.blue());
	}
}

QStandardItem* qeutil::makeCheckItem()
{
	auto* it = new QStandardItem;
	it->setCheckable(true);
	return it;
}

QStandardItem* qeutil::makeReadOnlyItem(const QString& text)
{
	auto* it = new QStandardItem(text);
	it->setEditable(false);
	return it;
}
