#pragma once

#include <QRegExp>
#include "util/qecontrolledtable.h"

class QEMoveableModel;
class TrainCollection;
class TrainNameRegexTable : public QEControlledTable
{
    Q_OBJECT
    TrainCollection& coll;
    QEMoveableModel* model;
    QVector<QRegExp> _names;
public:
    TrainNameRegexTable(TrainCollection& coll_, QWidget* parent=nullptr);
    QVector<QRegExp> names()const;
private:
    void initUI();
private slots:
//    void actApply();
    void refreshData(const QVector<QRegExp>& _names);
public slots:
    void clearNames();
};

