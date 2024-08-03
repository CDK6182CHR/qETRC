#pragma once
#include <QDialog>

#include <memory>

class TrainCollection;
class QTableView;
class RoutingEditModel;
class QCheckBox;
class Routing;
class QLineEdit;

class MergeRoutingDialog : public QDialog
{
    Q_OBJECT
    TrainCollection& m_coll;
    std::shared_ptr<Routing> m_routing;
    std::shared_ptr<Routing> m_other;
    QLineEdit* m_edName, *m_edOther;
    QCheckBox* m_ckSelPos;
    RoutingEditModel* m_model;
    QTableView* m_table;
public:
    MergeRoutingDialog(TrainCollection& coll, std::shared_ptr<Routing> routing, QWidget* parent=nullptr);
    void refreshData();

protected:
    void accept()override;

private:
    void initUI();

signals:
    void mergeApplied(std::shared_ptr<Routing> routing, std::shared_ptr<Routing> other,
        bool select_row, int pos);
};

