#pragma once

#include <QString>
#include <QJsonObject>
#include <QList>

//#include "rulernode.h"

#include "railintervaldata.hpp"

class Railway;
class TrainAdapter;

/**
 * 与标尺有关的数据结构，以及标尺封装（代理）类
 * 与pyETRC不同，Ruler类并不包含真正的数据，更像一个View
 * 尽量去实现旧有的API
 * 新增index，要求必须等于在QList中的坐标，采用这个去定位
 */
class Ruler:
        public RailIntervalData<RulerNode, Ruler>
{
    QString _name;

    friend class Railway;

    /**
     * 构造函数原则上仅允许Railway::addEmptyRuler调用
     */
    Ruler(Railway& railway, const QString& name, bool different, int index);
public:

    inline const QString& name()const { return _name; }
    inline void setName(const QString& name) { _name = name; }
    QString& nameRef() { return _name; }

    QJsonObject toJson()const;

    void show()const;

    /**
     * @brief totalInterval  区间总通通时分
     * @param from  起始站
     * @param to  终止站
     * @param dir from->to的行别。必须保证正确，否则返回值无意义。
     * @return 区间总通通时分；如果数据有问题，返回-1
     */
    int totalInterval(std::shared_ptr<RailStation> from,
                      std::shared_ptr<RailStation> to,
                      Direction _dir)const;

    bool isOrdinateRuler()const;

    /**
     * 注意签名。
     * 返回一个新建的Railway对象，只包含基线数据和当前标尺数据。
     */
    std::shared_ptr<Railway> clone()const;

    /**
     * 交换数据（但不交换对Railway的引用以及index）
     */
    void swap(Ruler& other);

    /**
     * pyETRC传统功能，从单一车次读取标尺。注意输入TrainAdapter
     * 将指定车次的区间运行数据直接读入本标尺，直接覆盖
     * 返回成功读取的区间数量
     * ！！注意：写入的区间实际上与this没啥关系，
     * 是从TrainAdapter里面直接读的区间
     */
    int fromSingleTrain(std::shared_ptr<const TrainAdapter> adp,
        int start, int stop);

    /**
     * 2021.09.25  非空节点数
     */
    int validNodeCount()const;


};

Q_DECLARE_METATYPE(std::shared_ptr<Ruler>);

