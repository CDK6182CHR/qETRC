#pragma once

#include <QDialog>
#include <QList>
#include <memory>

class RoutingListModel;
class Routing;
class TrainCollection;
class QTableView;
class QTextEdit;
class QCheckBox;
class QLineEdit;
/**
 * @brief The BatchParseRoutingDialog class
 * 批量解析，照着pyETRC写
 */
class BatchParseRoutingDialog : public QDialog
{
    Q_OBJECT;
    TrainCollection& coll;
    RoutingListModel*const model;

    QLineEdit* edSplitter;
    QCheckBox* ckVirtual,*ckFull;
    QTextEdit* edText;
    QTableView* table;
    QString report;
public:
    BatchParseRoutingDialog(TrainCollection& coll_, QWidget* parent=nullptr);
private:
    void initUI();
signals:
    void routingsParsed(const QList<std::shared_ptr<Routing>>& routings);
private slots:
    void actParse();
    void actDetail();
};

