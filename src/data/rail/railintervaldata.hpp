
#pragma once

#include <functional>
#include "railintervalnode.hpp"
#include "railway.h"

class Railway;

/**
 * Ruler等的抽象类
 * 2021年7月4日 将引用改为Reference_wrapper，方便交换操作。
 */
template <typename _Node, typename _Data>
class RailIntervalData{
//    static_assert (std::is_base_of_v<RailIntervalNode<_Node,_Data>,_Node>,
//    "Invalid type argument");
//    static_assert (std::is_base_of_v<RailIntervalData<_Node,_Data>,_Data >,
//    "Invalid Data type");

protected:
    std::reference_wrapper<Railway> _railway;
    bool _different;
    int _index;

    friend class Railway;

    /*
     * 构造函数原则上仅允许Railway的相关函数调用
     */
    RailIntervalData(Railway& railway,bool different,int index):
        _railway(railway),_different(different),_index(index){}

public:
    RailIntervalData(const RailIntervalData&) = delete;
    RailIntervalData(RailIntervalData&&) = delete;
    RailIntervalData& operator=(const RailIntervalData&) = delete;
    RailIntervalData& operator=(RailIntervalData&&) = delete;

    inline int index()const{return _index;}
    inline bool different()const{return _different;}

    inline std::shared_ptr<_Node> firstDownNode(){
        auto t=railway().firstDownInterval();
        if(t){
            return t->template getDataAt<_Node>(_index);
        }else{
            return std::shared_ptr<_Node>();
        }
    }
    inline std::shared_ptr<const _Node> firstDownNode()const{
        auto t= railway().firstDownInterval();
        if(t){
            return t->template getDataAt<_Node>(_index);
        }else{
            return std::shared_ptr<_Node>();
        }
    }
    inline std::shared_ptr<_Node> firstUpNode(){
        auto t= railway().firstUpInterval();
        if(t){
            return t->template getDataAt<_Node>(_index);
        }else{
            return std::shared_ptr<_Node>();
        }
    }
    inline std::shared_ptr<const _Node> firstUpNode()const{
        auto t= railway().firstUpInterval();
        if(t){
            return t->template getDataAt<_Node>(_index);
        }else{
            return std::shared_ptr<_Node>();
        }
    }

    inline std::shared_ptr<_Node> firstDirNode(Direction dir) {
        switch (dir) {
        case Direction::Down:return firstDownNode();
        case Direction::Up:return firstUpNode();
        default:return nullptr;
        }
    }

    inline std::shared_ptr<const _Node> firstDirNode(Direction dir)const {
        return const_cast<RailIntervalData*>(this)->firstDirNode(dir);
    }

    /**
     * 仅考虑近邻区间，获取数据
     */
    inline std::shared_ptr<_Node>
        getNode(const StationName& from,const StationName& to){
        auto t= railway().findInterval(from,to);
        if(!t&&!_different){
            t= railway().findInterval(to,from);
        }
        if(t)
            return t->template getDataAt<_Node>(_index);
        else
            return std::shared_ptr<_Node>();
    }

    inline std::shared_ptr<const _Node>
        getNode(const StationName& from,const StationName& to)const{
        auto t= railway().findInterval(from,to);
        if(!t&&!_different){
            t= railway().findInterval(to,from);
        }
        if(t)
            return t->template getDataAt<_Node>(_index);
        else
            return std::shared_ptr<_Node>();
    }

    inline auto& railway() { return _railway.get(); }
    inline const auto& railway()const { return _railway.get(); }

};
