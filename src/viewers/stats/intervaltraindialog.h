#pragma once

#include <QDialog>
#include <QStandardItemModel>
#include <data/analysis/inttrains/intervalcounter.h>

class QCheckBox;
class QLineEdit;
class IntervalTrainTable;
class Diagram;

class TrainFilterSelector;
class IntervalTrainDialog : public QDialog
{
    Q_OBJECT
    Diagram& diagram;
    TrainFilterSelector* const filter;
    IntervalCounter counter;  // 注意初始化顺序！
    IntervalTrainTable*const table;

    QLineEdit* edFrom,*edTo;
    QCheckBox* ckStop,*ckBusiness;
    QCheckBox* ckMultiStart, * ckMultiEnd;
    QCheckBox* ckRegexStart, * ckRegexEnd;

    bool informMultiCheck = true, informRegexCheck = true;
public:
    IntervalTrainDialog(Diagram& diagram, QWidget* parent=nullptr);
private:
    void initUI();
private slots:
    void updateData();
    void toCsv();
    void onMultiChecked(bool on);
    void onRegexChecked(bool on);
};

