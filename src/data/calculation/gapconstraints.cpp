#include "gapconstraints.h"

int GapConstraints::correlationRange() const
{
    int res=GLOBAL_DEFAULT;
    for(const auto& t:*this){
        if (t.second>res)
            res=t.second;
    }
    return res;
}

QString GapConstraints::toString() const
{
    QString res;
    int i = 0;
    for (auto itr = begin(); itr != end(); ++itr) {
        res.append(QObject::tr("%1. %2: %3 s\n").arg(++i)
            .arg(TrainGap::posTypeToString(itr->first.second, itr->first.first).replace('\n',' '))
            .arg(itr->second));
    }
    return res;
}
