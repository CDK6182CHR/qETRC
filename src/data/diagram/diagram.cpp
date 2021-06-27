#include "diagram.h"
#include "data/rail/railway.h"
#include "trainadapter.h"

#include <QFile>
#include <QJsonObject>

void Diagram::addRailway(std::shared_ptr<Railway> rail)
{
    _railways.append(rail);
    for (auto p : trains()) {
        p->bindToRailway(*rail, _config);
    }
}

void Diagram::addTrains(const TrainCollection& coll)
{
    //复制语义
    for (auto p : coll.trains()) {
        if (!_trainCollection.trainNameExisted(p->trainName())) {
            auto t = std::make_shared<Train>(*p);
            _trainCollection.trains().append(t);
            updateTrain(t);
        }
    }
}

std::shared_ptr<Railway> Diagram::railwayByName(const QString &name)
{
    for(const auto& p:_railways){
        if(p->name() == name)
            return p;
    }
    return nullptr;
}

void Diagram::updateRailway(std::shared_ptr<Railway> r)
{
    for (const auto& p:_trainCollection.trains()){
        p->updateBoundRailway(*r, _config);
    }
}

void Diagram::updateTrain(std::shared_ptr<Train> t)
{
    for(const auto& r:_railways){
        t->updateBoundRailway(*r, _config);
    }
}

TrainEventList Diagram::listTrainEvents(const Train& train) const
{
    TrainEventList res;
    for (auto p : train.adapters()) {
        res.append(qMakePair(p, p->listAdapterEvents(_trainCollection)));
    }
    return res;
}

void Diagram::bindAllTrains()
{
    for (auto p : _railways) {
        for (auto t : _trainCollection.trains()) {
            t->bindToRailway(*p, _config);
        }
    }
}


void Diagram::fromJson(const QString& filename)
{
    QFile f(filename);
    f.open(QFile::ReadOnly);
    auto contents = f.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(contents);
    fromJson(doc.object());
    f.close();
}

void Diagram::fromJson(const QJsonObject& obj)
{
    //车次和Config直接转发即可
    _trainCollection.fromJson(obj);
    //todo: 默认值的处理
    _config.fromJson(obj.value("config").toObject());
    _note = obj.value("markdown").toString();
    _version = obj.value("version").toString();

    //线路  line作为第一个，lines作为其他，不存在就是空
    auto rail0 = std::make_shared<Railway>(obj.value("line").toObject());
    _railways.append(rail0);

    //其他线路
    const QJsonArray& arrail = obj.value("lines").toArray();
    for (const auto& r : arrail) {
        _railways.append(std::make_shared<Railway>(r.toObject()));
    }

    //特殊：旧版排图标尺 （优先级低于rail中的）
    const auto& t = obj.value("config").toObject().value("ordinate");
    if (t.isString() && !t.toString().isEmpty()) {
        _railways.at(0)->setOrdinate(t.toString());
    }
    bindAllTrains();

}
