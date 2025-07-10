#pragma once

#include <util/qecontrolledtable.h>

class TimetableStdModel;
class RailStation;

/**
 * @brief The TimetableWidget class
 * 2022.04.10
 * BasicTrainWidget和EditTrainWidget的公共部分，
 * 从相应部分抽离出来，主要是需要加复制到开时刻的事件处理，避免重复代码。
 * Model和View在这里合流，类似于QTableWidget
 */
class TimetableWidget : public QEControlledTable
{
    Q_OBJECT
    TimetableStdModel* const _model;
public:
    TimetableWidget(bool commitInPlace, QWidget* parent=nullptr);
    void refreshData();
    auto* model(){return _model;}

private:
    void initUI();
private slots:
    void copyToDepart();
    void copyToArrive();

public slots:

    /**
     * Append rows to the table.
     * All rows are automatically set-up using the methods provided by the model.
     */
    void appendRows(int rows);
    
    /**
     * 2025.07.10: Append the stations to the end of the timetable.
     * The stations are selected from railway.
     */
	void appendStations(const QList<std::shared_ptr<const RailStation>>& stations);
};
