#pragma once

#include <QStandardItemModel>
#include <memory>
#include "data/rail/ruler.h"

/**
 * @brief The RulerModel class
 * 标尺编辑表格的模型  采用StandardItem暂存
 */
class RulerModel : public QStandardItemModel
{
    Q_OBJECT;
    std::shared_ptr<Ruler> ruler;
    bool updating=false;
public:
    enum{
        ColInterval=0,
        ColMinute,
        ColSeconds,
        ColStart,
        ColStop,
        ColMile,
        ColSpeed,
        ColMAX
    };
    explicit RulerModel(std::shared_ptr<Ruler> ruler_,
                        QObject *parent = nullptr);
    void refreshData();
private:
    void setupModel();

private slots:
    void onDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);

public slots:
    /**
     * 提交更改。暂定交由RailContext处理
     */
    void actApply();
    void actCancel();
};

