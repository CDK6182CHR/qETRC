#pragma once
#include <QWizardPage>

#include "model/train/trainlistreadmodel.h"

class TrainCollection;
class QTableView;
class TrainFilter;
class Diagram;

class ReadRulerPageTrain : public QWizardPage
{
    Q_OBJECT;
    Diagram& diagram;
    TrainCollection& coll;
    TrainListReadModel *mdUnsel, *mdSel;
    QTableView *tbUnsel,*tbSel;
    TrainFilter* filter;

public:
    ReadRulerPageTrain(Diagram& diagram_, QWidget* parent=nullptr);
    const auto& trains()const{return mdSel->trains();}
    virtual bool validatePage()override;
private:
    void initUI();
    void resizeTables();
    /**
     * 返回：反向排序的选择的行号表
     */
    QVector<int> inversedSelectedRows(QTableView* table);
private slots:
    void filtTrain();
    void filtTrainApplied();
    void selectAll();
    void deselectAll();
    void select();
    void deselect();
};

