
#include"linestylecombo.h"
#include<QHBoxLayout>
#include<QPainter>
#include<QPaintEvent>
#include<QPoint>
#include<QLine>

QPenCssEditor::QPenCssEditor(QWidget* parent /*= Q_NULLPTR*/)
    :QLabel(parent)
    , m_ps(Qt::SolidLine)
    , m_size(1)
{
}


QPenCssEditor::~QPenCssEditor()
{
}

void QPenCssEditor::SetDrawParameters(Qt::PenStyle ps, int isize)
{
    m_ps = ps;
    m_size = isize;
}


void QPenCssEditor::paintEvent(QPaintEvent* event)
{
    QSize size = this->size();
    QPainter painter(this);
    QPen pen;
    pen.setColor(Qt::black);
    //pen.setWidth(m_size);
    pen.setStyle(m_ps);
    painter.setPen(pen);
    QPoint p1(0, size.height() / 2);
    QPoint p2(size.width(), size.height() / 2);
    QLine line(p1, p2);
    painter.drawLine(line);
}


QPenWidget::QPenWidget(QWidget* parent /*= Q_NULLPTR*/)
    :QLineEdit(parent)
    , m_index(0)
    , m_ps(Qt::SolidLine)
{
    m_pLabel = new QLabel();
    m_pCssLabel = new QPenCssEditor();

    m_pLabel->setFixedSize(12, 12);

    QHBoxLayout* layout = new QHBoxLayout();
    layout->addWidget(m_pLabel);
    layout->addWidget(m_pCssLabel);
    layout->setContentsMargins(5, 0, 0, 2);
    setLayout(layout);
    setReadOnly(true);

    setStyleSheet("QLineEdit{border: none;}QLineEdit:hover{background-color:rgb(0,125,255);}");
}


QPenWidget::~QPenWidget()
{
}

void QPenWidget::updatePen(const int& index, Qt::PenStyle ps)
{
    m_index = index;
    m_ps = ps;

    QString strText = QString("%1 )").arg(QString::number(m_index));
    m_pLabel->setText(strText);
    m_pCssLabel->SetDrawParameters(m_ps, index);

}

void QPenWidget::mousePressEvent(QMouseEvent* event)
{
    emit click(m_index);
}

PenStyleCombo::PenStyleCombo(QWidget* parent /*= Q_NULLPTR*/)
    :QComboBox(parent)
{
    m_pPenEdit = new QPenWidget();
    m_pListWidget = new QListWidget();
    m_pPenEdit->setStyleSheet("");
    setContextMenuPolicy(Qt::NoContextMenu);//禁用菜单
    m_pListWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);//禁用垂直滚动条
    m_pListWidget->setStyleSheet("QListWidget::Item:hover{background-color:rgb(0,125,255);}");
    setLineEdit(m_pPenEdit);
    setModel(m_pListWidget->model());
    setView(m_pListWidget);
    for (int i = 0; i < 6; i++)
        appendItem(i);
}

PenStyleCombo::~PenStyleCombo()
{
}

void PenStyleCombo::SetType(int drawType)
{
    m_drawType = drawType;
}

void PenStyleCombo::appendItem(const int& index)
{
    QPenWidget* pWid = new QPenWidget(this);
    Qt::PenStyle ps = static_cast<Qt::PenStyle>(index);
    pWid->updatePen(index, ps);
    connect(pWid, SIGNAL(click(const int&)), this, SLOT(onClickPenWidget(const int&)));
    QListWidgetItem* pItem = new QListWidgetItem(m_pListWidget);

    m_pListWidget->addItem(pItem);
    m_pListWidget->setItemWidget(pItem, pWid);

}

void PenStyleCombo::onClickPenWidget(const int& index)
{
    m_index = index;
    m_pPenEdit->updatePen(index, static_cast<Qt::PenStyle>(index));
    hidePopup();
    emit SelectedItemChanged(m_index);
}


int PenStyleCombo::currentIndex()
{
    return m_index;
}

void PenStyleCombo::setCurrentIndex(int index)
{
    m_index = index;
    m_pPenEdit->updatePen(index, static_cast<Qt::PenStyle>(index));
}

void PenStyleCombo::SetList(QList<int>& list)
{
    m_list = list;
    m_pListWidget->clear();
    int icount = m_list.count();
    for (int i = 0; i < icount; i++)
        appendItem(m_list[i]);
}

QList<int> PenStyleCombo::GetList()
{
    return m_list;
}

