#pragma once

#include <QWidget>

class TrainFilterBasicWidget;
class QLineEdit;
class TrainCollection;
class PredefTrainFilterCore;
class QTextEdit;

/**
 * @brief The PredefTrainFilterWidget class
 * The widget for editing PredefTrainFilterCore.
 * The main part is TrainFilterBasicWidget, but add editors for
 * name and descriptions
 */
class PredefTrainFilterWidget : public QWidget
{
    Q_OBJECT
    TrainCollection& coll;
    PredefTrainFilterCore* core=nullptr;

    TrainFilterBasicWidget* basic;
    QLineEdit* edName;
    QTextEdit* edNote;
public:
    explicit PredefTrainFilterWidget(TrainCollection& coll, QWidget *parent = nullptr);
    auto* getCore(){return core;}
    
private:
    void initUI();

public slots:

    void setCore(PredefTrainFilterCore* core);

    /**
     * @brief refreshData
     * Note, this must be nullable for core
     */
    void refreshData();
    void actApply();
    void actClear();
    void clearNotChecked();

signals:
    void changeApplied(PredefTrainFilterCore* filter,
        std::unique_ptr<PredefTrainFilterCore>& data);

private slots:
    void informPredef();
};
