#pragma once

#include <QTime>
#include <QString>
#include <set>
#include <QList>
#include <QtLogging>

#include "data/common/traintime.h"

class QWidget;
class QStandardItemModel;
class QModelIndex;
class QStandardItem;
class QTableView;

namespace qeutil{

	extern const QString fileFilter;

/**
 * @brief parseTimeHMS 按照hh:mm:ss格式解析时间数据
 * 如果不能解析，返回默认构造的
 */
[[deprecated]] QTime parseTime(const QString& tm);

TrainTime parseTrainTime(const QString& tm);

/**
 * 返回tm1->tm2的秒数，考虑PBC
 */
inline int secsTo(const QTime& tm1, const QTime& tm2) {
	int secs = tm1.secsTo(tm2);
	return secs < 0 ? secs + 24 * 3600 : secs;
}

inline int secsTo(const TrainTime& tm1, const TrainTime& tm2, int period) {
	int secs = tm1.secsTo(tm2);
	return secs < 0 ? secs + period * 3600 : secs;
}

/**
 * 2023.02.12  此版本准确考虑跨日问题。
 */
int secsToStrict(const QTime& tm1, const QTime& tm2, int addDays);

int secsToStrict(const TrainTime& tm1, const TrainTime& tm2, int addDays, int period);

/**
 * 返回时间的中文字符串表示：xx分 或者 xx分xx秒
 */
QString secsToString(int secs);

/**
 * 返回时间间隔的中文表示 xx分 或者 xx分xx秒
 * 但数据为0时返回空
 */
QString secsToStringWithEmpty(int secs);

QString secsToString(const QTime& tm1, const QTime& tm2);

QString secsToString(const TrainTime& tm1, const TrainTime& tm2, int period);

/**
 * 返回 hh:mm:ss格式的时间字符串表示
 */
QString secsToStringHour(int secs);

/**
 * 用于天窗： hh:mm格式  传入分钟数！
 */
QString minsToStringHM(int mins);

/**
 * 返回 xx:xx形式的时间字符串表示
 * 支持负数
 */
QString secsDiffToString(int secs);

/**
 * 将StandardItemModel中的所有文字搞到CSV里面去
 */
bool tableToCsv(const QStandardItemModel* model, const QTableView* table, const QString& filename);

/**
 * 先显示选择文件的对话框，然后再导出到CSV
 * 2025.08.17: we should also accept a table in this function, because some tables may have
 * delegates that controlling the displayText.
 */
bool exportTableToCsv(const QStandardItemModel* model, const QTableView* table, QWidget* parent, const QString& initName);

bool ltIndexRow(const QModelIndex& idx1,const QModelIndex& idx2);

/**
 * @brief 2022.10.22  Get the (unique) rows from the QModelIndexList.
*/
std::set<int> indexRows(const QList<QModelIndex>& lst);

/**
 * 判断是否满足： left <= t <= right
 * 注意PBC
 */
bool timeInRange(const TrainTime& left, const TrainTime& right, const TrainTime& t, int period);

/**
 * 两个时间范围是否存在交叉。包含边界。
 * seealso: TrainStation::stopRangeIntersected
 */
bool timeRangeIntersected(const TrainTime& start1, const TrainTime& end1, const TrainTime& start2,
	const TrainTime& end2, int period_hours);

/**
 * 两个时间范围是否存在交叉。Excl后缀表示不含边界
 * seealso: TrainStation::stopRangeIntersected
 */
bool timeRangeIntersectedExcl(const TrainTime& start1, const TrainTime& end1, const TrainTime& start2,
	const TrainTime& end2, int period_hours);

/**
 * @brief timeCompare  全局函数 考虑周期边界条件下的时间比较
 * 采用能够使得两时刻之间所差时长最短的理解方式来消歧
 * 2022.03.09 从trainevents.h/.cpp 移动过来
 * @return tm1 < tm2  tm1是否被认为在tm2之前
 */
bool timeCompare(const TrainTime& tm1, const TrainTime& tm2, int period_hours);

/**
 * 2022.03.09
 * 由两对(start,to)指示的两条同一区间同向运行线是否交叉。
 * 交叉等价于前后发生互换：(start1 < start2) != (end1 < end2) 
 * 考虑PBC比较
 * 边界说明：如果两个start与两个end一个相等一个不相等（一端相交），返回false，
 * 这种情况由间隔来约束。如果两端都不相等就是正常的判断；如果两端都相等则返回true。
 */
bool timeCrossed(const TrainTime& start1, const TrainTime& start2,
	const TrainTime& end1, const TrainTime& end2, int period_hours);

/**
 * 两个时间范围是否存在交叉。包含边界。不考虑周期边界条件：
 * 即是保证start<=end。直接做简单的范围判断。
 * seealso: TrainStation::stopRangeIntersected
 */
bool timeRangeIntersectedNoPBC(const QTime& start1, const QTime& end1, const QTime& start2,
	const QTime& end2);

bool timeRangeIntersectedNoPBCExcl(const QTime& start1, const QTime& end1, const QTime& start2,
	const QTime& end2);

inline Qt::CheckState boolToCheckState(bool d) {
    return d ? Qt::Checked : Qt::Unchecked;
}

/**
 * 对指定基数m求最接近的整数解。
 * std::round理解为m=1的特殊情况。
 * 注意这里返回整数。
 */
int iround(double x, int m);

int ifloor(double x, int m);

int iceil(double x, int m);

QString msgTypeToString(QtMsgType type);

/**
 * 2023.11.17  add  for ETRC output
 * Write time string if valid, or treated as 0:00
 */
QString timeToStringNullable(const TrainTime& tm, TrainTime::TimeFormat fmt);


/**
 * 2024.06.24: get the invserse color: for (r,g,b,a), returns
 * (255-r, 255-g, 255-b, a).
 */
QColor inversedColor(const QColor& color);

QColor inversedColorIf(const QColor& color, bool stat);

void inverseColorIf(QColor& color, bool on);

QStandardItem* makeCheckItem();

QStandardItem* makeReadOnlyItem(const QString& text);

/**
 * Compare train names with possibly two-part algorithm (according to global configuration). 
 */
bool trainNameLess(const QString& s1, const QString& s2);
}
