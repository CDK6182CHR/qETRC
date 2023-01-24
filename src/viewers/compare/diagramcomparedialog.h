#pragma once
#include <QDialog>
#include <QStandardItemModel>
#include <data/diagram/diadiff.h>

class Diagram;
class TrainCollection;
class TrainDifference;
/**
 * @brief The DiagramCompareModel class
 * 运行图对比的Model, 使用stdModel实现，
 * 主要是因为颜色等如果分别取会涉及多次类型判断，不合适
 */
class DiagramCompareModel: public QStandardItemModel
{
    Q_OBJECT
    diagram_diff_t diagram_diff;

public:
    enum {
        ColTrainName=0,
        ColStarting1,
        ColTerminal1,
        ColDiff,
        ColStarting2,
        ColTerminal2,
        ColMAX
    };
    DiagramCompareModel(QObject* parent=nullptr);
    auto& diagramDiff(){return diagram_diff;}
    void resetData(diagram_diff_t&& diff);
private:
    void refreshData();

};

class QLineEdit;
class QCheckBox;
class TrainFilterSelector;
class QTableView;
class TrainCompareDialog;

class DiagramCompareDialog : public QDialog
{
    Q_OBJECT
    Diagram& diagram;
    DiagramCompareModel* const model;
    QLineEdit* edFile;
    QCheckBox* ckLocal,*ckChanged;
    TrainFilterSelector* filter;

    QTableView* table;
public:
    DiagramCompareDialog(Diagram& diagram, QWidget* parent=nullptr);
private:
    void initUI();
    bool loadFile(const QString& filename);
signals:
    void showStatus(const QString& text);
private slots:
    void viewFile();
    void onLocalToggled(bool on);
    void onChangeToggled(bool on);
    void onFilterApplied();
    void openTrainDialog(std::shared_ptr<TrainDifference> diff);
    void openTimetable(std::shared_ptr<TrainDifference> diff);
    void showTrainTimetable(std::shared_ptr<const Train> train);
    void onDoubleClicked(const QModelIndex& idx);
    void resetRowShow();
    void actTrainDiff();
    void actTrainTimetable();
};

