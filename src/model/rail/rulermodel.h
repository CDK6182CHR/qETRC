#pragma once

#include <memory>
#include "data/rail/ruler.h"
#include "intervaldatamodel.h"

/**
 * @brief The RulerModel class
 * 标尺编辑表格的模型  采用StandardItem暂存
 * 注意  构造传入的对象非空！！
 */
class RulerModel : public IntervalDataModel
{
    Q_OBJECT;
    std::shared_ptr<Ruler> ruler;
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
    void setupModel()override;

    void setupRow(int row, std::shared_ptr<RailInterval> railint)override;

    int rowIntervalSecs(int row)const;
    QString intervalString(const RailInterval& railint)const;
    void updateRowSpeed(int row);
protected:
    void copyRowData(int from, int to)override;

    void updateEquivRailIntRow(int row, std::shared_ptr<RailInterval> railint)override;

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

