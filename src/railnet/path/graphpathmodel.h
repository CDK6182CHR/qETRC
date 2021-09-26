#pragma once

#include <QStandardItemModel>
#include <railnet/graph/railnet.h>
/**
 * @brief The GraphPathModel class
 * 图上的一条径路的模型。理论上可以针对任意的path_t。
 * 只读。
 */
class GraphPathModel : public QStandardItemModel
{
    Q_OBJECT
    RailNet::path_t path;
public:
    enum {
        ColName=0,
        ColMile,
        ColOutDeg,
        ColLevel,
        ColMAX
    };
    explicit GraphPathModel(QObject *parent = nullptr);
    const auto& getPath(){return path;}
    void setPath(const RailNet::path_t& path);
    void setPath(RailNet::path_t&& path);

    /**
     * 到指定行的子路径。从数据path直接提取。
     * 如果输入非法或为第0行，返回空。
     */
    RailNet::path_t subPathTo(int row);
private:
    void setupModel();
    void setupRow(int row, double mile, const RailNet::vertex* v);
};

