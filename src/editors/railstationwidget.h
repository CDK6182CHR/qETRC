#pragma once

#include <QDialog>


class RailCategory;
class QLineEdit;
class Railway;
class RailStationModel;
class QEControlledTable;
struct RailInfoNote;
class QPlainTextEdit;

class RailNoteDialog :
    public QDialog
{
    Q_OBJECT
    std::shared_ptr<Railway> railway;
    const bool commitInPlace;
    QLineEdit* edAuthor, * edVersion;
    QPlainTextEdit* edNote;
public:
    explicit RailNoteDialog(std::shared_ptr<Railway> railway_,
        bool inplace, QWidget* parent = nullptr);
    void refreshData();
private:
    void initUI();
signals:
    void railNoteChanged(std::shared_ptr<Railway> railway, const RailInfoNote& data);
private slots:
    void actApply();
};

/**
 * @brief The RailStationWidget class
 * 线路的里程表编辑页面
 * pyETRC.LineWidget
 * 2021.07.10  将实际修改的权限交给RailContext
 */
class RailStationWidget : public QWidget
{
    Q_OBJECT
    QEControlledTable* ctable;
    RailStationModel* model;
    const bool commitInPlace;
    std::shared_ptr<Railway> railway;
    QLineEdit* edName;
    RailCategory& cat;
    bool _changed = false;
public:
    explicit RailStationWidget(RailCategory& cat_, bool inplace, QWidget *parent = nullptr);

    void setRailway(std::shared_ptr<Railway> rail);

    /**
     * 刷新基本数据（即不包含表格）
     */
    void refreshBasicData();

    void refreshData();

    auto getRailway() { return railway; }

    auto* getModel() { return model; }

    bool isChanged()const { return _changed; }

    /**
     * 应用更改，返回更新是否成功。
     * 是否成功通过changed标签判定。
     */
    bool applyChange();

protected:
    bool event(QEvent* e)override;

private:
    void initUI();
   

signals:

    /**
     * 此信号发送给railcontext来执行压栈
     */
    void railNameChanged(std::shared_ptr<Railway> railway, const QString& name);

    void focusInRailway(std::shared_ptr<Railway> rail);

    /**
     * 线路数据库中可能出现的，railway为空时要求提交
     */
    void invalidApplyRequest();

    void railNoteChanged(std::shared_ptr<Railway> railway, const RailInfoNote& data);

private slots:
    void actApply();
    void actCancel();
    void markChanged();
    void actNote();
    
};



