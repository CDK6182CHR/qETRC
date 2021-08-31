#pragma once

#include <memory>
#include <model/general/qemoveablemodel.h>

class TrainType;
class TrainCollection;
class TypeManager;

/**
 * @brief The TypeListModel class
 * 类型管理的model，即TypeManager._types的Model
 * 展示是比较平凡的；主要是提交过程需要考虑清楚。
 * 从这里删除并不一定有效，因为可能会涉及到列车指向的对象。
 * 注意本类同时处理TrainCollection的和default的；区别仅仅在应用后操作中体现。
 */
class TypeConfigModel : public QEMoveableModel
{
    Q_OBJECT;
    TypeManager& manager;
public:

    enum{
        ColName=0,
        ColPassenger,
        ColColor,
        ColWidth,
        ColLineStyle,
        ColMAX
    };
    explicit TypeConfigModel(TypeManager& manager_, QObject *parent = nullptr);
    void refreshData();

    std::pair< QMap<QString,std::shared_ptr<TrainType>>,
        QVector<QPair<std::shared_ptr<TrainType>,std::shared_ptr<TrainType>>>>
        appliedData();


private:
    void setupModel();
    void setColorItem(int row, const QColor& color);
    void setupType(int row, std::shared_ptr<TrainType> type);

protected:
    virtual void setupNewRow(int row) override;

};

