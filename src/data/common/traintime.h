#pragma once

#include <QTime>
#include <QString>
#include <QMetaType>

class TrainTime
{
    int m_secs = -1;
public:

    enum TimeFormat {
        HMS,
        HM,
    };

    TrainTime() = default;
    TrainTime(int h, int m, int s = 0);

    void setHMS(int h, int m, int s = 0);

    int hour()const;
    int minute()const;
    int second()const;

    bool isNull()const;
    bool isValid(int maxHour)const;

    int secondsSinceStart()const;

    QString toString(TimeFormat form)const;

    QString toStringHMS()const;

    QString toStringHM()const;

    /**
     * @brief fromString
     * Parse time from hh:mm or hh:mm:ss format.
     * Previously qeutil::parseTime.
     */
    static TrainTime fromString(const QString& s);

    static TrainTime fromSecondsSinceStart(int secs);

    void refineHour(int maxHours);

    bool operator==(const TrainTime& rhs)const;

    auto operator<=>(const TrainTime& rhs)const {
		return m_secs <=> rhs.m_secs;
    }

    void addHoursInplace(int h);

    void addMinutesInplace(int m);

    void addSecondsInplace(int s);

    // The addSecs/Mins/Hours functions are similar to QTime methods with same names.
    // But we should refine the hours by default!

    TrainTime addHours(int h, int period)const;
    TrainTime addMins(int m, int period)const;
    TrainTime addSecs(int s, int period)const;

    TrainTime addHoursUnrefined(int h)const;
    TrainTime addMinsUnrefined(int m)const;
    TrainTime addSecsUnrefined(int s)const;

    /**
     * Compute the time interval between this and rhs (rhs - *this)
     */
    int secsTo(const TrainTime& rhs)const;

    // For compatible with QTime (only explicit function calls for safety)
    QTime toQTime()const;
    //operator QTime()const;

    static TrainTime fromQTime(const QTime& t);
    //explicit TrainTime(const QTime& qtm);

private:
    explicit TrainTime(int secsSinceStart);
    void getHMS(int& h, int& m, int& s)const;
};

Q_DECLARE_METATYPE(TrainTime)
