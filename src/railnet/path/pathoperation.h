#pragma once

#include <railnet/graph/railnet.h>
#include <QVector>

/**
 * @brief The PathOperation class
 * 经由选择的一步操作 （不包含起始）
 * 最短路，邻站或者邻线三种方式。
 * 按照struct组织。
 * 暂定：不保存开始结点 （对应于表格中的一行）
 * 构造前必须保证有效性 （径路非空）
 * 暂定不额外保存邻线、邻站的线名、方向，从path中直接读取。
 */
class PathOperation
{
public:
    enum Method{
        ShortestPath,
        AdjStation,
        AdjRailway
    };
    std::shared_ptr<const RailNet::vertex> target;
    Method method;
    RailNet::path_t path;
    double mileStone;   // 延长公里  仅做参考
    PathOperation(std::shared_ptr<const RailNet::vertex> target,
                  Method method, const RailNet::path_t& path, double mileStone):
        target(target), method(method), path(path), mileStone(mileStone){}
    PathOperation(std::shared_ptr<const RailNet::vertex> target,
                  Method method, RailNet::path_t&& path, double mileStone):
        target(target), method(method), path(std::forward<RailNet::path_t>(path)),
        mileStone(mileStone){}

    /**
     * 由Path的首站返回区间线名。对于最短路模式，数据无意义。
     * path应当保证非空。如果为空，应警告。
     */
    QString railName()const;
    Direction dir()const;

    static QString methodString(Method m);
};

/**
 * @brief The PathOperationSeq class
 * 一个操作的序列，可以从此中数据直接生成Railway。
 */
class PathOperationSeq{
    std::shared_ptr<const RailNet::vertex> _start;
    QVector<PathOperation> _operations;
public:
    PathOperationSeq()=default;
    auto start()const{return _start;}
    void setStart(std::shared_ptr<const RailNet::vertex> start){this->_start=start;}
    auto& operations(){return _operations;}
    const auto& operations()const{return _operations;}

    void push(PathOperation&& op);
    void pop(){_operations.pop_back();}
    const PathOperation& back()const{return _operations.back();}

    std::shared_ptr<const RailNet::vertex> lastVertex()const;

    /**
     * @brief isValid
     * 返回能否生成Railway。具有起始站和至少一个非空Operation。
     */
    bool isValid()const;

    void clear();

    /**
     * 判空：start和operations皆为空。
     * 注意：valid并不等于! empty()。
     */
    bool empty()const;

    double currentMile()const;

    /**
     * 获取完整的径路表，用于转换生成Railway对象。
     */
    RailNet::path_t fullPath()const;

};

