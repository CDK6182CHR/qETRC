#include "routing.h"


void RoutingNode::setName(const QString &name)
{
    _name = name;
}

bool RoutingNode::isVirtual() const
{
    return _virtual;
}

void RoutingNode::setVirtual(bool virtual_)
{
    _virtual = virtual_;
}

bool RoutingNode::link() const
{
    return _link;
}

void RoutingNode::setLink(bool link)
{
    _link = link;
}

RoutingNode::RoutingNode(const QString &name, bool link):
    _name(name),_virtual(true),_link(link)
{

}

RoutingNode::RoutingNode(std::shared_ptr<Train> train, bool link):
    _train(train),_name(train->trainName().full()),_virtual(false),_link(link)
{

}

std::shared_ptr<Train> RoutingNode::train() const
{
    return _train;
}

void RoutingNode::setTrain(const std::shared_ptr<Train> &train)
{
    _train = train;
}

const QString &RoutingNode::name() const
{
    return _virtual?_name:_train->trainName().full();
}
