#pragma once
#include <QWidget>
#include <memory>

class Routing;
class QPushButton;
class QDoubleSpinBox;
class PenStyleCombo;
class QCheckBox;
class QComboBox;
class QLineEdit;
class QTableView;
class QEControlledTable;
class TimetableStdModel;
class Train;
class TrainCollection;
class TimetableWidget;

/**
 * @brief The EditTrainWidget class
 * pyETRC.CurrentWidget
 * 列车全部数据编辑
 */

class EditTrainWidget : public QWidget
{
    Q_OBJECT
    TrainCollection& coll;
    std::shared_ptr<Train> _train;

    QLineEdit* edTrainName,*edDownName,*edUpName;
    QLineEdit* edStaring,*edTerminal;
    QComboBox* cbType;
    QCheckBox* ckPassenger;

    QCheckBox* ckAutoPen;
    QDoubleSpinBox* spWidth;
    PenStyleCombo* cbLs;
    QPushButton* btnColor;
    QLineEdit* edRouting;

    QPushButton* btnToRouting;

    TimetableWidget* ctable;
    QTableView* table;

    QColor tmpColor;
public:
    explicit EditTrainWidget(TrainCollection& coll, std::shared_ptr<Train> train,
                             QWidget *parent = nullptr);
    auto train(){return _train;}
    void setTrain(const std::shared_ptr<Train>& train);
    void refreshData();
    void refreshBasicData();
    TimetableStdModel* getModel();

private:
    void initUI();

signals:
    void trainInfoChanged(std::shared_ptr<Train> train,
                          std::shared_ptr<Train> info);
    void switchToRouting(std::shared_ptr<Routing> routing);
    void removeTrain(std::shared_ptr<Train> train);
private slots:
    void actApply();
    void actCancel();
    void actRemove();

    void onAutoPenToggled(bool on);
    void setLineColor();
    void actSwitchToRouting();

    void setupTypeCombo();
};

