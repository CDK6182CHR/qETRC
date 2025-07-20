#pragma once
#include <QStandardItemModel>

#include <QList>
#include <memory>

class Train;
struct DiagramOptions;

/**
 * @brief The TrainListStdModel class
 * 对一组任意的Train的数据展示，并不借用TrainCollection。
 * 数据提供只读接口
 * 2022.05.06  增加刷新接口
 */
class TrainListReadModel : public QAbstractTableModel
{
    Q_OBJECT;
    const DiagramOptions& _ops;
    QList<std::shared_ptr<Train>> _trains{};
public:
    enum {
        ColTrainName=0,
        ColStarting,
        ColTerminal,
        ColType,
        ColShow,
        ColMile,
        ColSpeed,
        ColMAX
    };
    explicit TrainListReadModel(const DiagramOptions& ops, QObject *parent = nullptr);
    explicit TrainListReadModel(const DiagramOptions& ops, const QList<std::shared_ptr<Train>>& trains,
                                QObject* parent=nullptr);
    const auto& trains()const {return _trains;}

    virtual int rowCount(const QModelIndex &parent) const override;
    virtual int columnCount(const QModelIndex &parent) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;
    virtual void sort(int column, Qt::SortOrder order) override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    void appendTrain(std::shared_ptr<Train> train);
    void removeTrainAt(int i);
    std::shared_ptr<Train> takeTrainAt(int i);
    void clearTrains();
    void appendTrains(const QList<std::shared_ptr<Train>> train);

    void resetList(QList<std::shared_ptr<Train>>&& trains);
};

