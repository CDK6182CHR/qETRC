/*
 * 对标尺、天窗等属于区间数据类的封装
 * 封装一个结点
 */
#pragma once
#include <type_traits>
#include <memory>
#include <functional>

#include "railstation.h"

template <typename _Node, typename _Data>
class RailIntervalData;

template <typename _Node, typename _Data>
class RailIntervalNode{
//    static_assert (std::is_base_of_v<RailIntervalNode<_Node,_Data>,_Node>,
//    "Invalid type argument");
//    static_assert (std::is_base_of_v<RailIntervalData<_Node,_Data>,_Data >,
//    "Invalid Data type");

protected:
    using NodeType=_Node;
    using DataType=_Data;

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

    inline std::shared_ptr<_Node> nextNode() {
        auto t = _railint.nextInterval();
        if (t) {
            return t->template getDataAt<_Node>(data().index());
        }
        else {
            return std::shared_ptr<_Node>();
        }
    }
    inline std::shared_ptr<const _Node> nextNode()const{
        auto t=_railint.nextInterval();
        if(t){
            return t->template getDataAt<_Node>(data().index());
        }else{
            return std::shared_ptr<_Node>();
        }
    }

    inline std::shared_ptr<_Node> prevNode() {
        auto t = _railint.prevInterval();
        if (t) {
            return t->template getDataAt<_Node>(data().index());
        }
        else {
            return std::shared_ptr<_Node>();
        }
    }

    inline std::shared_ptr<const _Node> prevNode()const {
        auto t = _railint.prevInterval();
        if (t) {
            return t->template getDataAt<_Node>(data().index());
        }
        else {
            return std::shared_ptr<_Node>();
        }
    }

    inline std::shared_ptr<_Node> nextNodeCirc(){
        auto t=nextNode();
        if(!t&&isDownInterval()){
            return data().firstUpNode();
        }else{
            return t;
        }
    }

    inline std::shared_ptr<const _Node> nextNodeCirc()const{
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
    inline std::shared_ptr<_Node> nextNodeDiffCirc(){
        auto t=nextNode();
        if(!t&&isDownInterval()&& data().different()){
            return data().firstUpNode();
        }else{
            return t;
        }
    }

    inline std::shared_ptr<const _Node> nextNodeDiffCirc()const{
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


template<typename _Node, typename _Data>
RailIntervalNode<_Node, _Data>::RailIntervalNode(RailIntervalNode::DataType &data_,
                                                 RailInterval &railint):
    _data(std::ref(data_)),_railint(railint)
{

}


