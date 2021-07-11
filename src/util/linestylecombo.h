#pragma once

#include<QComboBox>
#include<QLabel>
#include<QLineEdit>
#include<QListWidget>



/**
 * @brief The LineStyleCombo class
 * 选择线型的ComboBox以及相关的实现  直接复制代码少量改动
 * https://blog.csdn.net/qq_33029733/article/details/109084388
 */
class QPenCssEditor :public QLabel
{
    Q_OBJECT
public:
    QPenCssEditor(QWidget* parent = Q_NULLPTR);
    ~QPenCssEditor();

    void SetDrawParameters(Qt::PenStyle ps, int isize);
    void paintEvent(QPaintEvent*) override;
private:
    Qt::PenStyle m_ps;//画点，还是画线
    int m_size;//大小
};


class QPenWidget : public QLineEdit
{
    Q_OBJECT
public:
    QPenWidget(QWidget* parent = Q_NULLPTR);
    ~QPenWidget();

    void updatePen(const int& index, Qt::PenStyle ps);

    void mousePressEvent(QMouseEvent* event);

signals:
    void click(const int& index);
private:
    QLabel* m_pLabel;
    QPenCssEditor* m_pCssLabel;
    int m_index;
    Qt::PenStyle m_ps;
};


class PenStyleCombo :public QComboBox
{
    Q_OBJECT
public:
    PenStyleCombo(QWidget* parent = Q_NULLPTR);
    ~PenStyleCombo();

    void SetType(int drawType);

    int currentIndex();
    void setCurrentIndex(int idx);

    void SetList(QList<int>& list);
    QList<int> GetList();

private slots:
    void onClickPenWidget(const int& index);

signals:
    void SelectedItemChanged(int);

private:
    void appendItem(const int& index);
private:
    QPenWidget* m_pPenEdit;
    QListWidget* m_pListWidget;
    QList<QColor> m_colorsList;
    int m_drawType;
    int m_index;
    QList<int> m_list;
};


