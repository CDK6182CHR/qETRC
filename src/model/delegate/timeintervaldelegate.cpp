#include "timeintervaldelegate.h"
#include "util/utilfunc.h"

QString TimeIntervalDelegate::displayText(const QVariant &value, const QLocale &locale) const
{
    Q_UNUSED(locale)
    return qeutil::secsDiffToString(value.toInt());
}

QString TimeIntervalDelegateHour::displayText(const QVariant& value, const QLocale& locale) const
{
    Q_UNUSED(locale)
    return qeutil::secsToStringHour(value.toInt());
}
