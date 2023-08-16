#pragma once
#include <QAbstractTableModel>
#include <vector>

class TrainPath;
/**
 * 2023.08.16  similar to PathListModel, but the underlined
 * data is just a vector of TrainPaths.
 */
class PathListReadModel : public QAbstractTableModel
{
    Q_OBJECT
    std::vector<TrainPath*> _paths;
public:
    enum {
        ColName=0,
        ColValid,
        ColStart,
        ColEnd,
        ColMAX
    };
    explicit PathListReadModel(QObject *parent = nullptr);
    const auto& paths()const{return _paths;}

    void resetData(std::vector<TrainPath*>&& paths);

    void resetData(const std::vector<TrainPath*>& paths);

    // QAbstractItemModel interface
public:
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
};

