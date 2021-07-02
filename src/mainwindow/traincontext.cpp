#include "traincontext.h"



TrainContext::TrainContext(Diagram& diagram_, SARibbonContextCategory* const context_,
	QObject* parent):
	QObject(parent),
	diagram(diagram_),cont(context_)
{
	initUI();
}

void TrainContext::initUI()
{
	//审阅
	if constexpr (true) {
		auto* page = cont->addCategoryPage(tr("审阅"));
		auto* panel = page->addPannel(tr(""));
	}

	//编辑
	if constexpr (true) {
		auto* page = cont->addCategoryPage(tr("编辑"));
		auto* panel = page->addPannel(tr(""));
	}
	
}

void TrainContext::setTrain(std::shared_ptr<Train> train_)
{
	train = train_;
	//todo 相关更新...
}
