#pragma once

#include <QWidget>
#include <memory>

class QTableView;
class QCheckBox;
class QLineEdit;
class TimetableQuickModel;
class Train;
/**
 * @brief The TimetableQuickWidget class
 * 快速查看的只读列车时刻表  pyETRC的Ctrl+Y面板
 */
class TimetableQuickWidget : public QWidget
{
    Q_OBJECT
    std::shared_ptr<Train> train;
    TimetableQuickModel* const model;

    QLineEdit* edName;
    QCheckBox* ckStopOnly;
    QTableView* table;
public:
    explicit TimetableQuickWidget(QWidget *parent = nullptr);
    auto getTrain(){return train;}
private:
    void initUI();
signals:
private slots:
    void onStopOnlyChanged(bool on);
public slots:
    void setTrain(std::shared_ptr<Train> train);
    void refreshData();

};

