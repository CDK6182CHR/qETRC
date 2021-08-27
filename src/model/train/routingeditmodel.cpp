#include "routingeditmodel.h"
#include "data/train/routing.h"
#include "model/delegate/qedelegate.h"
#include <QWidget>
#include <QMessageBox>

RoutingEditModel::RoutingEditModel(std::shared_ptr<Routing> routing_, QWidget *parent) :
    QEMoveableModel(parent), routing(routing_)
{
    setColumnCount(ColMAX);
    setHorizontalHeaderLabels({
     tr("车次"),tr("虚拟"),tr("始发"),tr("终到"),tr("连线")
                              });
    setupModel();
}

void RoutingEditModel::refreshData()
{
    setupModel();
}

void RoutingEditModel::setRouting(std::shared_ptr<Routing> routing)
{
    this->routing=routing;
    setupModel();
}

bool RoutingEditModel::getAppliedOrder(std::shared_ptr<Routing> res)
{
    auto* par = qobject_cast<QWidget*>(parent());
    for (int i = 0; i < rowCount(); i++) {
        bool vir = item(i, ColVirtual)->data(qeutil::BoolDataRole).toBool();
        bool link = bool(qvariant_cast<Qt::CheckState>(
            item(i, ColLink)->data(Qt::CheckStateRole)));
        if (vir) {
            res->appendVirtualTrain(item(i, ColTrainName)->text(), link);
        }
        else {
            auto train = qvariant_cast<std::shared_ptr<Train>>(
                item(i, ColTrainName)->data(qeutil::TrainRole));
            // 第一：检查是否在别的交路里面
            if (train->hasRouting() && train->routing().lock() != routing) {
                QMessageBox::warning(par, tr("错误"),
                    tr("第[%1]行：列车[%2]已经属于交路[%3]！\n"
                        "请注意每一个列车对象只能属于一个交路。").arg(i + 1)
                    .arg(train->trainName().full()).arg(train->routing().lock()->name()));
                return false;
            }
            else if (int prev = res->trainIndexLinear(train); prev != -1) {
                QMessageBox::warning(par, tr("错误"),
                    tr("第[%1]行：列车[%2]已经在本次交路编辑中添加过了，不能重复添加。"
                        "参见第[%3]行。").arg(i + 1).arg(train->trainName().full()).arg(prev + 1));
                return false;
            }
            //现在可以放心添加
            res->appendTrainSimple(train, link);
        }
    }
    return true;
}

void RoutingEditModel::setupModel()
{
    if(!routing){
        setRowCount(0);
        return;
    }

    setRowCount(routing->count());

    int row=0;
    for(const auto& p:routing->order()){
        if (p.isVirtual()){
            setVirtualRow(row, p.name(), p.link());
        }else{
            setRealRow(row, p.train(), p.link());
        }
        row++;
    }
}

void RoutingEditModel::setRealRow(int row, std::shared_ptr<Train> train, bool link)
{
    using SI=QStandardItem;
    auto* it=new SI(train->trainName().full());
    QVariant v;
    v.setValue(train);
    it->setData(v,qeutil::TrainRole);
    setItem(row,ColTrainName,it);

    it=new SI;
    it->setData(false,qeutil::BoolDataRole);
    setItem(row,ColVirtual,it);

    setItem(row,ColStarting,new SI(train->starting().toSingleLiteral()));
    setItem(row,ColTerminal,new SI(train->terminal().toSingleLiteral()));

    it=makeCheckItem();
    it->setCheckState(link ? Qt::Checked : Qt::Unchecked);
    setItem(row,ColLink,it);
}

void RoutingEditModel::setVirtualRow(int row, const QString &name, bool link)
{
    using SI=QStandardItem;
    auto* it=new SI(name);
    setItem(row,ColTrainName,it);

    it=new SI(tr("√"));
    it->setData(true,qeutil::BoolDataRole);
    setItem(row,ColVirtual,it);

    setItem(row,ColStarting,new SI("-"));
    setItem(row,ColTerminal,new SI("-"));

    it=makeCheckItem();
    it->setCheckState(link ? Qt::Checked : Qt::Unchecked);
    setItem(row,ColLink,it);
}

void RoutingEditModel::insertRealRow(int row, std::shared_ptr<Train> train, bool link)
{
    insertRow(row);
    setRealRow(row, train, link);
}

void RoutingEditModel::insertVirtualRow(int row, const QString& name, bool link)
{
    insertRow(row);
    setVirtualRow(row, name, link);
}

