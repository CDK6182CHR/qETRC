#pragma once
#include <QDialog>
#include <QTime>
#include <QStandardItemModel>
#include <memory>

class RailStation;
class Diagram;
class Railway;

class ConflictModel: public QStandardItemModel
{
    Q_OBJECT
    Diagram& diagram;
    std::shared_ptr<Railway> railway{};
    std::shared_ptr<const RailStation> station{};
    QTime arrive{},depart{};
public:

    enum {
        ColDir = 0,
        ColTrainName,
        ColTime,
        ColEvent,
        ColPos,
        ColType,
        ColMAX
    };

    ConflictModel(Diagram& diagram_, QObject* parent=nullptr);

    /**
     * @brief setupModel
     * 设置先找出车站事件表，再线性遍历找符合条件的。从pyETRC的实践来看，效率不成问题
     */
    void setupModel(std::shared_ptr<Railway> railway_, 
        std::shared_ptr<const RailStation> station_,
                    const QTime& arrive_,const QTime& depart_);
};

class QLineEdit;
class QTableView;

/**
 * @brief The ConflictDialog class
 * 检查冲突的对话框  与pyETRC不同：采用表格展示；
 * 且全程仅保留一个实例，查看新站时，更改数据
 */
class ConflictDialog : public QDialog
{
    Q_OBJECT;
    Diagram& diagram;
    std::shared_ptr<Railway> railway{};
    std::shared_ptr<const RailStation> station{};
    QTime arrive{},depart{};
    ConflictModel*const model;

    QLineEdit* edName, * edArrive, * edDepart;
    QTableView* table;
public:
    ConflictDialog(Diagram& diagram_, QWidget* parent=nullptr);
    void setData(std::shared_ptr<Railway> railway_,
        std::shared_ptr<const RailStation> station_,
                 const QTime& arrive_,const QTime& depart_);
    auto* getModel(){return model;}

private:
    void initUI();
};

