#pragma once

#ifdef QETRC_MOBILE

#include <QWidget>
#include <memory>

class ATrainOptions;
class Diagram;
class TrainListModel;
class TrainInfoWidget;
class TimetableQuickWidget;
class Train;
class TrainCollection;
class QListView;
class QTabWidget;
/**
 * @brief The ATrainPage class
 * 列车信息类。
 */
class ATrainPage : public QWidget
{
    Q_OBJECT
    Diagram& diagram;
    TrainCollection& coll;
    std::shared_ptr<Train> train{};
    QListView* lsTrains;
    QTabWidget* tab;
    TimetableQuickWidget* timetable;
    TrainInfoWidget* info;
    TrainListModel* model;
    ATrainOptions* opt;
public:
    explicit ATrainPage(Diagram& diagram, QWidget *parent = nullptr);
private:
    void initUI();

signals:

public slots:
    void setTrain(std::shared_ptr<Train> train);
    void refreshData();
private slots:
    void onCurrentRowChanged(const QModelIndex& idx);

    void actTrainEvent();
    void actTrainLines();
    void actRulerRef();
    void actDiagnosis();

};

#endif // ATRAINPAGE_H
