#pragma once

#include <QStyledItemDelegate>

class QTimeEdit;

/**
 * @brief The QETimeDelegate class
 * 标准的时间编辑Delegate，包括显示格式的设置
 */
class QETimeDelegate : public QStyledItemDelegate
{
    Q_OBJECT
    QString _format;
public:
    explicit QETimeDelegate(QObject *parent = nullptr,
                            const QString& format="hh:mm:ss");
    virtual QWidget *createEditor(QWidget *parent,
                                  const QStyleOptionViewItem &option,
                                  const QModelIndex &index) const override;
    virtual void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    virtual void setModelData(QWidget *editor,
                              QAbstractItemModel *model, const QModelIndex &index) const override;
    virtual QString displayText(const QVariant &value,
                                const QLocale &locale) const override;

protected:
    virtual void setupEditor(QTimeEdit* ed)const;
private slots:
    void onTimeChanged();
};

