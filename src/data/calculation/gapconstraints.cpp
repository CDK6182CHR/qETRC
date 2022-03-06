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
