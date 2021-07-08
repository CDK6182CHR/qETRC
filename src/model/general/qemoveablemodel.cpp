#include "qemoveablemodel.h"

QEMoveableModel::QEMoveableModel(QObject* parent) : QStandardItemModel(parent)
{

}

bool QEMoveableModel::moveRows(const QModelIndex& sourceParent, int sourceRow, int count, const QModelIndex& destinationParent, int destinationChild)
{
	updating = true;
	if (sourceParent == destinationParent) {
		for (int i = 0; i < count; i++) {
			int src = sourceRow + i, dst = destinationChild + i;
			if (src != dst) {
				//在dst前面插入一行，内容为src；然后删除src那一行
				insertRow(dst);
				int nsrc = src > dst ? src + 1 : src;
				for (int j = 0; j < columnCount(); j++) {
					setItem(dst, j, takeItem(nsrc, j));  //行号可能变了!
				}
				removeRow(nsrc);
			}
		}
	}
	updating = false;
	return true;
}

bool QEMoveableModel::insertRows(int row, int count, const QModelIndex& parent)
{
	updating = true;
	bool flag = QStandardItemModel::insertRows(row, count, parent);
	if (flag)
		for (int i = 0; i < count; i++)
			setupNewRow(row + i);
	updating = false;
	return flag;
}

void QEMoveableModel::setupNewRow(int row)
{
    Q_UNUSED(row);
}

QStandardItem *QEMoveableModel::makeCheckItem() const
{
    auto* item=new QStandardItem;
    item->setCheckable(true);
    item->setEditable(false);
    item->setTextAlignment(Qt::AlignCenter);
    return item;
}
