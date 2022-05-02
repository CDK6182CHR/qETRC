#pragma once
#include <vector>
#include <data/diagram/traingap.h>

namespace gapset{
/**
 * 2022.03.12
 * 一组具有特定意义的列车间隔类型分组。
 * 各子类型由其判定函数来定义。
 */
class GapGroupAbstract:
        public std::vector<TrainGapTypePair>
{
    QString _name;
    int _limit=6*60;
    QString _description;
public:
    GapGroupAbstract(const QString& name, const QString& description=""):
        _name(name),_description(description){}

    const auto& name()const{return _name;}
    void setName(const QString& n){_name=n;}
    int limit()const{return _limit;}
    void setLimit(int i){_limit=i;}
    const auto& description()const{return _description;}
    void setDescription(const QString& d){_description=d;}

    virtual bool matches(const TrainGapTypePair& type)const=0;

    virtual ~GapGroupAbstract()=default;
};

/**
 * @brief The GapGroupBasic class
 * 这是个兜底类，对所有的类型皆返回true
 */
class GapGroupBasic: public GapGroupAbstract{
public:
    using GapGroupAbstract::GapGroupAbstract;
    virtual bool matches(const TrainGapTypePair&)const override{return true;}
};

}


