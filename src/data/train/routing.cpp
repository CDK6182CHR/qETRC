#include "routing.h"
#include "train.h"
#include "traincollection.h"

#include <algorithm>
#include <QVector>
#include <QString>


void RoutingNode::setName(const QString &name)
{
    _name = name;
}

bool RoutingNode::isVirtual() const
{
    return _virtual;
}


bool RoutingNode::link() const
{
    return _link;
}

void RoutingNode::setLink(bool link)
{
    _link = link;
}

QString RoutingNode::toString() const
{
    QString res = name();
    if (isVirtual())
        res += QObject::tr("(虚拟)");
    return res;
}

bool RoutingNode::operator==(const RoutingNode& other) const
{
    if (_virtual != other._virtual || _link != other._link)
        return false;
    if (_virtual) {
        return _name == other._name;
    }
    else {
        return _train == other._train;
    }
}

bool RoutingNode::operator!=(const RoutingNode& other) const
{
    return !operator==(other);
}

void Routing::setHighlight(bool on)
{
    _highlighted = on;
}

std::shared_ptr<Routing> Routing::copyBase() const
{
    auto res = std::make_shared<Routing>();
    res->_name = _name;
    res->_model = _model;
    res->_owner = _owner;
    res->_note = _note;
    res->_highlighted = _highlighted;
    return res;
}

#define ATTR_SWAP(__name) std::swap(__name,other.__name);

void Routing::swapBase(Routing& other)
{
    ATTR_SWAP(_name);
    ATTR_SWAP(_model);
    ATTR_SWAP(_owner);
    ATTR_SWAP(_note);
    //ATTR_SWAP(_highlighted);
}

#undef ATTR_SWAP

bool Routing::baseEqual(const Routing& other) const
{
    return _name == other._name &&
        _model == other._model &&
        _owner == other._owner &&
        _note == other._note;
}

void Routing::updateTrainHooks()
{
    for (auto p = _order.begin(); p != _order.end(); ++p) {
        if (!p->isVirtual()) {
            p->train()->setRoutingSimple(shared_from_this(), p);
        }
    }
}

void Routing::resetTrainHooks()
{
    for (auto p = _order.begin(); p != _order.end(); ++p) {
        if (!p->isVirtual()) {
            p->train()->resetRoutingSimple();
        }
    }
}

void Routing::swapOrder(Routing& other)
{
    std::swap(_order, other._order);
}

bool Routing::orderEqual(const Routing& other) const
{
    if (_order.size() != other._order.size())
        return false;
    for (auto p1 = _order.begin(), p2 = other._order.begin();
        p1 != _order.end(); ++p1, ++p2) {
        if (p1->operator!=(*p2))
            return false;
    }
    return true;
}

QSet<std::shared_ptr<Train>> Routing::trainSet() const
{
    QSet<std::shared_ptr<Train>> res;
    for (const auto& p : _order) {
        if (!p.isVirtual()) {
            res.insert(p.train());
        }
    }
    return res;
}

QSet<std::shared_ptr<Train>> Routing::takenTrains(const Routing& previous) const
{
    auto s1 = previous.trainSet();
    auto s2 = trainSet();
    s1 -= s2;
    return s1;
}

void Routing::swap(Routing& other)
{
    swapBase(other);
    swapOrder(other);
}

void Routing::parse(TrainCollection& coll, const QString& str, const QString& splitter,
    bool fullOnly, QString& report, const Routing* ignore)
{
    report.append("-------------------\n");
    static const QVector<QString> initSplitters{ "-","~","—","～" };
    auto sp = splitter;
    if (sp.isEmpty()) {
        foreach(const auto & t, initSplitters) {
            if (str.contains(t)) {
                sp = t; break;
            }
        }

    }
    //都不包含，选第一个
    if (sp.isEmpty()) sp = initSplitters.at(0);
    auto names = str.split(sp);
    foreach(const auto & _t, names) {
        QString t = _t.simplified();
        if (t.isEmpty())
            continue;
        parseTrainName(coll, t, fullOnly, report, ignore);
    }
}

bool Routing::parseTrainName(TrainCollection& coll, const QString& t, 
    bool fullOnly, QString& report, const Routing* ignore)
{
    bool flag = false;

    if (t.contains(QObject::tr("检修")) || t.contains(QObject::tr("过夜"))) {
        report.append(QObject::tr("[INFO]    含有关键字[检修]或[过夜]，不作为车次考虑: %1")
            .arg(t));
        return flag;
    }

    if (int idx = trainNameIndexLinear(t); idx != -1) {
        report.append(QObject::tr("[ERROR] 车次[%1]已存在于本交路第[%2]位，不予添加\n")
            .arg(t).arg(idx + 1));
    }
    else {
        auto train = coll.findFullName(t);
        if (!train && !fullOnly) {
            train = coll.findFirstSingleName(t);
        }
        if (train) {
            //找到车次，判定是否属于其他交路
            if (auto idx = trainIndexLinear(train); idx != -1) {
                report.append(QObject::tr("[ERROR]   车次[%1]已经在本次识别中添加到第%2位，"
                    "不能重复添加。\n").arg(train->trainName().full()).arg(idx + 1));
            }
            else if (train->hasRouting() && train->routing().lock().get() != ignore) {
                report.append(QObject::tr("[WARNING] 车次[%1]已经属于交路[%2]，"
                    "设置为虚拟车次。\n").arg(train->trainName().full())
                    .arg(train->routing().lock()->name()));
                appendVirtualTrain(train->trainName().full(), true);
            }
            else {
                //实体车次添加成功
                report.append(QObject::tr("[INFO]    车次[%1]成功识别为实体车次。\n")
                    .arg(train->trainName().full()));
                appendTrainSimple(train, true);
                flag = true;
            }
        }
        else {
            //找不到车次
            report.append(QObject::tr("[WARNING] 找不到车次[%1]，设置为虚拟车次。\n")
                .arg(t));
            appendVirtualTrain(t, true);
        }
    }
    return flag;
}

bool Routing::detectTrains(TrainCollection& coll, bool fullOnly, QString& report, 
    const Routing* ignore)
{
    report += QObject::tr("\n[INFO]    现在在交路[%1]中进行车次识别\n").arg(name());
    bool flag = false;
    for (auto& p : _order) {
        if (p.isVirtual()) {
            // 搜索车次
            auto train = coll.findFullName(p.name());
            if (!train && !fullOnly) {
                train = coll.findFirstSingleName(p.name());
            }
            if (train) {
                if (int idx = trainIndexLinear(train); idx != -1) {
                    report += QObject::tr("[WARNING] 车次[%1]已经在本次识别的交路第[%2]位"
                        "出现过，不能重复，仍为虚拟。\n").arg(train->trainName().full())
                        .arg(idx + 1);
                }
                else if (train->hasRouting()) {
                    auto t = train->routing().lock();
                    report += QObject::tr("[WARNING] 车次[%1]已经属于交路[%2]，仍为虚拟。\n")
                        .arg(train->trainName().full(), t->name());
                }
                else {
                    report += QObject::tr("[INFO]    车次[%1]成功识别为实体车次。\n")
                        .arg(train->trainName().full());
                    p.setTrain(train);
                    flag = true;
                }
            }
            else {
                report += QObject::tr("[WARNING] 找不到车次[%1]，该车次仍为虚拟。\n")
                    .arg(p.name());
            }
        }
        else {  // not virtual
            //实体车次检查一下
            auto t = p.train();
            if (!t->hasRouting()) {
                report += QObject::tr("[ERROR]   车次[%1]列车端没有交路信息，与交路端数据冲突。"
                    "车次被改置为虚拟。这可能是由于程序内部错误。\n").arg(t->trainName().full());
                p.makeVirtual();
                flag = true;
            }
            else if (auto rt = t->routing().lock();
                rt && rt.get() != this && rt.get() != ignore) {
                report += QObject::tr("[ERROR]   车次[%1]列车端的交路信息指向交路[%2]，"
                    "与当前交路不一致，在当前交路中被设置为虚拟。这可能由于程序内部错误。\n")
                    .arg(t->trainName().full(), rt->name());
                flag = true;
            }
        }
    }
    return flag;
}

int Routing::virtualCount() const
{
    int cnt = 0;
    for (const auto& p : _order) {
        if (p.isVirtual())
            ++cnt;
    }
    return cnt;
}

int Routing::realCount() const
{
    int cnt = 0;
    for (const auto& p : _order) {
        if (!p.isVirtual())
            ++cnt;
    }
    return cnt;
}

bool Routing::checkForDiagram(QString &report) const
{
    bool flag=true;
    for(const auto& node:_order){
        if (node.isVirtual()){
            flag=false;
            report.append(QObject::tr("交路节点[%1]为虚拟节点，无法获得时刻信息\n").arg(node.name()));
        }else{
            auto train=node.train();
            if (train->empty()){
                report.append(QObject::tr("交路车次[%1]时刻表为空，无法确定时刻信息\n")
                              .arg(train->trainName().full()));
                continue;
            }
            //auto* s=train->boundStarting();
            //2023.05.15: change the criterion from boundStarting() to "starting station is the first station in timetable"
            if (!train->timetable().front().name.equalOrBelongsTo(train->starting())) {
                flag=false;
                report.append(QObject::tr("交路车次[%1]缺失始发站[%2]的时刻信息；时刻表首站为[%3]\n")
                    .arg(train->trainName().full(),
                        train->starting().toSingleLiteral(),
                        train->timetable().front().name.toSingleLiteral()));
            }
            //auto* t=train->boundTerminal();
            if(!train->timetable().back().name.equalOrBelongsTo(train->terminal())){
                flag=false;
                report.append(QObject::tr("交路车次[%1]缺失终到站[%2]的时刻信息；时刻表末站为[%3]\n")
                    .arg(train->trainName().full(),
                        train->terminal().toSingleLiteral(),
                        train->timetable().back().name.toSingleLiteral()));
            }
        }
    }
    return flag;
}

RoutingNode::RoutingNode(const QString &name, bool link):
    _name(name),_virtual(true),_link(link)
{

}

RoutingNode::RoutingNode(std::shared_ptr<Train> train, bool link):
    _train(train),_name(train->trainName().full()),_virtual(false),_link(link)
{

}

#define TOJSON(_key) {#_key,_##_key},
QJsonObject RoutingNode::toJson() const
{
    //start, end暂时不管
    return QJsonObject{
        {"start",""},
        {"end",""},
        {"checi",_name},
        TOJSON(link)
        TOJSON(virtual)
    };
}

std::shared_ptr<Train> RoutingNode::train() const
{
    return _train;
}

void RoutingNode::setTrain(const std::shared_ptr<Train> &train)
{
    _train = train;
    _virtual = false;
}

void RoutingNode::makeVirtual()
{
    _train = nullptr;
    _virtual = true;
}

const QString &RoutingNode::name() const
{
    return _virtual?_name:_train->trainName().full();
}


//Routing& Routing::operator=(Routing&& other) noexcept
//{
//    _order = std::move(other._order);
//    _name = std::move(other._name);
//    _note = std::move(other._note);
//    _model = std::move(other._model);
//    _owner = std::move(other._owner);
//    //安全起见，检查一下
//    for (auto p =_order.begin();p!=_order.end();++p) {
//        if (!p->isVirtual()) {
//            qDebug() << "Routing::operator=: WARNING: Unexpected non-virtual node: "
//                << p->name() << " in " << _name << Qt::endl;
//        }
//    }
//    return *this;
//}

Routing::Routing(const Routing& other):
    std::enable_shared_from_this<Routing>(other)
{
    _name = other._name;
    _model = other._model;
    _owner = other._owner;
    _note = other._note;
    _highlighted = other._highlighted;
    for (const auto& p : other._order) {
        _order.emplace_back(p);    // copy contruct;
    }
}

void Routing::fromJson(const QJsonObject& obj, TrainCollection& coll)
{
    _name = obj.value("name").toString();
    _note = obj.value("note").toString();
    _model = obj.value("model").toString();
    _owner = obj.value("owner").toString();
    const QJsonArray& ar = obj.value("order").toArray();
    for (auto p = ar.begin(); p != ar.end(); ++p) {
        const QJsonObject obn = p->toObject();
        bool v = obn.value("virtual").toBool();
        bool link = obn.value("link").toBool();
        //pyETRC中的start, end好像暂时没用
        //写文件的时候处理一下；其他时候就不管了
        if (v) {
            //直接添加空白的Node
            appendVirtualTrain(obn.value("checi").toString(), link);
        }
        else {
            appendTrainFromName(obn.value("checi").toString(), link, coll);
        }
    }
}

QJsonObject Routing::toJson() const
{
    QJsonArray ar;
    for (auto p = _order.begin(); p != _order.end(); ++p) {
        ar.append(p->toJson());
    }
    return QJsonObject{
        {"order",ar},
        TOJSON(name)
        TOJSON(note)
        TOJSON(model)
        TOJSON(owner)
    };
}

bool Routing::appendTrain(std::shared_ptr<Train> train, bool link)
{
    if (train->hasRouting())
        return false;
    _order.emplace_back(train, link);
    auto it = _order.end(); --it;
    train->setRouting(shared_from_this(), it);
    return true;
}

void Routing::appendTrainSimple(std::shared_ptr<Train> train, bool link)
{
    _order.emplace_back(train, link);
}

bool Routing::appendTrainFromName(const QString& trainName, bool link, 
    TrainCollection& _coll)
{
    auto t = _coll.findFullName(trainName);
    if (!t) {
        qDebug() << "Routing::appendTrainFromName: WARNING: TrainName not found: " << trainName <<
            ", will use virtual." << Qt::endl;
        appendVirtualTrain(trainName, link);
        return false;
    }
        
    else if (t->hasRouting()) {
        qDebug() << "Routing::appendTrainFromName: WARNING: train " << trainName << " already has a routing, "
            << "will use virtual. " << Qt::endl;
        appendVirtualTrain(trainName, link);
        return false;
    }
        
    _order.emplace_back(t, link);
    auto it = _order.end(); --it;
    t->setRouting(shared_from_this(), it);
    return true;
}

bool Routing::appendVirtualTrain(const QString& trainName, bool link)
{
    _order.emplace_back(trainName, link);
    return true;
}

QString Routing::orderString() const
{
    if (_order.empty())
        return "";
    auto p = _order.begin();
    QString res = p->toString();
    for (++p; p != _order.end(); ++p) {
        res += "-" + p->toString();
    }
    return res;
}

RoutingNode* Routing::preLinked(const Train& train)
{
    if (!train.hasRouting() || train.routing().lock().get() != this)
        return nullptr;
    auto it = train.routingNode().value_or(_order.end());
    if (it == _order.begin()||it==_order.end())
        return nullptr;
    auto pre = std::prev(it);
    if (pre->isVirtual())
        return nullptr;
    auto preend = pre->train()->boundTerminalRail();
    auto curstart = train.boundStartingRail();
    if (preend == curstart && curstart) {
        return &(*pre);
    }
    return nullptr;
}

RoutingNode* Routing::postLinked(const Train& train)
{
    if (!train.hasRouting() || train.routing().lock().get() != this)
        return nullptr;
    auto it = train.routingNode().value_or(_order.end());
    if (it == _order.end())
        return nullptr;
    auto post = std::next(it);
    if (post == _order.end() ||  post->isVirtual())
        return nullptr;
    auto curend = train.boundTerminalRail();
    auto poststart = post->train()->boundStartingRail();
    if (curend && curend == poststart) {
        return &(*post);
    }
    return nullptr;
}

QString Routing::preOrderString(const Train &train) 
{
    if (!train.hasRouting() || train.routing().lock().get() != this)
        return {};
    auto it = train.routingNode().value_or(_order.end());
    if (it == _order.begin()||it==_order.end())
        return {};
    auto pre = std::prev(it);
    if (pre->isVirtual())
        return QObject::tr("%1 (否)").arg(pre->name());
    auto preend = pre->train()->boundTerminalRail();
    auto curstart = train.boundStartingRail();
    if (preend == curstart && curstart) {
        return QObject::tr("%1 (是)").arg(pre->name());
    }
    return QObject::tr("%1 (否)").arg(pre->name());
}

QString Routing::postOrderString(const Train &train) 
{
    if (!train.hasRouting() || train.routing().lock().get() != this)
        return {};
    auto it = train.routingNode().value_or(_order.end());
    if (it == _order.end())
        return {};
    auto post = std::next(it);
    if (post == _order.end())
        return "-";
    else if(post->isVirtual()){
        return QObject::tr("%1 (否)").arg(post->name());
    }
    auto curend = train.boundTerminalRail();
    auto poststart = post->train()->boundStartingRail();
    if (curend && curend == poststart) {
        // link
        return QObject::tr("%1 (是)").arg(post->name());
    }
    return QObject::tr("%1 (否)").arg(post->name());
}

void Routing::print() const
{
    qDebug() << "Routing [" << name() << "]: " << orderString() << Qt::endl;
}

bool Routing::anyValidTrains() const
{
    for (auto p = _order.begin(); p != _order.end(); ++p) {
        if (!p->isVirtual())
            return true;
    }
    return false;
}

void Routing::replaceTrain(std::shared_ptr<Train> oldTrain, std::shared_ptr<Train> newTrain)
{
    if (!oldTrain->hasRouting() || oldTrain->routing().lock().get() != this) {
        qDebug() << "Routing::replaceTrain: WARNING: Invalid call: routing not complicatable. "
            << Qt::endl;
        return;
    }
    auto t = oldTrain->routingNode().value();
    oldTrain->resetRouting();
    t->setTrain(newTrain);
    newTrain->setRouting(shared_from_this(), t);  //包含了setRouting操作
}

void Routing::setNodeTrain(std::shared_ptr<Train> train, std::list<RoutingNode>::iterator node)
{
    node->setTrain(train);
    train->setRouting(shared_from_this(), node);
}

int Routing::trainIndex(std::shared_ptr<Train> train) const
{
    if (train->hasRouting() && train->routing().lock().get() == this) {
        return std::distance(_order.cbegin(), static_cast<CNodePtr>(train->routingNode().value()));
    }
    return -1;
}

int Routing::trainIndexLinear(std::shared_ptr<Train> train) const
{
    int i = 0;
    for (auto p = _order.begin(); p != _order.end(); ++p, ++i) {
        if (!p->isVirtual()&& p->train() == train)
            return i;
    }
    return -1;
}

int Routing::trainNameIndexLinear(const QString& trainName) const
{
    int i = 0;
    for (auto p = _order.begin(); p != _order.end(); ++p, ++i) {
        if (p->name() == trainName)
            return i;
    }
    return -1;
}


