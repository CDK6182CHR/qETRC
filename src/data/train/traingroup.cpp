#include "traingroup.h"

#include <QJsonArray>

#include "train.h"
#include "traincollection.h"

TrainGroupItem::TrainGroupItem(const std::shared_ptr<Train> &train):
    train(train), _name(train->trainName())
{

}

TrainGroupItem::TrainGroupItem(const TrainName &name):
    train(), _name(name)
{

}

const TrainName &TrainGroupItem::name() const
{
    if (train.expired()){
        return _name;
    } else{
        return train.lock()->trainName();
    }
}

bool TrainGroup::check(std::shared_ptr<const Train> train) const
{
    return linearSearch(train);
}

void TrainGroup::fromJson(const QJsonObject &obj, TrainCollection &coll)
{
    _name=obj.value("name").toString();
    _note=obj.value("note").toString();
    const auto& arr=obj.value("trains").toArray();
    foreach(const auto& v, arr){
        TrainName name(v.toString());
        if (auto train=coll.findFullName(name)){
            addItem(train);
        }else{
            addItem(name);
        }
    }
}

QJsonObject TrainGroup::toJson() const
{
    QJsonArray arr;
    for (const auto& it:_items){
        arr.append(it->name().full());
    }
    return QJsonObject{
        {"name",_name},
        {"note",_note},
        {"trains",arr}
    };
}

void TrainGroup::addItem(const TrainName &name)
{
    _items.emplace_back(std::make_unique<TrainGroupItem>(name));
}

void TrainGroup::addItem(std::shared_ptr<Train> train)
{
    _items.emplace_back(std::make_unique<TrainGroupItem>(train));
}

bool TrainGroup::linearSearch(std::shared_ptr<const Train> train) const
{
    for (const auto& it:_items){
        if (!it->train.expired() && it->train.lock() == train)
            return true;
    }
    return false;
}
