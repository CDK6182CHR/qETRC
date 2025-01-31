#include "splittraindialog.h"

#include <set>

#include <QVBoxLayout>
#include <QLabel>
#include <QTableView>
#include <QHeaderView>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QCheckBox>

#include "data/common/qesystem.h"
#include "data/train/train.h"
#include "data/train/traincollection.h"
#include "util/utilfunc.h"

SplitTrainModel::SplitTrainModel(std::shared_ptr<Train> train, QObject *parent):
    QStandardItemModel(parent), _train(train)
{
    setHorizontalHeaderLabels({
        tr("站名"), tr("到点"), tr("开点"), tr("营业"), tr("停时"),
        tr("新车次名")
    });
    setupModel();
    connect(this, &SplitTrainModel::dataChanged,
        [this](const QModelIndex& topLeft, const QModelIndex& bottomRight, const QList<int>& roles) {
            if (roles.contains(Qt::EditRole)) {
                if (topLeft.column() <= ColNewTrainName && ColNewTrainName <= bottomRight.column()) {
                    for (int r = topLeft.row(); r <= bottomRight.row(); r++) {
                        auto* it = item(r, ColNewTrainName);
                        if (!it->text().isEmpty()) {
                            it->setCheckState(Qt::Checked);
                        }
                        else {
                            it->setCheckState(Qt::Unchecked);
                        }
                    }
                }
            }
        });
}

QVector<std::shared_ptr<Train>> SplitTrainModel::acceptedData(TrainCollection& coll, QWidget* report_parent, bool cross_terminal)
{
    QVector<std::shared_ptr<Train>> res{};

    QString warn_msg;

    int start_row = 0;
    TrainName last_train_name = _train->trainName();
    std::set<QString> full_name_set;

    for (int row = 0; row < rowCount(); row++) {
        auto* it = item(row, ColNewTrainName);
        if (it->checkState() == Qt::Checked) {
            const QString& new_name = it->text();
            if (new_name.isEmpty()) {
                QMessageBox::warning(report_parent, tr("错误"),
                    tr("第%1行：新车次不能为空").arg(row+1));
                return {};
            }

            // Check name INTERNAL name collision
            if (full_name_set.contains(new_name)) {
                QMessageBox::warning(report_parent, tr("错误"),
                    tr("第%1行：新车次与其他新车次冲突").arg(row + 1));
                return {};
            }
            full_name_set.emplace(new_name);

            if (row - start_row >= 0) {
                // Generate a new train here!
                auto nt = createTrain(coll, report_parent, last_train_name, start_row, row, 
                    start_row > 0 && cross_terminal, cross_terminal);
                if (!nt)
                    return {};
                res.emplace_back(std::move(nt));
                start_row = row;
                last_train_name = TrainName(it->text());
            }
        }
        else {
            // new-train not selected
            if (!it->text().isEmpty()) {
                warn_msg.append(tr("第%1行：设置了新车次%2，但没有勾选复选框。所设置的新车次无效。\n").arg(it->text()).arg(row + 1));
            }
        }
    }

    // Process the last train part
    auto nt = createTrain(coll, report_parent, last_train_name, start_row, rowCount() - 1, start_row>0 && cross_terminal, false);
    if (!nt)
        return {};
    res.emplace_back(std::move(nt));

    if (!warn_msg.isEmpty()) {
        auto flag = QMessageBox::question(report_parent, tr("提示"), tr("新列车生成时有下列信息：\n%1\n是否确认继续？"));
        if (flag != QMessageBox::Yes)
            return {};
    }
    return res;
}

void SplitTrainModel::setupModel()
{
    refreshData();
}

void SplitTrainModel::refreshData()
{
    using SI=QStandardItem;
    if (!_train){
        setRowCount(0);
        return;
    }

    setRowCount(_train->stationCount());

    int n=0;
    for (auto itr=_train->timetable().begin(); itr!=_train->timetable().end();++itr,++n){
        auto* it=new SI(itr->name.toSingleLiteral());
        it->setEditable(false);
        setItem(n, ColName, it);

        it=new SI(itr->arrive.toString("hh:mm:ss"));
        it->setEditable(false);
        setItem(n, ColArrive, it);

        it=new SI(itr->arrive.toString("hh:mm:ss"));
        it->setEditable(false);
        setItem(n, ColDepart, it);

        it=new SI;
        it->setCheckState(qeutil::boolToCheckState(itr->business));
        it->setCheckable(false);
        setItem(n, ColBusiness, it);

        it=new SI(itr->stopString());
        it->setEditable(false);
        setItem(n, ColStopTime, it);

        it=new SI;
        it->setCheckState(Qt::Unchecked);
        it->setCheckable(true);
        setItem(n, ColNewTrainName, it);
    }
}

std::shared_ptr<Train> SplitTrainModel::createTrain(TrainCollection& coll, QWidget* report_parent, const TrainName& name, 
    int start_row, int end_row, bool first_starting, bool last_terminal)
{
    if (!coll.trainNameIsValid(name, _train)) {
        QMessageBox::warning(report_parent, tr("错误"),
            tr("第%1行：新车次与既有车次重复").arg(start_row + 1));
        return {};
    }

    // Now create new train object
    auto nt = std::make_shared<Train>(name, _train->starting(), _train->terminal(), _train->passenger());
    nt->setType(_train->type());

    auto itr_start = _train->timetable().begin(); std::advance(itr_start, start_row);
    auto itr_end = _train->timetable().begin(); std::advance(itr_end, end_row + 1);
    nt->timetable() = std::decay_t<decltype(nt->timetable())>(itr_start, itr_end);   // Copy

    // Requires timetable to be non-empty
    if (first_starting) {
        nt->timetable().front().arrive = nt->timetable().front().depart;
        nt->setStarting(nt->timetable().front().name);
    }
    if (last_terminal) {
        nt->timetable().back().depart = nt->timetable().back().arrive;
        nt->setTerminal(nt->timetable().back().name);
    }

    return nt;
}

SplitTrainDialog::SplitTrainDialog(TrainCollection& coll, std::shared_ptr<Train> train, QWidget* parent):
    QDialog(parent),
    m_coll(coll), m_train(train),
    m_model(new SplitTrainModel(train, this))
{
    setWindowTitle(tr("拆分车次 - %1").arg(train->trainName().full()));
    initUI();
}

void SplitTrainDialog::accept()
{
    auto data = m_model->acceptedData(m_coll, this, m_ckCrossTerminal->isChecked());
    if (data.empty()) {
        // Invalid data; error message should have been produced
        return;
    }

    emit splitApplied(m_train, std::move(data));
    QDialog::accept();
}

void SplitTrainDialog::reject()
{
    auto flag = QMessageBox::question(this, tr("拆分车次"),
        tr("如果退出，则已配置的数据将丢失，是否确认退出？"));
    if (flag == QMessageBox::Yes) {
        QDialog::reject();
    }
}

void SplitTrainDialog::initUI()
{
    resize(800, 800);
    auto* vlay = new QVBoxLayout(this);
    auto* lab = new QLabel(tr("请在下表配置要拆分的列车。在每一段拆分车次的起始车站，勾选“新车次名”列的复选框，然后填写"
        "新列车的全车次名。若第一行被勾选，则当前车次名将被修改为所配置的新车次名。"));
    lab->setWordWrap(true);
    vlay->addWidget(lab);
    m_ckCrossTerminal = new QCheckBox(tr("拆分交叉站设置为始发终到站"));
    m_ckCrossTerminal->setChecked(true);
    vlay->addWidget(m_ckCrossTerminal);

    m_table = new QTableView;
    m_table->verticalHeader()->setDefaultSectionSize(SystemJson::instance.table_row_height);
    m_table->setModel(m_model);
    m_table->setEditTriggers(QTableView::AllEditTriggers);

    {
        int c = 0;
        for (int w : {120, 80, 80, 40, 60, 120}) {
            m_table->setColumnWidth(c++, w);
        }
    }

    vlay->addWidget(m_table);

    auto* box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal);
    vlay->addWidget(box);
    connect(box, &QDialogButtonBox::accepted, this, &SplitTrainDialog::accept);
    connect(box, &QDialogButtonBox::rejected, this, &SplitTrainDialog::reject);
}
