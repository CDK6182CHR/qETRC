/*
 * Ruler等的抽象类
 */
#pragma once

#include "railintervalnode.hpp"
#include "railway.h"

class Railway;

template <typename _Node, typename _Data>
class RailIntervalData{
//    static_assert (std::is_base_of_v<RailIntervalNode<_Node,_Data>,_Node>,
//    "Invalid type argument");
//    static_assert (std::is_base_of_v<RailIntervalData<_Node,_Data>,_Data >,
//    "Invalid Data type");

protected:
    Railway& _railway;
    bool _different;
    int _index;

    friend class Railway;

    /*
     * 构造函数原则上仅允许Railway的相关函数调用
     */
    RailIntervalData(Railway& railway,bool different,int index):
        _railway(railway),_different(different),_index(index){}

public:
    inline int index()const{return _index;}
    inline bool different()const{return _different;}

    inline std::shared_ptr<_Node> firstDownNode(){
        auto t=_railway.firstDownInterval();
        if(t){
            return t->getDataAt<_Node>(_index);
        }else{
            return std::shared_ptr<_Node>();
        }
    }
    inline std::shared_ptr<const _Node> firstDownNode()const{
        auto t=_railway.firstDownInterval();
        if(t){
            return t->getDataAt<_Node>(_index);
        }else{
            return std::shared_ptr<_Node>();
        }
    }
    inline std::shared_ptr<_Node> firstUpNode(){
        auto t=_railway.firstUpInterval();
        if(t){
            return t->getDataAt<_Node>(_index);
        }else{
            return std::shared_ptr<_Node>();
        }
    }
    inline std::shared_ptr<const _Node> firstUpNode()const{
        auto t=_railway.firstUpInterval();
        if(t){
            return t->getDataAt<_Node>(_index);
        }else{
            return std::shared_ptr<_Node>();
        }
    }

    /*
     * 仅考虑近邻区间，获取数据
     */
    inline std::shared_ptr<_Node>
        getNode(const StationName& from,const StationName& to){
        auto t=_railway.findInterval(from,to);
        if(!t&&!_different){
            t=_railway.findInterval(to,from);
        }
        if(t)
            return t->getDataAt<_Node>(_index);
        else
            return std::shared_ptr<_Node>();
    }

    inline std::shared_ptr<const _Node>
        getNode(const StationName& from,const StationName& to)const{
        auto t=_railway.findInterval(from,to);
        if(!t&&!_different){
            t=_railway.findInterval(to,from);
        }
        if(t)
            return t->getDataAt<_Node>(_index);
        else
            return std::shared_ptr<_Node>();
    }

};
