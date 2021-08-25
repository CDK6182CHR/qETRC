#pragma once

#include <QStandardItemModel>
#include <QWizardPage>
#include <QList>

#include "data/diagram/diagram.h"

class QTableView;

class ReadRulerPreviewModel: public QStandardItemModel
{
	const ReadRulerReport& data;   // 引用Page里面的数据
	const QVector<std::shared_ptr<RailInterval>>& intervals;   //按顺序维护的区间次序
public:
	enum {
		ColInterval = 0,
		ColPass,
		ColStart,
		ColStop,
		ColTypeCount,
		ColDataCount,
		ColSatisfied,
		ColMAX
	};
    ReadRulerPreviewModel(const ReadRulerReport& data_, 
		const QVector<std::shared_ptr<RailInterval>>& intervals,
		QObject* parent=nullptr);
	void setData();
};


/**
 * 预览页面。本类只负责界面，不负责具体计算。
 */
class ReadRulerPagePreview : public QWizardPage
{
    Q_OBJECT;
    ReadRulerPreviewModel*const model;
    QTableView* table;
	ReadRulerReport data;
	QVector<std::shared_ptr<RailInterval>> intervals;
    friend class ReadRulerWizard;
public:
    ReadRulerPagePreview(QWidget* parent=nullptr);
	auto* getModel() { return model; }
	void setData(ReadRulerReport&& report, 
		const QVector<std::shared_ptr<RailInterval>>& intervals);
private:
    void initUI();
};


