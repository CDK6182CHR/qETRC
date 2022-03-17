#include "gapsetabstract.h"



void gapset::GapSetAbstract::setSingleLineAndBuild(bool on)
{
    if (on != _singleLine){
        _singleLine=on;
        buildSet();
    }
}
