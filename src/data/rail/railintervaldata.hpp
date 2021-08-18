
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

    /**
     * 如果设置为false，将下行数据改换为相应反向区间的数据。
     * precondition: 如果设置为false，则不存在上下行分设。
     */
    inline void setDifferent(bool d) {
        if (_different && !d) {
            copyDownToUp();
        }
        _different = d; 
    }

    /**
     * 将下行数据复制给上行，用于设置different=false时
     * precondition: 不存在上下行分设站
     */
    inline void copyDownToUp() {
        for (auto p = _railway.get().firstUpInterval(); p; p = p->nextInterval()) {
            auto pinv = p->inverseInterval();
            if (pinv) {
                auto d = p->template getDataAt<_Node>(_index);
                auto dinv = pinv->template getDataAt<_Node>(_index);
                d->operator=(*dinv);
            }
            else {
                qDebug() << "RailIntervalData::setDifferent: WARNING: "
                    << "Unexpected null inverse interval: " << *p << Qt::endl;
            }
        }
    }

    inline void copyUpToDown() {
        for (auto p = _railway.get().firstDownInterval(); p; p = p->nextInterval()) {
            auto pinv = p->inverseInterval();
            if (pinv) {
                auto d = p->template getDataAt<_Node>(_index);
                auto dinv = pinv->template getDataAt<_Node>(_index);
                d->operator=(*dinv);
            }
            else {
                qDebug() << "RailIntervalData::setDifferent: WARNING: "
                    << "Unexpected null inverse interval: " << *p << Qt::endl;
            }
        }
    }

    inline std::shared_ptr<_Node> firstDownNode() {
        auto t = railway().firstDownInterval();
        if (t) {
            return t->template getDataAt<_Node>(_index);
        }
        else {
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

    /**
     * 2021.08.09  返回指定车站沿指定方向上一近邻区间的数据
     */
    inline std::shared_ptr<_Node>
        dirPrevNode(std::shared_ptr<RailStation> st, Direction dir)
    {
        auto t = st->dirPrevInterval(dir);
        if (t) {
            return t->template getDataAt<_Node>(_index);
        }
        else
            return {};
    }

    inline std::shared_ptr<_Node>
        dirNextNode(std::shared_ptr<RailStation> st, Direction dir)
    {
        auto t = st->dirNextInterval(dir);
        if (t) {
            return t->template getDataAt<_Node>(_index);
        }
        else
            return {};
    }

    inline std::shared_ptr<const _Node>
        dirPrevNode(std::shared_ptr<const RailStation> st, Direction dir)const
    {
        auto t = st->dirPrevInterval(dir);
        if (t) {
            return t->template getDataAt<_Node>(_index);
        }
        else
            return {};
    }

    inline std::shared_ptr<const _Node>
        dirNextNode(std::shared_ptr<const RailStation> st, Direction dir)const
    {
        auto t = st->dirNextInterval(dir);
        if (t) {
            return t->template getDataAt<_Node>(_index);
        }
        else
            return {};
    }

    inline auto& railway() { return _railway.get(); }
    inline const auto& railway()const { return _railway.get(); }

};
