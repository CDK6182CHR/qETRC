#pragma once
#include <QWizardPage>

#include "model/train/trainlistreadmodel.h"

class TrainCollection;
class QTableView;

class ReadRulerPageTrain : public QWizardPage
{
    Q_OBJECT;
    TrainCollection& coll;
    TrainListReadModel *mdUnsel, *mdSel;
    QTableView *tbUnsel,*tbSel;
public:
    ReadRulerPageTrain(TrainCollection& coll_, QWidget* parent=nullptr);
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
    void selectAll();
    void deselectAll();
    void select();
    void deselect();
};

