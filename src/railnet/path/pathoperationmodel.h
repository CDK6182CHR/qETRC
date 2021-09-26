#pragma once

#include <QAbstractTableModel>

#include "pathoperation.h"

/**
 * @brief The PathOperationModel class
 * 结点径路选择的Model。原则上，在程序运行过程中为单例，管理的操作Seq也是单例。
 * 采用AbstractModel，不允许直接编辑，只能在末尾增删结点。
 * 增删操作，在本类完成。
 */
class PathOperationModel : public QAbstractTableModel
{
    Q_OBJECT
    const RailNet& net;
    PathOperationSeq seq;
public:
    enum {
        ColName=0,
        ColMile,
        ColMethod,
        ColPath,
        ColMAX
    };
    explicit PathOperationModel(const RailNet& net, QObject *parent = nullptr);
    const auto& sequence()const{return seq;}

    virtual int rowCount(const QModelIndex &parent) const override;
    virtual int columnCount(const QModelIndex &parent) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    /**
     * 按最短路算法添加，返回是否成功。这里进行计算工作。
     * 如果不成功，report中报告错误原因。
     * 首站的添加也通过这个。
     */
    bool addByShortestPath(const QString& target, QString* report);

    /**
     * 此重载用于添加反向径路。vertex对象可以直接获得，不必绕一圈站名。
     */
    bool addByShortestPath(const std::shared_ptr<const RailNet::vertex>& target,
                           QString* report);

    /**
     * 按邻站添加。
     * 可能报错的原因是，空表，或者连不上。保证入参非空。
     */
    bool addByAdjStation(std::shared_ptr<RailNet::edge> ed, QString* report);

    /**
     * 按邻线添加。
     * 可能报错原因是，空表，或连不上。保证入参非空。
     */
    bool addByAdjPath(RailNet::path_t&& path, QString* report);


    /**
     * @brief popSelect  删除最后一个选择的站 直接执行
     */
    void popSelect();

    void clearSelect();

    /**
     * 从反向的Sequence读取和添加。返回是否成功。
     * 对于最短路，也采用最短路算法添加；对于邻站邻线，都用邻线算法添加，
     * 如果没有径路，则报告失败，并保留当前的样子。
     * 调用之前，清空数据。
     */
    bool fromInverseSeq(const PathOperationSeq& other, QString* report);

private:

    /**
     * 添加首站；保证添加前，seq为空，不做前置检查。
     */
    void addStartStation(std::shared_ptr<const RailNet::vertex> station);

    /**
     * 添加中间结点；保证添加前，seq非空，并不做检查。
     */
    void appendOperation(PathOperation&& op);
signals:
    void lastVertexChanged(std::shared_ptr<const RailNet::vertex> ve);
};

