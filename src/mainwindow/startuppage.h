#pragma once

#include <QFrame>

class QCheckBox;

/**
 * @brief The StartupPage class
 * 2022.08.28  启动页面  LOGO  版本  提示信息
 */
class StartupPage : public QFrame
{
    Q_OBJECT
    QCheckBox* ckDoNotShow;
public:
    StartupPage();
    static void onStartup();
private:
    void initUI();
protected:
    virtual void closeEvent(QCloseEvent* ev)override;
};

