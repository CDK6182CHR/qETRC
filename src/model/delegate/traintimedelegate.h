#pragma once

#include <QStyledItemDelegate>
#include "data/common/traintime.h"

class TrainTimeEdit;
struct DiagramOptions;

/**
 * @brief The QETimeDelegate class
 * 标准的时间编辑Delegate，包括显示格式的设置
 */
class TrainTimeDelegate : public QStyledItemDelegate
{
    Q_OBJECT
    const DiagramOptions& _ops;
    TrainTime::TimeFormat _format;
public:
    explicit TrainTimeDelegate(const DiagramOptions& ops, QObject* parent = nullptr,
        const TrainTime::TimeFormat format = TrainTime::HMS);
    virtual QWidget* createEditor(QWidget* parent,
        const QStyleOptionViewItem& option,
        const QModelIndex& index) const override;
    virtual void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    virtual void setModelData(QWidget* editor,
        QAbstractItemModel* model, const QModelIndex& index) const override;
    virtual QString displayText(const QVariant& value,
        const QLocale& locale) const override;

protected:
    virtual void setupEditor(TrainTimeEdit* ed)const;
private slots:
    void onTimeChanged();
};


/**
 * 采用TimeDataRole来保存数据；DisplayRole来展示数据。
 */
class TrainTimeQuickDelegate :public QStyledItemDelegate
{
    Q_OBJECT;
    const DiagramOptions& _ops;
    TrainTime::TimeFormat _format;
public:
    TrainTimeQuickDelegate(const DiagramOptions& ops, QObject* parent = nullptr, TrainTime::TimeFormat format = TrainTime::HMS);
    virtual QWidget* createEditor(QWidget* parent,
        const QStyleOptionViewItem& option,
        const QModelIndex& index) const override;
    virtual void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    virtual void setModelData(QWidget* editor,
        QAbstractItemModel* model, const QModelIndex& index) const override;

protected:
    virtual void setupEditor(TrainTimeEdit* ed)const;
private slots:
    void onTimeChanged();

};

