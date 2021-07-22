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

    /**
     * apply时调用
     * 根据当前model的内容，生成新的Ruler对象（放在返回的Railway里面）
     */
    std::shared_ptr<Railway> appliedRuler();
private:
    void setupModel();
    int rowIntervalSecs(int row)const;

private slots:
    void onDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);

public slots:
    /**
     * 提交更改。暂定交由RailContext处理
     * --取消
     * 与RailTable不一样，这个交给RulerWidget负责
     * see: appliedRuler()
     */
    //void actApply();
    void actCancel();
};

