#include "startuppage.h"

#include <QCheckBox>
#include <QLabel>
#include <QPicture>
#include <QPushButton>
#include <QTextBrowser>
#include <QVBoxLayout>
#include <QCloseEvent>

#include <mainwindow/version.h>
#include <data/common/qesystem.h>


StartupPage::StartupPage():
    QFrame()
{
    initUI();
}

void StartupPage::onStartup()
{
    if (SystemJson::instance.show_start_page){
        auto* page=new StartupPage;
        page->setAttribute(Qt::WA_DeleteOnClose);
        page->show();
    }
}

void StartupPage::initUI()
{
    setFrameShape(StyledPanel);
    resize(800,600);
    setWindowFlags(Qt::SplashScreen|Qt::WindowStaysOnTopHint);

    auto* vlay=new QVBoxLayout(this);
    auto* hlay=new QHBoxLayout;
    vlay->addLayout(hlay);
    auto* lab=new QLabel;
    lab->setPixmap(QPixmap(":/icons/icon-transparent.png"));
    hlay->addWidget(lab);
    lab->setFixedHeight(100);

    QFont fontVersion("Arial", 18);
    lab=new QLabel(qespec::VERSION.data());
    lab->setAlignment(Qt::AlignBottom | Qt::AlignLeft);
    lab->setFont(fontVersion);
    hlay->addWidget(lab);

    lab=new QLabel(tr("R%2   %1").arg(qespec::DATE.data())
                   .arg(qespec::RELEASE_CODE));
    lab->setFont(fontVersion);
    lab->setAlignment(Qt::AlignBottom | Qt::AlignLeft);
    hlay->addWidget(lab);
    vlay->addSpacing(40);

    QFont font2("黑体",20);

    QFont font3;
    font3.setPointSize(14);

    lab=new QLabel(tr("<font color=\"#41cd52\">提示</font>"));

    lab->setFont(font2);
    vlay->addWidget(lab);

    QString txt=tr(R"(
1. 针对运行图数据元素的操作，请选中对应元素后，在<font color="red">工具栏</font>对应页面操作。<br>
2. 如果运行线显示不完整，首先检查<font color="red">最大跨越站数</font>的设置是否正确。<br>
3. 在线文档：<a href="%1">%1</a><br>
4. 疑问与反馈请联系：mxy0268@qq.com。提出任何问题以前，请至少确保已经阅读过在线文档的《快速上手》章节。
)").arg(qespec::DOC_URL.data());
    lab=new QLabel(txt);
    lab->setWordWrap(true);
    lab->setOpenExternalLinks(true);

    lab->setFont(font3);
    vlay->addWidget(lab);

    hlay=new QHBoxLayout;
    lab=new QLabel(tr("<font color=\"#41cd52\">更新日志</font>"));
    lab->setFont(font2);
    hlay->addWidget(lab);
    lab=new QLabel(tr("%1 → %2").arg(qespec::LAST_VERSION.data(),
                                     qespec::VERSION.data()));
    QFont font4;
    font4.setPointSize(14);
    lab->setFont(font4);
    hlay->addStretch(1);
    hlay->addWidget(lab);
    hlay->addStretch(5);

    vlay->addSpacing(20);
    vlay->addLayout(hlay);

    lab=new QLabel(qespec::UPDATE_LOG.data());
    lab->setWordWrap(true);
    lab->setFont(font3);
    vlay->addWidget(lab);

    vlay->addStretch(1);

    lab = new QLabel(tr("Please note that qETRC is a free software that comes WITHOUT ANY WARRANTY."));
    vlay->addWidget(lab);
    hlay=new QHBoxLayout;
    ckDoNotShow=new QCheckBox(tr("不再显示此页面"));
    hlay->addWidget(ckDoNotShow);

    hlay->addStretch(1);
    auto* btn=new QPushButton("关闭");
//    btn->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
    hlay->addWidget(btn);
    vlay->addLayout(hlay);

    connect(btn, &QPushButton::clicked,this,&StartupPage::close);
}

void StartupPage::closeEvent(QCloseEvent *ev)
{
    SystemJson::instance.show_start_page=!ckDoNotShow->isChecked();
    ev->accept();
}
