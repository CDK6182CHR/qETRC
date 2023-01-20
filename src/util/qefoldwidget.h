#pragma once

#include <QWidget>

class QToolButton;
/**
 * @brief The QEFoldWidget class
 * Widget with a title bar and could be folded.
 * like, eg, https://www.jianshu.com/p/e38a9f8470eb
 */
class QEFoldWidget : public QWidget
{
    Q_OBJECT
    QToolButton* btnFold;
    QWidget* _titleWidget;
    QWidget* _paneWidget;
    bool m_expanded=false;
public:

    /**
     * @brief QEFoldWidget
     * Construct with existing title and pane widgets.
     * The FoldWidget takes the ownership of such two widgets.
     */
    explicit QEFoldWidget(QWidget* titleWidget, QWidget* paneWidget,
            QWidget *parent = nullptr);

    /**
     * @brief QEFoldWidget
     * A version for convenience, that creates a QLabel for titleWidget.
     */
    explicit QEFoldWidget(const QString& title, QWidget* paneWidget,
            QWidget *parent=nullptr);

    auto* titleWidget(){return _titleWidget;}
    auto* paneWidget(){return _paneWidget;}
//    void setTitleWidget(QWidget* w){_titleWidget=w;}
//    void setPaneWidget(QWidget* w){_paneWidget=w;}

private:
    void initUI();

signals:
    void toggled(bool expanded);

public slots:
    void toggle();
};

