#pragma once
#include <QWizardPage>

class Train;
class TrainListReadModel;

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
    const QList<std::shared_ptr<Train>>& trains()const;
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

