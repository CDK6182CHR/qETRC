#pragma once

#include <QScrollArea>
#include <QWidget>
#include <memory>

class QFormLayout;
class QTextBrowser;
class QLineEdit;
class Train;
class Routing;
/**
 * @brief The TrainInfoWidget class
 * pyETRC.TrainInfoWidget  列车信息（速览信息）
 * 全局单例
 */
class TrainInfoWidget : public QScrollArea
{
    Q_OBJECT
    std::shared_ptr<Train> train{};

    QLineEdit* edName,*edNameDir,*edStartEnd,*edType,*edPassen;
    QLineEdit* edStations, *edLines,*edMile,*edTime,*edSpeed;
    QLineEdit* edTechSpeed;
    QLineEdit* edRouting,*edModel,*edOwner,*edPre,*edPost;

    // 2023.02.14: add the following corresponding to total information
    QLineEdit* edTotTime, * edTotRun, * edTotStay, * edTotMile, * edTotSpeed, * edTotTechSpeed;
    QTextBrowser* edOrder;

    QFormLayout* flay;
public:
    explicit TrainInfoWidget(QWidget *parent = nullptr);
    auto getTrain(){return train;}
signals:
    void editTrain(std::shared_ptr<Train> train);
    void editTimetable(std::shared_ptr<Train> train);
    void showTimetable(std::shared_ptr<Train> train);
    void switchToRouting(std::shared_ptr<Routing> routing);

private:
    void initUI();
    QLineEdit* makeLineEdit(const QString& title)const;

    bool checkNotEditingTrain();
    
public slots:
    void setTrain(std::shared_ptr<Train> train);
    void resetTrain();
    void refreshData();
    void clearData();

private slots:
    void toText();
    void actEditTimetable();
    void actEditTrain();
    void actShowTimetable();
    void actSwitchToRouting();
};

