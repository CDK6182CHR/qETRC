#pragma once
#include <memory>
#include <deque>
#include <QJsonObject>

#include "trainname.h"
#include "itrainfilter.h"

class TrainCollection;
class Train;

struct TrainGroupItem{
    std::weak_ptr<Train> train;

    TrainGroupItem(const std::shared_ptr<Train>& train);
    TrainGroupItem(const TrainName& name);
    const TrainName& name()const;
private:
    TrainName _name;
};

/**
 * @brief The TrainGroup class is a set of train,
 * typically used as filter.
 * A TrainGroup lists a set of trains, which may be inclusive or exclusive.
 * The trains are specified by weak_ptr<Train> in run time,
 * and i/o with JSON using train names.
 * The TrainName must be stored, to keep track of train name ALL THE TIME.
 * In the first version, we just implement the linear search.
 */
class TrainGroup: public ITrainFilter
{
    QString _name;
    QString _note;
    std::deque<std::unique_ptr<TrainGroupItem>> _items;
public:
    TrainGroup()=default;
    auto& items(){return _items;}
    const auto& items()const{return _items;}
    const auto& name()const{return _name;}
    const auto& note()const{return _note;}
    void setName(const QString& n){_name=n;}
    void setNote(const QString& n){_note=n;}

    virtual bool check(std::shared_ptr<const Train> train)const override;

    void fromJson(const QJsonObject& obj, TrainCollection& coll);
    QJsonObject toJson()const;

    void addItem(const TrainName& name);
    void addItem(std::shared_ptr<Train> train);

private:
    bool linearSearch(std::shared_ptr<const Train> train)const;
};

