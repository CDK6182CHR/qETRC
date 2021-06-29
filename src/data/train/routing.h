#pragma once

#include <list>
#include "train.h"

/**
 * @brief The RoutingNode class 交路中的一个结点 pyETRC.data.CircuitNode
 * 可能是虚拟的或者持有实际列车对象
 * 列车暂定采用std::shared_ptr保存
 */
class RoutingNode{
    std::shared_ptr<Train> _train;
    QString _name;
    bool _virtual;
    bool _link;

public:
    /**
     * @brief 仅有车次，创建虚拟交路结点
     */
    RoutingNode(const QString& name, bool link);

    /**
     * @brief 具有列车对象，创建实际交路结点
     */
    RoutingNode(std::shared_ptr<Train> train,bool link);

    RoutingNode(const RoutingNode&)=default;
    RoutingNode(RoutingNode&&)noexcept=default;
    RoutingNode& operator=(const RoutingNode&)=default;
    RoutingNode& operator=(RoutingNode&&)noexcept=default;

    std::shared_ptr<Train> train() const;
    void setTrain(const std::shared_ptr<Train> &train);
    const QString& name() const;
    void setName(const QString &name);
    bool isVirtual() const;
    void setVirtual(bool _virtual);
    bool link() const;
    void setLink(bool link);
};


class TrainCollection;

/**
 * @brief Routing 车底交路类 pyETRC.data.Circuit
 * 使用std::list，列车那边持有这里面的迭代器
 */
class Routing:
    public std::enable_shared_from_this<Routing>
{
    TrainCollection& coll;
    std::list<RoutingNode> _order;
    QString _name, _note, _model, _owner;

public:

    using NodePtr=std::list<RoutingNode>::iterator;
    using CNodePtr=std::list<RoutingNode>::const_iterator;

    /**
     * @brief Routing
     * 注意不能从构造函数读JSON，因为涉及到shared_from_this
     */
    Routing(TrainCollection& coll);

    Routing(const Routing&)=default;
    Routing(Routing&&)noexcept=default;
    Routing& operator=(const Routing&)=delete;
    Routing& operator=(Routing&&)=delete;

    void fromJson(const QJsonObject& obj,TrainCollection& coll);
    QJsonObject toJson()const;

    inline const QString& name()const{return _name;}
    inline const QString& note()const{return _note;}
    inline const QString& model()const{return _model;}
    inline const QString& owner()const{return _owner;}
    inline void setName(const QString& s){_name=s;}
    inline void setNote(const QString& s){_note=s;}
    inline void setModel(const QString& s){_model=s;}
    inline void setOwner(const QString& s){_owner=s;}

    inline auto& order(){return _order;}
    inline const auto& order()const{return _order;}

    inline int count()const{return _order.size();}

    /**
     * @brief 添加列车
     * 注意同时向列车设置交路信息
     * 注意列车不能属于其他交路，否则添加失败
     */
    bool appendTrain(std::shared_ptr<Train> train);

    /**
     * @brief pyETRC.Circuit.changeTrainToVirtual()
     * 删除车次时，默认情况下把该车次的结点变成虚拟的，而不是删除它。
     * 在这种意义下，删除结点的操作好像不太用得到。
     */
    void makeVirtual(std::shared_ptr<Train> train);

    QString orderString()const;

    /**
     * @brief 如果前序车能够连上，那么返回该节点
     * 否则返回空
     */
    RoutingNode* preLinked(const Train& train);

    RoutingNode* postLinked(const Train& train);

    //解析车次等 下次
};

