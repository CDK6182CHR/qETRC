#include "typemodel.h"

#include "data/train/traintype.h"
#include "model/delegate/qedelegate.h"

TypeConfigModel::TypeConfigModel(TypeManager &manager_, QObject *parent):
    QEMoveableModel(parent),manager(manager_)
{
    setColumnCount(ColMAX);
    setHorizontalHeaderLabels({tr("类型名"),tr("客车"),tr("颜色"), tr("线宽"),tr("线型")});
    setupModel();
}

void TypeConfigModel::refreshData()
{
    setupModel();
}

std::pair<QMap<QString, std::shared_ptr<TrainType> >,
    QVector<QPair<std::shared_ptr<TrainType>, std::shared_ptr<TrainType> > > >
    TypeConfigModel::appliedData()
{
    QMap<QString, std::shared_ptr<TrainType>> data;
    QVector<QPair<std::shared_ptr<TrainType>, std::shared_ptr<TrainType> > >  modified;

    for(int i=0;i<rowCount();i++){
        auto ptr=qvariant_cast<std::shared_ptr<TrainType>>(
                         item(i,ColName)->data(qeutil::TrainTypeRole));
        if(ptr){
            QPen pen(item(i,ColColor)->background(),
                     item(i,ColWidth)->data(Qt::EditRole).toDouble(),
                     static_cast<Qt::PenStyle>(item(i,ColLineStyle)->data(Qt::EditRole).toInt()));
            auto np=std::make_shared<TrainType>(item(i,ColName)->text(),pen,
                    item(i,ColPassenger)->checkState()==Qt::Checked );
            data.insert(np->name(), ptr);
            if(ptr->operator!=(*np)){
                // 对象发生调整
                modified.append(qMakePair(ptr,np));
            }
        }
    }
    return std::make_pair(data,modified);
}

void TypeConfigModel::setupModel()
{
    setRowCount(manager.types().size());
    int row=0;
    setupType(row++,manager.getDefaultType());
    for(auto p=manager.types().begin();p!=manager.types().end();++p){
        if(p.value() != manager.getDefaultType()){
            setupType(row++,p.value());
        }
    }
}

void TypeConfigModel::setColorItem(int row, const QColor &color)
{
    auto it=new QStandardItem(color.name().toUpper());
    it->setBackground(color);
    it->setEditable(false);
    setItem(row,ColColor,it);
}

void TypeConfigModel::setupType(int row, std::shared_ptr<TrainType> type)
{
    using SI=QStandardItem;
    auto* it=new SI(type->name());
    QVariant v;
    v.setValue(type);
    it->setData(v,qeutil::TrainTypeRole);
    setItem(row,ColName,it);

    it=makeCheckItem();
    it->setCheckState(type->isPassenger()?Qt::Checked:Qt::Unchecked);
    setItem(row,ColPassenger,it);

    setColorItem(row, type->pen().color());

    it=new SI();
    it->setData(type->pen().widthF(),Qt::EditRole);
    setItem(row,ColWidth,it);

    it=new SI;
    it->setData(static_cast<int>(type->pen().style()),Qt::EditRole);
    setItem(row,ColLineStyle,it);
}

void TypeConfigModel::setupNewRow(int row)
{
    using SI=QStandardItem;
    setItem(row,ColName,new SI);
    setItem(row,ColPassenger,makeCheckItem());
    setColorItem(row, manager.getDefaultPen().color());

    auto* it=new SI;
    it->setData(manager.getDefaultPen().widthF(),Qt::EditRole);
    setItem(row,ColWidth,it);

    it=new SI;
    it->setData(static_cast<int>(manager.getDefaultPen().style()),Qt::EditRole);
    setItem(row,ColLineStyle,it);
}

