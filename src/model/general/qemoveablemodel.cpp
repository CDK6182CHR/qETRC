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

void QEMoveableModel::moveUp(int row)
{
	if (row > 0 && row < rowCount()) {
        updating=true;
		for (int c = 0; c < columnCount(); c++) {
			auto* it1 = takeItem(row - 1, c);
			auto* it2 = takeItem(row, c);
			setItem(row - 1, c, it2);
			setItem(row, c, it1);
		}
        updating=false;
	}
}

void QEMoveableModel::moveDown(int row)
{
	if (row >= 0 && row < rowCount()-1) {
		moveUp(row + 1);
	}
}

void QEMoveableModel::exchangeRow(int row1, int row2)
{
	if (row1 != row2) {
		for (int c = 0; c < columnCount(); c++) {
			auto* it1 = takeItem(row1, c);
			auto* it2 = takeItem(row2, c);
			setItem(row1, c, it2);
			setItem(row2, c, it1);
		}
	}
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
