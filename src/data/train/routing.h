#pragma once

#include <list>
#include <QJsonObject>
#include "train.h"

/**
 * @brief The RoutingNode class 交路中的一个结点 pyETRC.data.CircuitNode
 * 可能是虚拟的或者持有实际列车对象
 * 列车暂定采用std::shared_ptr保存
 * 
 * 在两车次之间连线，当且仅当：
 * （1）用户勾选了连线；
 * （2）前一车次终到站、后一车次始发站为本线同一车站。
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
    RoutingNode(std::shared_ptr<Train> train, bool link);

    RoutingNode(const RoutingNode&)=default;
    RoutingNode(RoutingNode&&)noexcept=default;
    RoutingNode& operator=(const RoutingNode&)=default;
    RoutingNode& operator=(RoutingNode&&)noexcept=default;

    QJsonObject toJson()const;

    std::shared_ptr<Train> train() const;

    /**
     * 同时设置为非虚拟
     * 但不包含对列车那边的操作 （因为自己拿不到自己的迭代器）
     */
    void setTrain(const std::shared_ptr<Train> &train);

    /**
     * 清除已有的列车指向，同时设置为虚拟结点
     * 删除列车时，由TrainCollection调用
     */
    void makeVirtual();

    const QString& name() const;
    void setName(const QString &name);
    bool isVirtual() const;
    //void setVirtual(bool _virtual);
    bool link() const;
    void setLink(bool link);

    QString toString()const;
};


class TrainCollection;

/**
 * @brief Routing 车底交路类 pyETRC.data.Circuit
 * 使用std::list，列车那边持有这里面的迭代器
 */
class Routing:
    public std::enable_shared_from_this<Routing>
{
    TrainCollection& _coll;
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

    /**
     * 2021.07.04 准备实现  导入交路时使用
     * 在不同的交路间拷贝。注意这个时候原则上应该全都是虚拟车次。
     * 相当于原来的coverBaseData
     */
    Routing& operator=(Routing&& other)noexcept;

    /**
     * 接受pyETRC格式的circuits数组元素
     */
    void fromJson(const QJsonObject& obj);
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
    bool appendTrain(std::shared_ptr<Train> train, bool link);

    /**
     * 读JSON的操作  返回是否成功
     * 如果失败，直接加虚拟结点
     */
    bool appendTrainFromName(const QString& trainName, bool link);

    bool appendVirtualTrain(const QString& trainName, bool link);

    /**
     * @brief pyETRC.Circuit.changeTrainToVirtual()
     * 删除车次时，默认情况下把该车次的结点变成虚拟的，而不是删除它。
     * 在这种意义下，删除结点的操作好像不太用得到。
     */
    void makeVirtual(std::shared_ptr<Train> train);

    QString orderString()const;

    /**
     * @brief 如果前序车能够连上，那么返回该节点  pyETRC.data.Circuit.preorderLinked
     * 否则返回空
     */
    RoutingNode* preLinked(const Train& train);

    RoutingNode* postLinked(const Train& train);

    void print()const;

    /**
     * 是否有至少一个非虚拟车次
     */
    bool anyValidTrains()const;

    /**
     * 替换列车。使用newTrain将结点中的oldTrain替换掉。
     * 其中oldTrain逻辑上是xvalue，此操作同时清除其中对routing的引用。
     * 同时将本交路引用交给newTrain.
     * 已知oldTrain本来是属于本交路的。
     */
    void replaceTrain(std::shared_ptr<Train> oldTrain, std::shared_ptr<Train> newTrain);

    /**
     * 将node所示位置的列车设置为train。
     * 如果node以前存在列车，将其释放掉。
     * 此操作由ImportTrain调用。不放在Train或者Node里面，是因为shared_from_this总是出错
     */
    void setNodeTrain(std::shared_ptr<Train> train, std::list<RoutingNode>::iterator node);

    /**
     * 返回列车在本交路中的索引编号。线性复杂度。
     * 如果列车不在本交路中，返回-1
     */
    int trainIndex(std::shared_ptr<Train> train)const;
};

