#pragma once

#include <QString>
#include <QMap>
#include <QList>
#include <QVector>
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
    QPen _pen;
    bool _passenger;
public:
    TrainType(const QString& name, const QPen& pen, bool passenger=false) :
        _name(name), _pen(pen),_passenger(passenger){}

    const QPen& pen()const{return _pen;}
    QPen& pen() { return _pen; }

    bool isPassenger()const { return _passenger; }
    void setIsPassenger(bool p) { _passenger = p; }

    const QString& name()const { return _name; }

    bool operator==(const TrainType& other)const;
    bool operator!=(const TrainType& other)const;

    void swap(TrainType& other);
};



/**
 * @brief The TypaManager class
 * 列车类型集合，包含判定类型，判定UI信息等
 */
class TypeManager{

    using TypeSet=QMap<QString,std::shared_ptr<TrainType>>;

    /**
     * @brief _types
     * 逻辑上，这里作为类型的容器。注意，原则上，所有的类型都应该属于这个表。
     * 使用Map，原因是希望保持一定的迭代顺序
     */
    QMap<QString,std::shared_ptr<TrainType>> _types;

    /**
     * @brief _regs
     * 按顺序进行正则匹配
     */
    QVector<QPair<QRegExp, std::shared_ptr<TrainType>>> _regs;

    /**
     * 默认的版本。注意这个不应该是static，因为系统Config和默认Config指定的默认颜色可能不同。
     * 2021.08.13  增加一个默认客车用的pen。如果能确定是客车，就先用这个
     */
    QPen defaultPen, defaultPenPassenger;
    std::shared_ptr<TrainType> defaultType;

public:
    TypeManager();
    TypeManager(const TypeManager&)=delete;
    TypeManager(TypeManager&&)noexcept=default;

    /**
     * 拷贝赋值 手写 复制所有类型对象。
     */
    TypeManager& operator=(const TypeManager& another);

    TypeManager& operator=(TypeManager&&)noexcept=default;

    const auto& types()const { return _types; }
    auto& typesRef() { return _types; }

    /**
     * 读取系统默认配置 config.json
     * 如果读取失败，采用内置参数
     */
    void readForDefault(const QJsonObject& obj);

    /**
     * 读取运行图的设置。如果读取失败，采用默认设置
     */
    void readForDiagram(const QJsonObject& obj, const TypeManager& defaultManager);

    void initDefaultTypes();
    
    /**
     * obj所示为具有Config格式的对象。
     */
    void toJson(QJsonObject& obj)const;

    /**
     * @brief 添加新的类型
     */
    std::shared_ptr<TrainType> addType(const QString& name,
                                           const QPen& pen);

    void appendRegex(const QRegExp& reg, const QString& name);

    /**
     * 用来读pyETRC的配置数据，同时写进是否客车的数据
     */
    std::shared_ptr<TrainType> appendRegex(const QRegExp& reg, const QString& name, bool passenger);

    std::shared_ptr<TrainType> fromRegex(const TrainName& name)const;

    /**
     * @brief findOrCreate
     * 按类型名查找类对象，如果不存在，则创建并添加进去。
     * 创建新的UI时，复制默认的。
     */
    std::shared_ptr<TrainType> findOrCreate(const QString& name);

    /**
     * 考虑是否为客车的版本
     * 如果有信息指明为客车，则优先用客车pen
     */
    std::shared_ptr<TrainType> findOrCreate(const QString& name, bool passenger);

    /**
     * 用于导入车次：根据名称查找类型；
     * 如果类型不存在，加入入参的【拷贝】到类型表（深拷贝）
     */
    std::shared_ptr<TrainType> findOrCreate(std::shared_ptr<TrainType> tp);

    /**
     * 如果找不到，返回空
     */
    std::shared_ptr<TrainType> find(const QString& name)const { return _types.value(name); }

    inline bool isNull()const { return _types.isEmpty(); }

    const QPen& getDefaultPen()const{return defaultPen;}

    auto getDefaultType()const{return defaultType;}

    auto& regex()const { return _regs; }

    auto& regexRef() { return _regs; }

    /**
     * 交换_types和_regs；都是浅拷贝。
     */
    void swapForRegex(TypeManager& other);

private:
    /**
     * 输入格式是pyETRC的config.json或者graph中config对象
     * 返回是否成功
     */
    bool fromJson(const QJsonObject& obj);

    
};

Q_DECLARE_METATYPE(std::shared_ptr<TrainType>);


