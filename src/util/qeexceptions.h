#ifndef QEEXCEPTIONS_H
#define QEEXCEPTIONS_H

#include <QException>
#include <QString>

#include "data/rail/stationname.h"

class StationNotInRailException:
        public QException
{
public:
    StationName name;
    StationNotInRailException(const StationName& name_):name(name_){}
};

#endif // QEEXCEPTIONS_H
