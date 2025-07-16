#include "traintime.h"

TrainTime::TrainTime(int h, int m, int s)
{
    setHMS(h, m, s);
}

void TrainTime::setHMS(int h, int m, int s)
{
    m_secs = h * 3600 + m * 60 + s;
}

int TrainTime::hour() const
{
    return m_secs / 3600;
}

int TrainTime::minute() const
{
    return m_secs / 60 % 60;
}

int TrainTime::second() const
{
    return m_secs % 60;
}

bool TrainTime::isNull() const
{
    return m_secs == -1;
}

bool TrainTime::isValid(int maxHour) const
{
    return m_secs >= 0 && m_secs < maxHour * 3600;
}

int TrainTime::secondsSinceStart() const
{
    return m_secs;
}

QString TrainTime::toString(TimeFormat form) const
{
    if (isNull()) {
        return {};
    }

    switch (form) {
    case HMS: return toStringHMS();
    case HM: return toStringHM();
    default: return {};
    }
}

QString TrainTime::toStringHMS() const
{
    return QString("%1:%2:%3").arg(hour()).arg(minute(), 2, 10, QChar('0'))
        .arg(second(), 2, 10, QChar('0'));
}

QString TrainTime::toStringHM() const
{
    return QString("%1:%2").arg(hour()).arg(minute(), 2, 10, QChar('0'));
}

TrainTime TrainTime::fromString(const QString& tm)
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
    return TrainTime();
}

TrainTime TrainTime::fromSecondsSinceStart(int secs)
{
    return TrainTime(secs);   // Private constructor
}

void TrainTime::refineHour(int maxHours)
{
    int max_secs = maxHours * 3600;
    m_secs = (m_secs % max_secs + max_secs) % max_secs;
}

bool TrainTime::operator==(const TrainTime& rhs) const
{
    return m_secs == rhs.m_secs;
}

void TrainTime::addHoursInplace(int h)
{
    m_secs += h * 3600;
}

void TrainTime::addMinutesInplace(int m)
{
    m_secs += m * 60;
}

void TrainTime::addSecondsInplace(int s)
{
    m_secs += s;
}

TrainTime TrainTime::addHours(int h, int period)const
{
    TrainTime res = fromSecondsSinceStart(m_secs + h * 3600);
	res.refineHour(period);
    return res;
}

TrainTime TrainTime::addMins(int m, int period)const
{
    TrainTime res = fromSecondsSinceStart(m_secs + m * 60);
    res.refineHour(period);
    return res;
}

TrainTime TrainTime::addSecs(int s, int period)const
{
    TrainTime res = fromSecondsSinceStart(m_secs + s);
    res.refineHour(period);
    return res;
}

TrainTime TrainTime::addHoursUnrefined(int h) const
{
    return fromSecondsSinceStart(m_secs + h * 3600);
}

TrainTime TrainTime::addMinsUnrefined(int m) const
{
    return fromSecondsSinceStart(m_secs + m * 60);
}

TrainTime TrainTime::addSecsUnrefined(int s) const
{
    return fromSecondsSinceStart(m_secs + s);
}

int TrainTime::secsTo(const TrainTime& rhs) const
{
    return rhs.m_secs - m_secs;
}

QTime TrainTime::toQTime() const
{
	return QTime(hour(), minute(), second());
}

TrainTime TrainTime::fromQTime(const QTime& t)
{
	return TrainTime(t.hour(), t.minute(), t.second());
}

TrainTime::TrainTime(const QTime& qtm):
	m_secs(qtm.msecsSinceStartOfDay() / 1000)
{
}

TrainTime::operator QTime() const
{
    return toQTime();
}

TrainTime::TrainTime(int secsSinceStart) :
    m_secs(secsSinceStart)
{

}

void TrainTime::getHMS(int& h, int& m, int& s) const
{
    s = m_secs % 60;
    m = m_secs / 60 % 60;
    h = m_secs / 3600;
}

