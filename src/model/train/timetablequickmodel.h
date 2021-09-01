#pragma once

#include <QStandardItemModel>
#include <memory>

class Train;

/**
 * @brief The TimetableQuickModel class
 * 双行式的列车时刻表
 */
class TimetableQuickModel : public QStandardItemModel
{
    std::shared_ptr<Train> train;
public:
    enum {
        ColName,
        ColTime,
        ColNote,
        ColMAX
    };

    explicit TimetableQuickModel(QObject *parent = nullptr);
    auto getTrain(){return train;}
private:
    void setupModel();
    void setTimeItem(int row,const QTime& tm);
    void setStationColor(int row, const QColor& color);
public slots:
    void refreshData();
    void setTrain(std::shared_ptr<Train> train);
};

