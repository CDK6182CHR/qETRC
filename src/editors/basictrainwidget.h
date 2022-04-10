#pragma once

#include <QWidget>
#include <memory>

class Train;
class TrainCollection;
class TimetableWidget;
class TimetableStdModel;
class QTableView;

/**
 * @brief The BasicTrainWidget class
 * pyETRC.CurrentWidget
 * 列车基本信息编辑面板，包含车次、时刻表等。
 */
class BasicTrainWidget : public QWidget
{
    Q_OBJECT;
    TrainCollection& coll;
    std::shared_ptr<Train> _train;
    const bool commitInPlace;
    TimetableWidget* ctable;
    QTableView* table;
public:
    explicit BasicTrainWidget(TrainCollection &coll_, bool commitInPlace_,
                              QWidget *parent = nullptr);

    auto train(){return _train;}
    void setTrain(std::shared_ptr<Train> train);
    void refreshData();
    void refreshBasicData();
    TimetableStdModel* timetableModel();
    ~BasicTrainWidget()noexcept = default;

private:
    void initUI();

signals:

private slots:
    void actApply();
    void actCancel();
};

