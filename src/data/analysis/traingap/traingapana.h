#pragma once

#include <data/diagram/traingap.h>

class Railway;


class TrainFilterCore;
class Diagram;

/**
 * @brief The TrainGapAna class
 * 2022.04.30 新增
 * 将TrainGap使用类似IntTrains的模式实现分析，
 * 相关算法主要封装到这个类里面。
 * 原则上所有算法都应该从Diagram下面弄出来，但现在暂时不做这一步重构
 * 主要目的是封装全局最小间隔计算
 * @see traingapstatdialog.cpp
 */
class TrainGapAna
{
    Diagram& diagram;
    const TrainFilterCore& filter;
    bool _singleLine=false;
    int _cutSecs;
public:
    TrainGapAna(Diagram& diagram, const TrainFilterCore& filter);

    void setSingleLine(bool on){_singleLine=on;}
    void setCutSecs(int secs){_cutSecs=secs;}

    std::map<TrainGapTypePair,int>
        globalMinimal(std::shared_ptr<Railway> rail)const;
};

