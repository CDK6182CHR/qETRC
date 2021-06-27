#pragma once

#include <QTime>
#include <QString>

namespace qeutil{

/**
 * @brief parseTimeHMS 按照hh:mm:ss格式解析时间数据
 * 如果不能解析，返回默认构造的
 */
QTime parseTime(const QString& tm);

}
