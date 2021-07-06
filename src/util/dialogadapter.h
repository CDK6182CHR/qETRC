#pragma once

#include <QDialog>
#include <QWidget>


class DialogAdapter : public QDialog
{
    Q_OBJECT;
    QWidget* widget;
public:
    DialogAdapter(QWidget* widget_,QWidget* parent=nullptr,
                  bool closeButton=true);
};

