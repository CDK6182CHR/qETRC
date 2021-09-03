#pragma once

#include <QTime>
#include <QString>

class QWidget;
class QStandardItemModel;
class QModelIndex;

namespace qeutil{

	extern const QString fileFilter;

/**
 * @brief parseTimeHMS 按照hh:mm:ss格式解析时间数据
 * 如果不能解析，返回默认构造的
 */
QTime parseTime(const QString& tm);

/**
 * 返回tm1->tm2的秒数，考虑PBC
 */
inline int secsTo(const QTime& tm1, const QTime& tm2) {
	int secs = tm1.secsTo(tm2);
	return secs < 0 ? secs + 24 * 3600 : secs;
}

/**
 * 返回时间的中文字符串表示：xx分 或者 xx分xx秒
 */
QString secsToString(int secs);

QString secsToString(const QTime& tm1, const QTime& tm2);

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
bool tableToCsv(const QStandardItemModel* model, const QString& filename);

/**
 * 先显示选择文件的对话框，然后再导出到CSV
 */
bool exportTableToCsv(const QStandardItemModel* model, QWidget* parent, const QString& initName);

bool ltIndexRow(const QModelIndex& idx1,const QModelIndex& idx2);

static constexpr int msecsOfADay = 24 * 3600 * 1000;

/**
 * 判断是否满足： left <= t <= right
 * 注意PBC
 */
bool timeInRange(const QTime& left, const QTime& right, const QTime& t);

/**
 * 两个时间范围是否存在交叉。包含边界。
 * seealso: TrainStation::stopRangeIntersected
 */
bool timeRangeIntersected(const QTime& start1, const QTime& end1, const QTime& start2,
	const QTime& end2);

}
