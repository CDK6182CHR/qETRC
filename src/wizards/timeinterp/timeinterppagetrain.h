#pragma once

#include <QWizardPage>
#include <memory>
#include <QVector>

class Train;
class TrainListReadModel;
class QTableView;
class QComboBox;
class QCheckBox;
class RailRulerCombo;
class Diagram;

/**
 * @brief The TimeInterpPageTrain class
 * 首页：选择车次和杂项
 */
class TimeInterpPageTrain : public QWizardPage
{
    Q_OBJECT
    Diagram& diagram;
    RailRulerCombo* cbRuler;
    QCheckBox* ckToStart,*ckToEnd;
    QComboBox* cbPrec;
    QTableView* table;
    TrainListReadModel*const model;
    friend class TimeInterpWizard;
public:
    TimeInterpPageTrain(Diagram& diagram, QWidget* parent=nullptr);

    /**
     * @brief selectedTrains
     * 原则上，仅在下一步时调用一次。
     */
    QVector<std::shared_ptr<Train>> selectedTrains();
private:
    void initUI();
private slots:
    void actHelp();
    virtual bool validatePage() override;
};

