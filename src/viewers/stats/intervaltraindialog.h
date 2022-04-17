#pragma once

#include <QDialog>
#include <QStandardItemModel>
#include <data/analysis/inttrains/intervalcounter.h>

class QCheckBox;
class QLineEdit;
class IntervalTrainTable;
class Diagram;

class TrainFilter;
class IntervalTrainDialog : public QDialog
{
    Q_OBJECT
    Diagram& diagram;
    TrainFilter* const filter;
    IntervalCounter counter;  // 注意初始化顺序！
    IntervalTrainTable*const table;

    QLineEdit* edFrom,*edTo;
    QCheckBox* ckStop,*ckBusiness;
public:
    IntervalTrainDialog(Diagram& diagram, QWidget* parent=nullptr);
private:
    void initUI();
private slots:
    void updateData();
    void toCsv();
};

