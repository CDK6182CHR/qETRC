#pragma once

#include <QScrollArea>
#include <QWidget>
#include <memory>

class QFormLayout;
class QTextBrowser;
class QLineEdit;
class Train;
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
    QLineEdit* edRun,*edStay,*edTechSpeed;
    QLineEdit* edRouting,*edModel,*edOwner,*edPre,*edPost;
    QTextBrowser* edOrder;

    QFormLayout* flay;
public:
    explicit TrainInfoWidget(QWidget *parent = nullptr);
    auto getTrain(){return train;}
signals:
    void editTrain(std::shared_ptr<Train> train);
    void showTimetable(std::shared_ptr<Train> train);

private:
    void initUI();
    QLineEdit* makeLineEdit(const QString& title)const;
public slots:
    void setTrain(std::shared_ptr<Train> train);
    void refreshData();

private slots:
    void toText();
    void actEditTrain();
    void actShowTimetable();
};

