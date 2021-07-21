#include "rulertabpy.h"

RulerTabPy::RulerTabPy(std::shared_ptr<Ruler> ruler_, bool hasMain_, QWidget *parent) :
    QWidget(parent),ruler(ruler_),hasMain(hasMain_)
{

}
