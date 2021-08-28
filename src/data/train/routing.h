#pragma once

#include <list>
#include <QSet>
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

    /**
     * 信息等价判定，virtual时只考虑name，否则只考虑train
     */
    bool operator==(const RoutingNode& other)const;
    bool operator!=(const RoutingNode& other)const;
};


class TrainCollection;

/**
 * @brief Routing 车底交路类 pyETRC.data.Circuit
 * 使用std::list，列车那边持有这里面的迭代器
 * 2021.08.26：取消对coll的反向引用。
 */
class Routing:
    public std::enable_shared_from_this<Routing>
{
    std::list<RoutingNode> _order;
    QString _name, _note, _model, _owner;
    bool _highlighted = false;

public:

    using NodePtr=std::list<RoutingNode>::iterator;
    using CNodePtr=std::list<RoutingNode>::const_iterator;

    /**
     * @brief Routing
     * 注意不能从构造函数读JSON，因为涉及到shared_from_this
     */
    Routing() = default;

    /**
     * 2021.08.28给出实现，深拷贝语义。
     * 用于交路车次识别。当前对象【不获得】车次管理权（车次那边的引用不变）
     */
    Routing(const Routing& other);

    Routing(Routing&&)noexcept=default;
    Routing& operator=(const Routing&)=delete;

    /**
     * 2021.07.04 准备实现  导入交路时使用
     * 在不同的交路间拷贝。注意这个时候原则上应该全都是虚拟车次。
     * 相当于原来的coverBaseData
     * 2021.08.26 删除collection反向引用之后，可以使用默认的移动
     */
    Routing& operator=(Routing&& other)noexcept = default;

    /**
     * 接受pyETRC格式的circuits数组元素
     */
    void fromJson(const QJsonObject& obj, TrainCollection& coll);
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

    inline int count()const { return static_cast<int>(_order.size()); }

    /**
     * @brief 添加列车
     * 注意同时向列车设置交路信息
     * 注意列车不能属于其他交路，否则添加失败
     */
    bool appendTrain(std::shared_ptr<Train> train, bool link);

    /**
     * 添加列车，用在交路编辑的提交页面，生成临时对象。
     * 只添加列车的引用到本类，不更新列车中对交路的引用。
     * 由外界保证合法性。
     */
    void appendTrainSimple(std::shared_ptr<Train> train, bool link);

    /**
     * 读JSON的操作  返回是否成功
     * 如果失败，直接加虚拟结点
     */
    bool appendTrainFromName(const QString& trainName, bool link, TrainCollection& coll);

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
     * 返回列车在本交路中的索引编号。利用了Train中的Hook信息，常数复杂度
     */
    int trainIndex(std::shared_ptr<Train> train)const;
    
    /**
     * 线性查找。 
     */
    int trainIndexLinear(std::shared_ptr<Train> train)const;

    /**
     * 线性查找车次；虚拟、实体都查找
     */
    int trainNameIndexLinear(const QString& trainName)const;


    /**
     * 仅仅是维护状态。这个状态不写入文件。
     */
    void setHighlight(bool on);

    bool isHighlighted()const { return _highlighted; }

    /**
     * 复制基础数据（不包括序列）
     */
    std::shared_ptr<Routing> copyBase()const;

    void swapBase(Routing& other);

    /**
     * 对比基本信息是否一致
     */
    bool baseEqual(const Routing& other)const;

    /**
     * 更新所管理的所有列车中，对自身（以及迭代器）的引用。
     * 用在应用order更改后。
     */
    void updateTrainHooks();

    /**
     * 删除列车中指向自身的引用。
     * 用于删除（撤销添加）交路的操作。
     */
    void resetTrainHooks();

    /**
     * 交换套跑次序信息
     * 暂定直接交换；RoutingNode的地址会变化
     * 注意没有更新列车的所有权 （列车中对交路的反向指针不变！！）
     * seealso: updateTrainHooks()
     */
    void swapOrder(Routing& other);

    /**
     * 判定套跑次序是否一致
     */
    bool orderEqual(const Routing& other)const;

    /**
     * 所有包含列车的集合
     */
    QSet<std::shared_ptr<Train>> trainSet()const;

    /**
     * 与此前的版本相比，被拿掉的列车的集合
     * 这些列车运行线需要单独更新。
     */
    QSet<std::shared_ptr<Train>> takenTrains(const Routing& previous)const;

    void swap(Routing& other);

    bool empty()const { return _order.empty(); }

    /**
     * 解析字符串，然后附加到最后。
     */
    void parse(TrainCollection& coll, const QString& str, const QString& splitter,
        bool fullOnly, QString& report, const Routing* ignore);

    /**
     * 用在解析字符串以及识别车次中，识别指定的单个车次
     * 返回：是否识别为real train
     */
    bool parseTrainName(TrainCollection& coll, const QString& name, bool fullOnly,
        QString& report, const Routing* ignore);

    /**
     * 识别车次：对本交路所有虚拟车次，尝试找出Train。采用简单添加（不更新hook）的算法
     * `ignore` 给出允许重复的交路，也就是this本身对应的那个交路。
     * 同时检查非虚拟车次是否有问题
     * 如果列车属于该交路，那么不报错
     * 返回：是否改变
     */
    bool detectTrains(TrainCollection& coll, bool fullOnly, QString& report,
        const Routing* ignore);

    int virtualCount()const;
    int realCount()const;
};

