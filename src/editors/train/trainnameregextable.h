#pragma once

#include <QRegularExpression>
#include "util/qecontrolledtable.h"

class QEMoveableModel;
class TrainCollection;
class TrainNameRegexTable : public QEControlledTable
{
    Q_OBJECT
    TrainCollection& coll;
    QEMoveableModel* model;
    QVector<QRegularExpression> _names;
public:
    TrainNameRegexTable(TrainCollection& coll_, QWidget* parent=nullptr);
    QVector<QRegularExpression> names()const;
private:
    void initUI();
private slots:

public slots:
    void refreshData(const QVector<QRegularExpression>& _names);
    void clearNames();
};

