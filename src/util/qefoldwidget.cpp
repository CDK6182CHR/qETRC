#include "qefoldwidget.h"

#include <QLabel>
#include <QToolButton>
#include <QVBoxLayout>
#include <QStyle>
#include <QApplication>

QEFoldWidget::QEFoldWidget(QWidget *titleWidget, QWidget *paneWidget, QWidget *parent):
    QWidget(parent), _titleWidget(titleWidget), _paneWidget(paneWidget)
{
    //setContentsMargins(0, 0, 0, 0);
    initUI();
    //setStyleSheet("border: 3px solid");
}

QEFoldWidget::QEFoldWidget(const QString &title, QWidget *paneWidget, QWidget *parent):
    QEFoldWidget(new QLabel(title), paneWidget, parent)
{

}

void QEFoldWidget::initUI()
{
    auto* vlay=new QVBoxLayout(this);
    auto* hlay=new QHBoxLayout;
    vlay->setContentsMargins(0, 0, 0, 0);

    btnFold=new QToolButton;
    btnFold->setIcon(qApp->style()->standardIcon(QStyle::SP_ToolBarHorizontalExtensionButton));
    connect(btnFold,&QToolButton::clicked,this,&QEFoldWidget::toggle);
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

void QEFoldWidget::expand()
{
    if (!m_expanded){
        btnFold->setIcon(qApp->style()->standardIcon(QStyle::SP_ToolBarVerticalExtensionButton));
        _paneWidget->setVisible(true);
        m_expanded=true;
        emit toggled(m_expanded);
    }
}

void QEFoldWidget::collapse()
{
    if (m_expanded){
        // toggle collapse
        btnFold->setIcon(qApp->style()->standardIcon(QStyle::SP_ToolBarHorizontalExtensionButton));
        _paneWidget->setVisible(false);
        m_expanded=false;
        emit toggled(m_expanded);
    }
}

