#pragma once

#include <QStyledItemDelegate>

class QSpinBox;

/**
 * @brief The GeneralSpinDelegate class
 * 一般的SpinBox代理，把创建之后的预处理工作作为模版方法。
 */
class GeneralSpinDelegate : public QStyledItemDelegate
{
public:
    explicit GeneralSpinDelegate(QObject *parent = nullptr);
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;

    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const override;
protected:
    /**
     * @brief setupSpin
     * 默认实现不做任何事
     */
    virtual void setupSpin(QSpinBox* sp)const;
};


class SteppedSpinDelegate: public GeneralSpinDelegate
{
    int _step;
public:
    SteppedSpinDelegate(int step, QObject* parent=nullptr);
protected:
    virtual void setupSpin(QSpinBox* sp)const override;
};

class SecondSpinDelegate: public GeneralSpinDelegate
{
public:
    SecondSpinDelegate(QObject* parent);
protected:
    virtual void setupSpin(QSpinBox* sp)const override;
};

