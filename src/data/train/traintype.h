#pragma once

#include <QString>
#include <QHash>
#include <QList>
#include <memory>
#include <Qt>
#include <QRegExp>
#include <QColor>
#include <QPen>

#include "data/train/trainname.h"



/**
 * pyETRC中无对应抽象
 * 封装与列车类型有关的所有数据
 * UI作为const指针，全程只能指向同一个对象，但可以修改内容
 */
class TrainType
{
    QString _name;
    const std::shared_ptr<QPen> _pen;
    bool _passenger;
public:
    TrainType(const QString& name, const QPen& pen, bool passenger=false) :
        _name(name), _pen(std::make_shared<QPen>(pen)),_passenger(passenger){}

    std::shared_ptr<QPen> pen(){return _pen;}
    std::shared_ptr<const QPen> pen()const{return _pen;}

    bool isPassenger()const { return _passenger; }
    void setIsPassenger(bool p) { _passenger = p; }

    const QString& name()const { return _name; }
};



/**
 * @brief The TypaManager class
 * 列车类型集合，包含判定类型，判定UI信息等
 */
class TypeManager{

    /**
     * @brief _types
     * 逻辑上，这里作为类型的容器。注意，原则上，所有的类型都应该属于这个表。
     */
    QHash<QString,std::shared_ptr<TrainType>> _types;

    /**
     * @brief _regs
     * 按顺序进行正则匹配
     */
    QList<QPair<QRegExp,std::shared_ptr<TrainType>>> _regs;

    static QPen defaultPen;
    static std::shared_ptr<TrainType> defaultType;

public:
    //todo: copy assign, for default assign
    TypeManager() {
        //TODO: 临时版本：采用默认
        initDefaultTypes();
    }
    TypeManager(const TypeManager&)=delete;
    TypeManager(TypeManager&&)=default;
    TypeManager& operator=(const TypeManager&)=delete;
    TypeManager& operator=(TypeManager&&)=default;

    void readForDefault(const QJsonObject& obj);
    void readForDiagram(const QJsonObject& obj, const TypeManager& defaultManager);
    
    QJsonObject toJson()const;

    /**
     * @brief 添加新的类型
     */
    std::shared_ptr<TrainType> addType(const QString& name,
                                           const QPen& pen);

    void appendRegex(const QRegExp& reg, const QString& name);

    /**
     * 用来读pyETRC的配置数据，同时写进是否客车的数据
     */
    void appendRegex(const QRegExp& reg, const QString& name, bool passenger);

    std::shared_ptr<const TrainType> fromRegex(const TrainName& name)const;

    /**
     * @brief findOrCreate
     * 按类型名查找类对象，如果不存在，则创建并添加进去。
     * 创建新的UI时，复制默认的。
     */
    std::shared_ptr<TrainType> findOrCreate(const QString& name);

private:
    void fromJson(const QJsonObject& obj);

    void initDefaultTypes();
};


