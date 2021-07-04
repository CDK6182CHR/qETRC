#pragma once

#include <QTime>
#include <QString>

class QWidget;

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

}
