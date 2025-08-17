#include "mergeroutingdialog.h"

#include "data/train/routing.h"
#include "model/train/routingeditmodel.h"
#include "data/common/qesystem.h"
#include "dialogs/selectroutingdialog.h"

#include <QCheckBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTableView>
#include <QVBoxLayout>
#include <QTableView>
#include <QHeaderView>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QMessageBox>

MergeRoutingDialog::MergeRoutingDialog(TrainCollection& coll, std::shared_ptr<Routing> routing, QWidget *parent):
    QDialog(parent), m_coll(coll), m_routing(routing),
    m_model(new RoutingEditModel(m_routing, this))
{
    setWindowTitle(tr("合并交路 - %1").arg(routing->name()));
    setAttribute(Qt::WA_DeleteOnClose, true);
    initUI();
}

void MergeRoutingDialog::refreshData()
{
    m_edName->setText(m_routing->name());
    m_edOther->setText(m_other->name());
    m_model->setRouting(m_routing);
}

void MergeRoutingDialog::accept()
{
    if (!m_other) {
        QMessageBox::warning(this, tr("错误"), tr("没有选择要合并的交路"));
        return;
    }
    else if (m_other == m_routing) {
        QMessageBox::warning(this, tr("错误"), tr("所选交路和当前交路重合"));
        return;
    }

    int sel_row = 0;
    if (m_ckSelPos->isChecked()) {
        auto idx = m_table->currentIndex();
        if (!idx.isValid()) {
            QMessageBox::warning(this, tr("错误"), tr("未选择合并位置。请选择要合并交路的车次列的位置，或者不勾选“添加到指定位置”"
                "以将新车次列置于末尾"));
            return;
        }
        sel_row = idx.row();
    }
    emit mergeApplied(m_routing, m_other, m_ckSelPos->isChecked(), sel_row);
    QDialog::accept();
}

void MergeRoutingDialog::initUI()
{
    resize(800, 600);
    auto* vlay=new QVBoxLayout(this);

    auto* lab=new QLabel(tr("此功能可将所选的交路合并至当前交路。所选交路中的车次序列将连续的放入"
                              "当前交路的车次序列中、所选车次之前。合并后，所选交路将被删除。"));
    lab->setWordWrap(true);
    vlay->addWidget(lab);

    auto* form=new QFormLayout;

    m_edName=new QLineEdit;
    m_edName->setReadOnly(true);
    form->addRow(tr("当前交路"), m_edName);
    m_edName->setText(m_routing->name());

    auto* hlay = new QHBoxLayout;
    m_edOther=new QLineEdit;
    m_edOther->setReadOnly(true);
    hlay->addWidget(m_edOther);
    auto* btn=new QPushButton(tr("选择"));
    hlay->addWidget(btn);
    form->addRow(tr("合并交路"), hlay);
    vlay->addLayout(form);

    connect(btn, &QPushButton::clicked, [this]() {
        auto ret = SelectRoutingDialog::selectRouting(m_coll, false, this);
        if (!ret.isAccepted || !ret.routing)
            return;
        if (ret.routing == this->m_routing) {
            QMessageBox::warning(this, tr("错误"),
                tr("所选要合并的交路与当前交路相同！"));
            return;
        }
        this->m_other = ret.routing;
        this->m_edOther->setText(m_other->name());
    });

    lab=new QLabel(tr("要合并交路的车次序列，默认添加到原交路序列的最末。如需指定位置，请勾选下面复选框，"
                        "并在下表中选择要添加的位置。将添加到所选行之前。"));
    lab->setWordWrap(true);
    vlay->addWidget(lab);

    m_ckSelPos = new QCheckBox(tr("添加到指定位置（不勾选为添加到最后）"));
    vlay->addWidget(m_ckSelPos);

    m_table=new QTableView;
    m_table->verticalHeader()->setDefaultSectionSize(SystemJson::get().table_row_height);
    m_table->setModel(m_model);
    m_table->setEditTriggers(QTableView::NoEditTriggers);
    m_table->setEnabled(false);
    vlay->addWidget(m_table);

    auto* box=new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    vlay->addWidget(box);
    connect(box, &QDialogButtonBox::accepted, this, &MergeRoutingDialog::accept);
    connect(box, &QDialogButtonBox::rejected, this, &MergeRoutingDialog::close);

    connect(m_ckSelPos, &QCheckBox::toggled, [this](bool checked) {
        m_table->setEnabled(checked);
        });
}
