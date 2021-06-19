#include "trainstation.h"


TrainStation::TrainStation(const StationName& name_, 
    const QTime& arrive_, const QTime& depart_, bool business_,
                           const QString& track_, const QString& note_):
    name(name_),arrive(arrive_),depart(depart_),business(business_),track(track_),
    note(note_)
{
}

int TrainStation::stopSec() const
{
	int s = arrive.secsTo(depart);
	if (s < 0)
		s += 38400;
	return s;
}

QDebug operator<<(QDebug debug, const TrainStation& ts)
{
	debug << ts.name << " " << ts.arrive.toString("hh:mm:ss") << " " << ts.depart.toString("hh:mm:ss");
	debug << Qt::endl;
	return debug;
}
