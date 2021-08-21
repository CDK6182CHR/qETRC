#include "routing.h"
#include "data/train/traincollection.h"

#include <algorithm>


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

Routing::Routing(TrainCollection& coll):
    _coll(coll)
{
}

Routing& Routing::operator=(Routing&& other) noexcept
{
    _order = std::move(other._order);
    _name = std::move(other._name);
    _note = std::move(other._note);
    _model = std::move(other._model);
    _owner = std::move(other._owner);
    //安全起见，检查一下
    for (auto p =_order.begin();p!=_order.end();++p) {
        if (!p->isVirtual()) {
            qDebug() << "Routing::operator=: WARNING: Unexpected non-virtual node: "
                << p->name() << " in " << _name << Qt::endl;
        }
    }
    return *this;
}

void Routing::fromJson(const QJsonObject& obj)
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
            appendTrainFromName(obn.value("checi").toString(), link);
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

bool Routing::appendTrainFromName(const QString& trainName, bool link)
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
    auto pre = it; --pre;
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
    auto post = it; ++post;
    if (post == _order.end())
        return nullptr;
    auto curend = train.boundTerminalRail();
    auto poststart = post->train()->boundStartingRail();
    if (curend && curend == poststart) {
        return &(*post);
    }
    return nullptr;
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
