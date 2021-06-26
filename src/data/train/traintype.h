#pragma once

#include <QString>
#include <QHash>
#include <QList>
#include <memory>
#include <Qt>
#include <QRegExp>
#include <QColor>
#include <QPen>

/**
 * @brief The TrainUI struct  pyETRC.data.train.UI dict
 * 列车铺画的运行线样式设定，作为Train或者TrainType的成员，以shared_ptr形式
 * 具体来说，如果Train采用默认，那么直接和TrainType里面的share；
 * 如果Train采用手工构造，则Train自己管理一个对象
 */
struct TrainUI {
    QColor color{0,128,0};
    double lineWidth{1.0};
    Qt::PenStyle lineStyle{Qt::SolidLine};
    QPen toQPen()const;
    static TrainUI defaultUI;

    TrainUI()=default;
    TrainUI(const TrainUI&)=default;
};



/**
 * pyETRC中无对应抽象
 * 封装与列车类型有关的所有数据
 * UI作为const指针，全程只能指向同一个对象，但可以修改内容
 */
class TrainType
{
    QString _name;
    const std::shared_ptr<TrainUI> _ui;
public:
    TrainType(const QString& name, std::shared_ptr<TrainUI> ui);

    std::shared_ptr<TrainUI> ui(){return _ui;}
    std::shared_ptr<const TrainUI> ui()const{return _ui;}
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

public:
    TypeManager()=default;
    TypeManager(const TypeManager&)=delete;
    TypeManager(TypeManager&&)=delete;
    TypeManager& operator=(const TypeManager&)=delete;
    TypeManager& operator=(TypeManager&&)=delete;

    void fronJson(const QJsonObject& obj);
    QJsonObject toJson()const;

    /**
     * @brief 原位构造新的类型
     */
    std::shared_ptr<TrainType> emplaceType(const QString& name,
                                           const QRegExp& reg,
                                           std::shared_ptr<TrainUI> ui);

    std::shared_ptr<const TrainType> fromRegex(const QRegExp& reg)const;

    /**
     * @brief findOrCreate
     * 按类型名查找类对象，如果不存在，则创建并添加进去。
     * 创建新的UI时，复制默认的。
     */
    std::shared_ptr<TrainType> findOrCreate(const QString& name);
};


