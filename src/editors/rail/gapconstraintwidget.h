#pragma once
#include <QWidget>

class GapConstraintModel;
class GapConstraints;


class QTableView;
namespace gapset {
class GapSetAbstract;
}

class GapConstraintWidget : public QWidget
{
    Q_OBJECT
    GapConstraints& _constraints;
    GapConstraintModel* const _model;
    QTableView* table;
public:
    explicit GapConstraintWidget(GapConstraints& constraints,
                                 QWidget *parent = nullptr);
    void refreshData();
private:
    void initUI();

signals:

public slots:
    void onApply();

};

