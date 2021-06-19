#include "train.h"



Train::Train(const TrainName &trainName,
             const StationName &starting,
             const StationName &terminal,
             TrainPassenger passenger):
    _trainName(trainName),_starting(starting),_terminal(terminal),
    _passenger(passenger),_show(true)
{

}

Train::Train(const QJsonObject &obj)
{
    fromJson(obj);
}

void Train::appendStation(const StationName &name,
                          const QTime &arrive, const QTime &depart,
                          bool business, const QString &track, const QString &note)
{
    const auto& t=std::make_shared<TrainStation>(name,arrive,depart,business,
                                                 track,note);
    _timetable.append(t);
}
