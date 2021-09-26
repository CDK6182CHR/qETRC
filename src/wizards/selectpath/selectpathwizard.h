#pragma once

#include <QWizard>

class PathSelectWidget;
class RailNet;
class SelectPathPageStart;
class SelectPathWizard : public QWizard
{
    Q_OBJECT
    const RailNet& net;
    SelectPathPageStart* pgStart;
    PathSelectWidget* selDown,*selUp;
    QWizardPage* pgDown,*pgUp;
public:
    SelectPathWizard(const RailNet& net, QWidget* parent=nullptr);

protected:
    virtual void initializePage(int id) override;
private:
    void initUI();
private slots:
    /**
     * @brief regenerateInversePath
     * 重新生成的按钮触发。有一个提示机制。
     */
    void regenerateInversePath();

    /**
     * 第一次进入反向的页面时自动触发。
     */
    void generateInversePath();

};
