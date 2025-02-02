#pragma once

#include <QDialog>
#include <QStandardItemModel>
#include <vector>
#include <utility>

#include "data/diagram/diagram.h"   // 使用了定义的Alias



/**
 * @brief The RailSectionEventsModel class
 * 指定y坐标值处的切片事件。只考虑y坐标值作为输入。
 * 可以更新数据。初始化时无数据；后面设置数据时才计算和更新数据。
 */
class RailSectionEventsModel: public QStandardItemModel
{
    Q_OBJECT
    Diagram& diagram;
    std::shared_ptr<Railway> railway;
    double y=0;
    Diagram::SectionEventList lst;
public:
    enum{
        ColTrainName=0,
        ColTime,
        ColType,
        ColDir,
        ColStarting,
        ColTerminal,
        ColModel,
        ColOwner,
        ColMAX
    };
    RailSectionEventsModel(Diagram& diagram_, std::shared_ptr<Railway> rail,
                           QObject* parent=nullptr);
    void setY(double y);
    QTime timeForRow(int row)const;

private:
    void setupModel();
};


class QDoubleSpinBox;
class QLineEdit;
class QTableView;

class RailSectionEventsDialog : public QDialog
{
    Q_OBJECT;
    Diagram& diagram;
    std::shared_ptr<Railway> railway;

    RailSectionEventsModel* model;
    QDoubleSpinBox* spMile;
    QLineEdit* edDownIt,*edUpIt;
    QTableView* table;
public:
    RailSectionEventsDialog(Diagram& diagram_, std::shared_ptr<Railway> railway_,
                            QWidget* parent=nullptr);

private:
    void initUI();
signals:
    void locateToEvent(int pageIndex, std::shared_ptr<const Railway>, double mile,
        const QTime&);
private slots:

    /**
     * @brief updateData
     * 改变里程后更新数据。
     */
    void updateData();

    void toCsv();

    void actLocate();
};

