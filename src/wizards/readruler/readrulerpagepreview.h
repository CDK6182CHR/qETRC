#pragma once

#include <QStandardItemModel>
#include <QWizardPage>
#include <QList>

#include "data/diagram/diagram.h"
#include "util/dialogadapter.h"

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
	void refreshData();
};

/**
 * 计算细节表格
 */
class ReadRulerDetailModel : public QStandardItemModel
{
public:
	enum {
		ColTrainName = 0,
		ColAttach,
		ColStd,
		ColReal,
		ColDiff,
		ColDiffAbs,
		ColMark,
		ColMAX
	};
	ReadRulerDetailModel(QObject* parent = nullptr);
	void setupModel(std::shared_ptr<RailInterval> railint,
		const readruler::IntervalReport& itrep, bool useAverage);
};

class ReadRulerSummaryModel : public QStandardItemModel
{
public:
	enum {
		ColType = 0,
		ColValue,
		ColCount,
		ColUsed,
		ColMAX
	};

	ReadRulerSummaryModel(QObject* parent = nullptr);
	void setupModel(const readruler::IntervalReport& itrep);
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
	bool useAverage;
    
	ReadRulerDetailModel* mdDetail;
	ReadRulerSummaryModel* mdSummary;
	QTableView* tbDetail, * tbSummary;
	DialogAdapter* dlgDetail, * dlgSummary;

public:
    ReadRulerPagePreview(QWidget* parent=nullptr);
	auto* getModel() { return model; }
	void setData(ReadRulerReport&& report, 
		const QVector<std::shared_ptr<RailInterval>>& intervals,
		bool useAverage);
	auto& getData() { return data; }
private:
    void initUI();

private slots:
	void actShowDetail();
	void actShowSummary();
	void onDoubleClicked(const QModelIndex& idx);
};


