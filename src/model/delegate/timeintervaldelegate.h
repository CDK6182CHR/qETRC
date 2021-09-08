#pragma once

#include <QStyledItemDelegate>

/**
 * @brief The TimeIntervalDelegate class
 * 以mm:ss格式显示时间差。
 * 暂时不写编辑逻辑；只是实现显示字符串。
 */
class TimeIntervalDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    using QStyledItemDelegate::QStyledItemDelegate;
    //explicit TimeIntervalDelegate(QObject *parent = nullptr);
    virtual QString displayText(const QVariant &value, const QLocale &locale) const override;
};
