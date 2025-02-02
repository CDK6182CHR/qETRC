#pragma once

#include <QDialog>

#include "model/general/qemoveablemodel.h"
#include "util/selecttraincombo.h"
#include "util/qecontrolledtable.h"

class BatchCopyTrainModel:
        public QEMoveableModel
{
    Q_OBJECT
public:
    enum{
        ColTrainName=0,
        ColStartTime,
        ColMAX
    };
    BatchCopyTrainModel(QObject* parent=nullptr);

    void addRow(const QString& trainName, const QTime& time);

protected:
    virtual void setupNewRow(int row) override;
};

class QLineEdit;
class QSpinBox;
class QTimeEdit;
class Diagram;
class Train;

/**
 * @brief The BatchCopyTrainDialog class
 * pyETRC传承功能扩展，批量复制运行线
 * 扩展为可选参考车次; 默认为当前车次。
 */
class BatchCopyTrainDialog : public QDialog
{
    Q_OBJECT;
    Diagram& diagram;
    TrainCollection& coll;
    std::shared_ptr<Train> train;

    SelectTrainCombo* cbTrain;
    QEControlledTable* ctab;
    BatchCopyTrainModel* const model;
    QTableView *table;
    QLineEdit* edFormat, *edRef;
    QTimeEdit* edStartTime;
    QSpinBox* spStartNumber, *spCount, *spMin, *spSec;

public:
    BatchCopyTrainDialog(Diagram& diagram_, std::shared_ptr<Train> train_,
                         QWidget* parent=nullptr);
private:
    void initUI();

signals:
    /**
     * @brief applied
     * 将（校验车次通过后的）列车表发送给NaviModel（暂定）处理
     * 参考导入线路ImportTrain的操作
     * seealso: qecmd::addNewTrain 操作流程
     */
    void applied(const QVector<std::shared_ptr<Train>>& trains);
private slots:
    void setTrain(std::shared_ptr<Train> train);
    void onApply();
    void onGenerateArray();
    void onShowHelp();
};

