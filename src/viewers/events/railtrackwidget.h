#pragma once
#include <QDialog>
#include <QSplitter>
#include <vector>
#include "data/rail/trackdiagramdata.h"
#include "util/buttongroup.hpp"

class RailTrackAdjustModel;
class QEMoveableModel;
class QTableView;
class QEControlledTable;
class QSpinBox;
class QSlider;
class RailStation;
class Railway;
class Diagram;
struct AdapterStation;
class TrainLine;
class TrackDiagram;
class TrackDiagramData;

/**
 * @brief The RailTrackSetupWidget class
 * 左侧那个设置的面板。形式上，对于RailTrackWidget来说，是单例
 */
class RailTrackSetupWidget: public QWidget
{
    Q_OBJECT
    TrackDiagramData& _data;
    RadioButtonGroup<3,QVBoxLayout>* gpMode;
    RadioButtonGroup<2>* gpMainStay;
    QSpinBox* spSame,*spOpps;
    QEControlledTable* ctable;
    QTableView* table;
    QEMoveableModel* model;

    bool informOnAdjust=true;
public:
    RailTrackSetupWidget(TrackDiagramData& data, QWidget* parent=nullptr);

private:
    void initUI();
signals:
    void applied();
    void actSaveOrder(const QList<QString>& order);
    void repaintDiagram();  // 转发信号 ..
public slots:
    void refreshData();
private slots:
    void actApply();
    void actSave();
    void onModeChanged();
    void actAdjust();
};

/**
 * @brief The TrackAdjustDialog class
 * 调整股道次序和名称的对话框
 */
class TrackAdjustDialog: public QDialog
{
    Q_OBJECT
    QVector<std::shared_ptr<Track>>& order;
    RailTrackAdjustModel* model;
    QTableView* table;
public:
    TrackAdjustDialog(QVector<std::shared_ptr<Track>>& order,
                      QWidget* parent=nullptr);
private:
    void initUI();
signals:
    void repaintDiagram();
private slots:
    void moveUp();
    void moveDown();
};


/**
 * @brief The RailTrackWidget class
 * pyETRC.StationVisualizeDialog  股道分析的对话框
 */
class RailTrackWidget : public QSplitter
{
    Q_OBJECT
    using events_t=std::vector<std::pair<std::shared_ptr<TrainLine>,
        const AdapterStation*>>;
    Diagram& diagram;
    std::shared_ptr<Railway> railway;
    std::shared_ptr<RailStation> station;

    events_t events;
    TrackDiagramData data;   // 必须在events后初始化
    TrackDiagram* trackDiagram;
    RailTrackSetupWidget* setupWidget;
    QSlider* slider;

public:
    RailTrackWidget(Diagram& diagram, std::shared_ptr<Railway> railway,
                    std::shared_ptr<RailStation> station, QWidget* parent=nullptr);
signals:
    void actSaveTrackOrder(std::shared_ptr<Railway> rail, std::shared_ptr<RailStation> station,
                      const QList<QString>& order);
    void saveTrackToTimetable(const QVector<TrainStation*>&,
                              const QVector<QString>&);
private:
    void initUI();
public slots:
    void refreshData();
private slots:
    void onSaveOrder(const QList<QString>& order);
    void actSaveTracks();
};

