/*
 * 对标尺、天窗等属于区间数据类的封装
 * 封装一个结点
 */
#pragma once
#include <type_traits>
#include <memory>
#include <functional>

#include "railstation.h"

template <typename Node, typename Data>
class RailIntervalData;

template <typename Node, typename Data>
class RailIntervalNode{
//    static_assert (std::is_base_of_v<RailIntervalNode<Node,Data>,Node>,
//    "Invalid type argument");
//    static_assert (std::is_base_of_v<RailIntervalData<Node,Data>,Data >,
//    "Invalid Data type");

protected:
    using NodeType=Node;
    using DataType=Data;

    /**
     * 2022.04.03：Railway::swapBase()交换结点时，需要交换对头节点的引用。
     * 因此改为Ref-wrapper
     */
    std::reference_wrapper<DataType> _data;
    RailInterval& _railint;

    auto& data() { return this->_data.get(); }
    const auto& data()const { return this->_data.get(); }

public:
    RailIntervalNode(DataType& data,RailInterval& railint);

    RailIntervalNode(const RailIntervalNode&)=delete;
    RailIntervalNode(RailIntervalNode&&)=default;

    /**
     * ！！非平凡操作，看好再调用
     * 2022.04.03  设置对头结点的引用，用在swapBase()操作之后。
     * 这个操作应当由Railway调用；_Data类无法正确解决这个问题。
     */
    void setDataNode(std::reference_wrapper<DataType> data) {
        this->_data = data;
    }

    const RailInterval& railInterval()const{return _railint;}
    RailInterval& railInterval(){return _railint;}

    inline std::shared_ptr<Node> nextNode() {
        auto t = _railint.nextInterval();
        if (t) {
            return t->template getDataAt<Node>(data().index());
        }
        else {
            return std::shared_ptr<Node>();
        }
    }
    inline std::shared_ptr<const Node> nextNode()const{
        auto t=_railint.nextInterval();
        if(t){
            return t->template getDataAt<Node>(data().index());
        }else{
            return std::shared_ptr<Node>();
        }
    }

    inline std::shared_ptr<Node> prevNode() {
        auto t = _railint.prevInterval();
        if (t) {
            return t->template getDataAt<Node>(data().index());
        }
        else {
            return std::shared_ptr<Node>();
        }
    }

    inline std::shared_ptr<const Node> prevNode()const {
        auto t = _railint.prevInterval();
        if (t) {
            return t->template getDataAt<Node>(data().index());
        }
        else {
            return std::shared_ptr<Node>();
        }
    }

    inline std::shared_ptr<Node> nextNodeCirc(){
        auto t=nextNode();
        if(!t&&isDownInterval()){
            return data().firstUpNode();
        }else{
            return t;
        }
    }

    inline std::shared_ptr<const Node> nextNodeCirc()const{
        auto t=nextNode();
        if(!t&&isDownInterval()){
            return data().firstUpNode();
        }else{
            return t;
        }
    }

    /**
     * 仅在different时才循环的版本
     */
    inline std::shared_ptr<Node> nextNodeDiffCirc(){
        auto t=nextNode();
        if(!t&&isDownInterval()&& data().different()){
            return data().firstUpNode();
        }else{
            return t;
        }
    }

    inline std::shared_ptr<const Node> nextNodeDiffCirc()const{
        auto t=nextNode();
        if(!t&&isDownInterval()&& data().different()){
            return data().firstUpNode();
        }else{
            return t;
        }
    }

    inline bool isDownInterval()const{
        return _railint.isDown();
    }

    inline const StationName& fromStationName()const{
        return _railint.fromStation()->name;
    }

    inline const StationName& toStationName()const{
        return _railint.toStation()->name;
    }

    auto& dataHead(){return _data.get();}
    const auto& dataHead()const{return _data.get();}

};


template<typename Node, typename Data>
RailIntervalNode<Node, Data>::RailIntervalNode(RailIntervalNode::DataType &data_,
                                                 RailInterval &railint):
    _data(std::ref(data_)),_railint(railint)
{

}


