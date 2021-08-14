#pragma once
#include <QStyledItemDelegate>

class QDoubleSpinBox;

/**
 * @brief The GeneralDoubleSpinDelegate class
 * 默认三位小数
 */
class GeneralDoubleSpinDelegate : public QStyledItemDelegate
{
    int _decimals;
public:
    explicit GeneralDoubleSpinDelegate(QObject *parent = nullptr, int decimals=3);


    virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                                  const QModelIndex &index) const override;
    virtual void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    virtual void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
    virtual QString displayText(const QVariant &value, const QLocale &locale) const override;

protected:
    virtual void setupSpin(QDoubleSpinBox* sp)const;
};

