#include "qefoldwidget.h"

#include <QLabel>
#include <QToolButton>
#include <QVBoxLayout>
#include <QStyle>
#include <QApplication>

QEFoldWidget::QEFoldWidget(QWidget *titleWidget, QWidget *paneWidget, QWidget *parent):
    QWidget(parent), _titleWidget(titleWidget), _paneWidget(paneWidget)
{
    initUI();
}

QEFoldWidget::QEFoldWidget(const QString &title, QWidget *paneWidget, QWidget *parent):
    QEFoldWidget(new QLabel(title), paneWidget, parent)
{

}

void QEFoldWidget::initUI()
{
    auto* vlay=new QVBoxLayout(this);
    auto* hlay=new QHBoxLayout;

    btnFold=new QToolButton;
    btnFold->setIcon(qApp->style()->standardIcon(QStyle::SP_ToolBarHorizontalExtensionButton));
    connect(btnFold,&QToolButton::toggled,this,&QEFoldWidget::toggle);
    hlay->addWidget(btnFold);
    hlay->addWidget(_titleWidget);
    vlay->addLayout(hlay);

    vlay->addWidget(_paneWidget);
    _paneWidget->setVisible(false);

}

void QEFoldWidget::toggle()
{
    if (m_expanded){
        // toggle collapse
        btnFold->setIcon(qApp->style()->standardIcon(QStyle::SP_ToolBarHorizontalExtensionButton));
        _paneWidget->setVisible(false);
        m_expanded=false;
    }else{
        // toggle expand
        btnFold->setIcon(qApp->style()->standardIcon(QStyle::SP_ToolBarVerticalExtensionButton));
        _paneWidget->setVisible(true);
        m_expanded=true;
    }
    emit toggled(m_expanded);
}

