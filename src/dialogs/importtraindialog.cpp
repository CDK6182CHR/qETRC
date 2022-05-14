#include "importtraindialog.h"

#ifndef QETRC_MOBILE_2
#include "util/utilfunc.h"
#include "data/train/routing.h"
#include "data/diagram/diagram.h"
#include "data/train/train.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QSplitter>
#include <QString>

#include <editors/trainlistwidget.h>

ImportTrainDialog::ImportTrainDialog(Diagram& diagram_, QWidget* parent):
	QDialog(parent),diagram(diagram_)
{
	setAttribute(Qt::WA_DeleteOnClose);
	initUI();
	resize(1200, 800);
	setWindowTitle(tr("导入车次"));
}

void ImportTrainDialog::initUI()
{
    auto* hlay = new QHBoxLayout;
    auto* sp = new QSplitter(Qt::Horizontal);
    widget = new TrainListWidget(other, nullptr);
	sp->addWidget(widget);

    auto* w = new QWidget;

	auto* vlay = new QVBoxLayout;
	auto* form = new QFormLayout;

	auto* ch = new QHBoxLayout;
	edFile = new QLineEdit;
	ch->addWidget(edFile);
	auto* btn = new QPushButton(tr("浏览"));
	connect(btn, SIGNAL(clicked()), this, SLOT(actView()));
	ch->addWidget(btn);
	vlay->addLayout(ch);

	ckLocal = new QCheckBox(tr("仅与本运行图有重叠的车次"));
	ckLocal->setChecked(true);
	form->addRow(tr("筛选"), ckLocal);

	rdConflict = new RadioButtonGroup<2,QVBoxLayout>({ "忽略冲突车次","覆盖冲突车次" }, this);
	form->addRow(tr("冲突车次"), rdConflict);
	rdConflict->get(0)->setChecked(true);

	rdRouting = new RadioButtonGroup<3,QVBoxLayout>({ "以原图交路为准","以所导入交路为准","不导入任何交路" }, this);
	form->addRow(tr("冲突交路"), rdRouting);
	rdRouting->get(0)->setChecked(true);

	edPrefix = new QLineEdit;
	form->addRow(tr("附加前缀"), edPrefix);
	edSuffix = new QLineEdit;
	form->addRow(tr("附加后缀"), edSuffix);
	vlay->addLayout(form);

    auto* label = new QLabel(tr("先选择文件名，然后在左侧列表中删除不需要导入的车次，剩余车次都将被导入。"
        "若不选择“不导入任何交路”，则将导入指定运行图（或车次数据库）中的所有交路数据；"
        "若交路名称与原有冲突则自动重命名。\n"
        "如果车次同时属于一个既有交路和一个被导入运行图中的交路时，若选择“以原图交路为准”，"
        "则将该车次划给原运行图中既有交路，而从导入的交路中删去该车次；若选择“以所导入交路为准”，"
        "则将该车次从原图中既有交路删除，而划给新导入的交路。\n\n"
        "请注意此操作不支持撤销。"));
    label->setWordWrap(true);
    vlay->addWidget(label);
	
	auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	vlay->addWidget(buttons);
	connect(buttons, SIGNAL(accepted()), this, SLOT(actApply()));
	connect(buttons, SIGNAL(rejected()), this, SLOT(actCancel()));

    w->setLayout(vlay);
    sp->addWidget(w);
    hlay->addWidget(sp);
	setLayout(hlay);
}

void ImportTrainDialog::actView()
{
	QString filename = QFileDialog::getOpenFileName(this, tr("导入车次"), {}, 
        QObject::tr("pyETRC车次数据库文件(*.pyetdb; *.json);\n"
            "pyETRC运行图文件(*.pyetgr; *.json)\nETRC运行图文件(*.trc)\n所有文件(*.*)")
    );
	if (filename.isEmpty())
		return;
	bool flag=other.fromJson(filename, diagram.trainCollection().typeManager());
	if (!flag || other.isNull()) {
		QMessageBox::warning(this, tr("错误"), tr("文件错误或为空，请检查！"));
		return;
	}
    //2022.01.23：清除非本线车次算法修改
	//diagram.applyBindOn(other);
	//if (ckLocal->isChecked())
	//	other.removeUnboundTrains();
    if (ckLocal->isChecked()) {
        other.removeNonLocal(diagram.railCategory());
    }
    diagram.applyBindOn(other);
	widget->refreshData();
	edFile->setText(filename);
}

void ImportTrainDialog::actApply()
{
    auto f = QMessageBox::question(this, tr("导入车次"), tr("导入车次操作不支持撤销。是否确认导入车次？"));
    if (f != QMessageBox::Yes)
        return;
    bool cover = rdConflict->get(1)->isChecked();
    //删除新导入的图中没有任何牵连的交路
    QList<std::shared_ptr<Routing>> routings;
    for (auto p : other.routings()) {
        if (p->anyValidTrains()) {
            routings.append(p);   //注意还是原来的对象
        }
    }

    auto& coll = diagram.trainCollection();
    //导入车次
    int new_cnt = 0;
    for (auto train : other.trains()) {
        train->trainName().setFull(edPrefix->text() + 
            train->trainName().full() + edSuffix->text());
        auto oldTrain = diagram.trainCollection().findFullName(train->trainName());
        if (!oldTrain) {
            //可以直接添加
            train->resetRouting();  // ?
            diagram.trainCollection().appendTrain(train);
            train->setType(coll.typeManager().findOrCreate(train->type()));
            new_cnt++;
        }
        else if (cover) {
            if (oldTrain->hasRouting()) {
                oldTrain->routing().lock()->replaceTrain(oldTrain, train);
            }
            coll.removeTrain(oldTrain);
            coll.appendTrain(train);
        }
        else {
            //清理交路信息
            train->resetRouting();
        }
    }

    //下面导入交路
    int routing_cnt = 0;
    if (!rdRouting->get(2)->isChecked()) {
        bool coverRouting = rdRouting->get(1)->isChecked();
        for (auto routing : other.routings()) {
            //有反向引用，必须创建新对象
            auto newRouting = std::make_shared<Routing>();
            //Move。注意此时原来交路的所有Node都应该为虚拟
            newRouting->operator=(std::move(*routing));
            newRouting->setName(coll.validRoutingName(newRouting->name()));
            for (auto p = newRouting->order().begin(); p != newRouting->order().end(); ++p) {
                //注意这是std::list！
                auto t = coll.findFullName(p->name());
                if (!t)
                    continue;   //继续虚拟
                if (!t->hasRouting()) {
                    //原来没有交路，放心添加就好
                    newRouting->setNodeTrain(t, p);
                }
                else if (coverRouting) {
                    //且以新图中交路为准，则把老交路中这个节点设置为虚拟
                    //注意pyETRC中是删除，这里改了一下
                    //其实直接执行Train的reset就好
                    t->resetRouting();
                    newRouting->setNodeTrain(t, p);
                    //t->setRouting(newRouting, p);
                }
            }
            if (newRouting->anyValidTrains()) {
                coll.routings().append(newRouting);
                routing_cnt++;
            }
        }
    }
    int all_cnt = other.trainCount();
    QString text = "";
    if (cover) {
        text += tr("成功导入%1个车次。\n").arg(all_cnt);
        text += tr("其中覆盖%1个车次。\n").arg(all_cnt - new_cnt);
    }
    else {
        text += tr("成功导入%1个车次。\n").arg(new_cnt);
        text += tr("有%1个重复车次被跳过。\n").arg(all_cnt - new_cnt);
    }
    if (routing_cnt) {
        text += tr("同时引入%1个新交路。").arg(routing_cnt);
    }
    QMessageBox::information(this, tr("信息"), text);
    emit trainsImported();
    done(QDialog::Accepted);
}

void ImportTrainDialog::actCancel()
{
	done(QDialog::Rejected);
}

#endif
