#include "pathrulerseg.h"


#include "data/rail/ruler.h"
#include "trainpathseg.h"

QString PathRulerSeg::getRulerName()const
{
    if (ruler.expired()) {
        return ruler_name;
    }
    else {
        return ruler.lock()->name();
    }
}

PathRulerSeg::PathRulerSeg(std::shared_ptr<Ruler> ruler_):
    ruler_name(ruler_->name()), ruler(ruler_)
{
}

PathRulerSeg::PathRulerSeg(const QString& name):
    ruler_name(name), ruler()
{
}

PathRulerSeg::PathRulerSeg(const QJsonObject& obj, const TrainPathSeg& pathseg)
{
    fromJson(obj, pathseg);
}

void PathRulerSeg::fromJson(const QJsonObject& obj, const TrainPathSeg& pathseg)
{
    ruler_name = obj.value("ruler_name").toString();
    if (!pathseg.railway.expired()) {
        auto rail = pathseg.railway.lock();
        ruler = rail->rulerByName(ruler_name);
    }
}

QJsonObject PathRulerSeg::toJson() const
{
    return QJsonObject{
        {"ruler_name", getRulerName()}
    };
}

bool PathRulerSeg::checkIsValid()const
{
    if (ruler.expired()) {
        qWarning() << "PathRulerSeg::checkIsValid: the ruler has been deleted";
        return false;
    }
    auto r = ruler.lock();
    if (!r->isValid()) {
        qWarning() << "PathRulerSeg::checkIsValid: the ruler is in invalid state (mostly deleted)";
        return false;
    }
    return true;
}
