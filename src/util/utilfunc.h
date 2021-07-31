#pragma once

#include <QTime>
#include <QString>

class QWidget;
class QStandardItemModel;

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
 * 将StandardItemModel中的所有文字搞到CSV里面去
 */
bool tableToCsv(const QStandardItemModel* model, const QString& filename);

/**
 * 先显示选择文件的对话框，然后再导出到CSV
 */
bool exportTableToCsv(const QStandardItemModel* model, QWidget* parent, const QString& initName);

}
