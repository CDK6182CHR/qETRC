#pragma once
#include <QWizardPage>
#include <memory>

class Train;
class TrainListReadModel;

class TrainCollection;
class QTableView;
class TrainFilter;
class Diagram;
class Railway;

class ReadRulerPageTrain : public QWizardPage
{
    Q_OBJECT;
    Diagram& diagram;
    TrainCollection& coll;
    TrainListReadModel *mdUnsel, *mdSel;
    QTableView *tbUnsel,*tbSel;
    TrainFilter* filter;

    /**
     * 2022.05.06增加
     * 仅用来记录当前设置的线路；如果改变，要重新初始化表格
     */
    std::shared_ptr<Railway> _rail{};

public:
    ReadRulerPageTrain(Diagram& diagram_, QWidget* parent=nullptr);
    const QList<std::shared_ptr<Train>>& trains()const;
    virtual bool validatePage()override;
    void refreshForRail(std::shared_ptr<Railway> railway);
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

