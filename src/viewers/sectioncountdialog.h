#pragma once

#include <QDialog>
#include <QTableView>
#include <QStandardItemModel>
#include <map>
#include "data/diagram/diagram.h"
#include "data/rail/railway.h"


/**
 * @brief The SectionCountModel class
 * 使用StandardItem暂存数据，防止数据变化导致区间指针失效
 */
class SectionCountModel: public QStandardItemModel
{
    Q_OBJECT
    Diagram& diagram;
    std::shared_ptr<Railway> railway;
    std::map<std::shared_ptr<RailInterval>,int> secs;
public:
    enum {
        ColDir=0,
        ColStart,
        ColEnd,
        ColCount,
        ColMAX
    };
    SectionCountModel(Diagram& diagram_,std::shared_ptr<Railway> railway_,
                      QObject* parent=nullptr);
private:
    void setupModel();
};



/**
 * @brief The SectionCountDialog class
 * (ETRC风格的)断面对数表
 */
class SectionCountDialog : public QDialog
{
    Q_OBJECT;
    QTableView* table;
    Diagram& diagram;
    std::shared_ptr<Railway> railway;
    SectionCountModel* model;
public:
    SectionCountDialog(Diagram& diagram_,std::shared_ptr<Railway> railway_,
                       QWidget* parent=nullptr);

private:
    void initUI();
};

