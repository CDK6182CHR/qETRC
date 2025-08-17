#include "tagtrainsdialog.h"
#include "data/common/qesystem.h"
#include "data/train/traintag.h"
#include "data/train/train.h"
#include "data/train/traincollection.h"
#include "model/train/trainlistreadmodel.h"
#include "util/buttongroup.hpp"
#include "util/utilfunc.h"

#include <algorithm>
#include <QLabel>
#include <QTableView>
#include <QVBoxLayout>
#include <QHeaderView>

TagTrainsDialog::TagTrainsDialog(const DiagramOptions& ops, TrainCollection& coll, std::shared_ptr<TrainTag> tag, QWidget* parent) :
    QDialog(parent), m_ops(ops), m_coll(coll), m_tag(tag), m_model(new TrainListReadModel(m_ops, this))
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(tr("标签列车表 - %1").arg(tag->name()));
    initUI();
    refreshData();
}

void TagTrainsDialog::refreshData()
{
    auto trains = m_coll.tagTrains(m_tag);
    m_model->resetList(std::move(trains));
}

void TagTrainsDialog::initUI()
{
    resize(600, 600);
    auto* vlay = new QVBoxLayout(this);
    auto* lab = new QLabel(tr("以下为包含标签 [%1] 的列车。").arg(m_tag->name()));
    lab->setWordWrap(true);
    vlay->addWidget(lab);

    m_table = new QTableView;
    m_table->verticalHeader()->setDefaultSectionSize(SystemJson::get().table_row_height);
    m_table->setModel(m_model);
    m_table->setSelectionBehavior(QTableView::SelectRows);
    m_table->setSelectionMode(QTableView::MultiSelection);

    {
        int c = 0;
        for (int w : {100, 100, 100, 80, 40, 80, 80}) {
            m_table->setColumnWidth(c++, w);
        }
    }
    vlay->addWidget(m_table);

    auto* g = new ButtonGroup<3>({ "关闭", "刷新", "删除选中"});
    vlay->addLayout(g);
    g->connectAll(SIGNAL(clicked()), this,
        { SLOT(close()), SLOT(refreshData()), SLOT(actRemove()) });
}

void TagTrainsDialog::actRemove()
{
    const auto& lst = m_table->selectionModel()->selectedRows();
    auto rows = qeutil::indexRows(lst);

    std::vector<std::pair<std::shared_ptr<Train>, int>> remove_data;
    for (int r : rows) {
        auto train = m_model->trains().at(r);
        int idx = train->tagIndex(m_tag);
        if (idx < 0) {
            qWarning() << "TagTrainsDialog::actRemove: INTERNAL INCONSISTENCY: the train does not contain required tag "
                << train->trainName().full() << ", " << m_tag->name();
        }
        else {
            remove_data.emplace_back(train, idx);
        }
    }

    if (remove_data.empty())
        return;

    emit removeTrains(m_tag, remove_data);

    // the dialog is in modal state, so just simply remove the rows are OK
    // the actual operations are done by emitting the signal above
    for (auto itr = rows.rbegin(); itr != rows.rend(); ++itr) {
        m_model->removeTrainAt(*itr);
    }
}

