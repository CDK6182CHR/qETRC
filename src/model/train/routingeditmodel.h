#pragma once

#include "model/general/qemoveablemodel.h"

class Routing;
class Train;

/**
 * @brief The RoutingEditModel class
 * 交路编辑页面，单个交路的model
 */
class RoutingEditModel : public QEMoveableModel
{
    Q_OBJECT;
    std::shared_ptr<Routing> routing;
public:
    enum {
        ColTrainName=0,
        ColVirtual,
        ColStarting,
        ColTerminal,
        ColLink,
        ColMAX
    };
    explicit RoutingEditModel(std::shared_ptr<Routing> routing_, QWidget *parent = nullptr);
    void refreshData();
    void setRouting(std::shared_ptr<Routing> routing);

    /**
     * 将表中的交路序列信息写入res中。
     * res保证初始为空。返回是否成功
     * 如果不成功，这里直接报错。
     */
    bool getAppliedOrder(std::shared_ptr<Routing> res);
    
private:
    void setupModel();
signals:
    void routingInserted(int row);   //主要是通告table改变当前行
public slots:
    void setRealRow(int row,std::shared_ptr<Train> train, bool link);
    void setVirtualRow(int row, const QString& name, bool link);

    /**
     * 由插入的Dialog调用。在指定位置插入行。
     */
    void insertRealRow(int row, std::shared_ptr<Train> train,bool link);
    void insertVirtualRow(int row, const QString& name, bool link);
};

