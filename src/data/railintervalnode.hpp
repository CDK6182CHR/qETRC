/*
 * 对标尺、天窗等属于区间数据类的封装
 * 封装一个结点
 */
#pragma once
#include <type_traits>
#include <memory>

#include "railinterval.h"
#include "stationname.h"
#include "railstation.h"

template <typename _Node, typename _Data>
class RailIntervalData;

template <typename _Node, typename _Data>
class RailIntervalNode{
    static_assert (std::is_base_of_v<RailIntervalNode<_Node,_Data>,_Node>,
    "Invalid type argument");
    static_assert (std::is_base_of_v<RailIntervalData<_Node,_Data>,_Data >,
    "Invalid Data type");

    using NodeType=_Node;
    using DataType=_Data;

    DataType& _data;
    RailInterval& _railint;

public:
    RailIntervalNode(DataType& data,RailInterval& railint);

    RailIntervalNode(const RailIntervalNode&)=delete;
    RailIntervalNode(RailIntervalNode&&)=default;

    const RailInterval& railInterval()const{return _railint;}
    RailInterval& railInterval(){return _railint;}

    inline std::shared_ptr<_Node> nextNode(){
        auto t=_railint.nextInterval();
        if(t){
            return t->getDataAt<_Node>(_data.index());
        }else{
            return std::shared_ptr<_Node>();
        }
    }
    inline std::shared_ptr<const _Node> nextNode()const{
        auto t=_railint.nextInterval();
        if(t){
            return t->getDataAt<_Node>(_data.index());
        }else{
            return std::shared_ptr<_Node>();
        }
    }

    inline std::shared_ptr<_Node> nextNodeCirc(){
        auto t=nextNode();
        if(!t&&_data.different()&&isDownInterval()){
            return _data.firstUpNode();
        }else{
            return t;
        }
    }

    inline std::shared_ptr<const _Node> nextNodeCirc()const{
        auto t=nextNode();
        if(!t&&_data.different()&&isDownInterval()){
            return _data.firstUpNode();
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

};


template<typename _Node, typename _Data>
RailIntervalNode<_Node, _Data>::RailIntervalNode(RailIntervalNode::DataType &data,
                                                 RailInterval &railint):
    _data(data),_railint(railint)
{

}


