#pragma once

#include <QDialog>
#include <QStandardItemModel>
#include <functional>
#include <vector>
#include <utility>

#include "data/diagram/trainevents.h"
#include "data/common/qeglobal.h"

class QCheckBox;
class Diagram;
class Railway;
class RailStation;
class TrainFilter;
class QLabel;
class QTableView;
class TrainFilter;

class RailStationEventListModel:public QStandardItemModel
{
    Diagram& diagram;
    std::shared_ptr<Railway> rail;
    std::shared_ptr<RailStation> station;
    RailStationEventList lst;
public:
    enum {
        ColTrainName = 0,
        ColTime,
        ColEventType,
        ColPos,
        ColTrainType,
        ColDirection,
        ColStarting,
        ColTerminal,
        ColModel,
        ColOwner,
        ColNote,
        ColMAX
    };
    RailStationEventListModel(Diagram &diagram,
                              const std::shared_ptr<Railway> &rail,
                              const std::shared_ptr<RailStation> &station,
                              QObject *parent = nullptr);

    void setupModel();
    const auto& getData()const { return lst; }
    std::shared_ptr<const Train> trainForRow(int row)const;
    QTime timeForRow(int row)const;
};

class RailStationEventListDialog : public QDialog
{
    Q_OBJECT;
    Diagram& diagram;
    std::shared_ptr<Railway> rail;
    std::shared_ptr<RailStation> station;
    RailStationEventListModel* const model;

    QTableView* table;
    QCheckBox* ckPosPre, *ckPosPost;
    QLabel* lbCount;
    TrainFilter*const filter;
public:
    RailStationEventListDialog(Diagram &diagram,
                               const std::shared_ptr<Railway> &rail,
                               const std::shared_ptr<RailStation> &station,
                               QWidget *parent=nullptr);
private:
    void initUI();

    /**
     * 根据pred所示条件筛选行。显示所给行，其他的不管。
     * bool pred(int row);
     */
    void showTableRows(std::function<bool(int)> pred);

    /**
     * 隐藏所给行，其他的不管
     */
    void hideTableRows(std::function<bool(int)> pred);

    /**
     * 根据pred，返回true的显示，否则隐藏
     */
    void filtTableRows(std::function<bool(int)> pred);
    void updateShownRows(int cnt);

signals:
    void locateOnEvent(int pageIndex, std::shared_ptr<const Railway>,
        std::shared_ptr<const RailStation>, const QTime&);

private slots:

    void onPreShowChanged(bool on);
    void onPostShowChanged(bool on);

    void onPosShowChanged();

    void toCsv();

    void gapAnalysis();

    void onFilterChanged();

    void actLocate();

};

