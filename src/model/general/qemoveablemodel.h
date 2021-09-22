#pragma once

#include <QStandardItemModel>

/**
 * @brief The QEMoveableModel class
 * 对QStandardItemModel的轻度扩展，支持上移、下移操作
 */
class QEMoveableModel : public QStandardItemModel
{
	Q_OBJECT
protected:
	bool updating = false;
public:
	explicit QEMoveableModel(QObject* parent = nullptr);

    /**
     * QStandardItemModel似乎没有实现这玩意，因此只有自己写。
     * 这个API有点小题大做。
     * preconditions:
     * （1）sourceParent==destinationParent （否则返回false）
     * （2）src和dest的范围不重合 （否则UB）
     */
	virtual bool moveRows(const QModelIndex& sourceParent, int sourceRow,
		int count, const QModelIndex& destinationParent, int destinationChild) override;

	virtual bool insertRows(int row, int count,
		const QModelIndex& parent = QModelIndex()) override;

	/**
	 * 2021.09.21重写：不再调用moveRows；
	 * 直接写和上一行交换item。
	 */
	void moveUp(int row);
	void moveDown(int row);

	void exchangeRow(int row1, int row2);

protected:
	/**
	 * @brief setupNewRow  模版方法
	 * 新插入的行执行的初始化操作，需子类重写。
	 * 默认实现不做任何事
	 */
	virtual void setupNewRow(int row);

    /**
     * 方便函数，生成一个只能Check不能Edit的Item
     */
    QStandardItem* makeCheckItem()const;

};


