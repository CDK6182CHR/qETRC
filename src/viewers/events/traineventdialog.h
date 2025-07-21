#pragma once

#include <memory>
#include <QDialog>
#include <QStandardItemModel>

#include "data/diagram/trainevents.h"

struct IntervalEvent;
struct StationEvent;
class TrainAdapter;
class Railway;
class Diagram;
class Train;
class QTextStream;


/**
 * @brief The TrainEventModel class
 * 列车事件表的模型。采用只读模型，但是注意采用StandardItemModel保存数据，
 * 因为对话框存续期间，线路列车信息可能变化，导致车站指针悬空。
 */
class TrainEventModel:public QStandardItemModel
{
    Q_OBJECT;
    std::shared_ptr<Train> train;
    Diagram& diagram;

    /**
     * @brief etrcReport
     * ETRC风格的报告，计算后立即生成好
     */
    QString etrcReport;
public:

    enum Columns {
        ColRail=0,
        ColTime,
        ColPlace,
        ColMile,
        ColEvent,
        ColOther,
        ColNote,
        ColMAX
    };

    TrainEventModel(std::shared_ptr<Train> train_,
                    Diagram& diagram_,
                    QObject* parent=nullptr);

    const QString& etrc()const{return etrcReport;}

    bool exportToCsv(const QString& filename);

    TrainTime timeForRow(int row)const;
    std::shared_ptr<Railway> railForRow(int row)const;
    double mileForRow(int row)const;

    /**
     * 2022.05.14：导出到CSV文件，批量导出的核心算法部分。
     * 由于与setupModel()的算法相当一致，所以定义在这个类里面。
     */
    static void exportToCsvBatch(QTextStream& s, const TrainEventList& evlst, int period_hours);

private:
    void setupModel();
    void setStationRow(int row, std::shared_ptr<TrainAdapter> adp, const StationEvent& e);
    void setIntervalRow(int row, std::shared_ptr<TrainAdapter> adp, const IntervalEvent& e);
    
    static void exportStationRow(QTextStream& s, std::shared_ptr<TrainAdapter> adp, const StationEvent& e);
    static void exportIntervalRow(QTextStream& s, std::shared_ptr<TrainAdapter> adp, const IntervalEvent& e);
};

class QTableView;

/**
 * @brief The TrainEventDialog class
 * 列车事件表对话框
 */
class TrainEventDialog : public QDialog
{
    Q_OBJECT;
    Diagram& diagram;
    std::shared_ptr<Train> train;
    TrainEventModel* model;
    QTableView* table;
public:
    TrainEventDialog(Diagram& diagram_, std::shared_ptr<Train> train, QWidget* parent = nullptr);

    void initUI();

signals:
    void locateToEvent(int pageIndex, std::shared_ptr<const Railway> railway,
        double mile, const TrainTime&);

private slots:
    void exportETRC();
    void exportText();
    void exportExcel();
    void exportCsv();
    void actLocate();

};

