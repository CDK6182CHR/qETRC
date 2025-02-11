#pragma once

#include <QAbstractTableModel>

#include <memory>
#include <vector>

#include "data/trainpath/pathruler.h"

class TrainPath;

struct PathRulerSegmentData {
    bool valid;
    int interval_seconds = 0;
    double run_peed = 0;
};

/**
 * 2025.02.10  Experimental
 * Here we try a new method for managing the data for model. We use an abstract model framework, 
 * with a COPY of data managed by this class. Each time a PathRuler is assigned, the data is COPIED into
 * the local data.
 * On applied, another copy of the managed data should be provided.
 * The model must be initialized with a non-empty TrainPath object.
 */
class PathRulerModel : public QAbstractTableModel
{
    Q_OBJECT
	PathRuler m_ruler;
    TrainPath* m_path;
    std::vector<PathRulerSegmentData> m_seg_data;

public:
    enum Columns {
        ColRailName = 0,
        ColStartStation,
        ColEndStation,
        ColDir,
        ColMile,
        ColRulerName,
        ColPassTime,
        ColRunSpeed,
        ColMAX,
    };

	PathRulerModel(PathRuler ruler, QObject* parent = nullptr);

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;

    /**
     * This is only valid for the RulerName column
     */
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    const auto& ruler()const { return m_ruler; }
    const auto& path()const { return m_path; }

private:
    void computeSegmentData();

public slots:
    void setRuler(PathRuler ruler);
};
