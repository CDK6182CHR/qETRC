#pragma once
#include <QWidget>
#include <QUndoCommand>
#include <memory>
#include <deque>
#include <QPointer>


class QUndoStack;
class RailCategory;
class QMenu;
class Railway;
class Forbid;
class QTreeView;
class RailDBModel;
class RailDB;
class Ruler;

class QLineEdit;
namespace navi {
class AbstractComponentItem;
}

class RulerWidget;

/**
 * @brief The RailDBNavi class
 * 线路数据库的导航部分。将主要的操作逻辑尽量封装在这里。
 */
class RailDBNavi : public QWidget
{
    Q_OBJECT
    using ACI=navi::AbstractComponentItem;

    std::shared_ptr<RailDB> _raildb;
    RailDBModel* const model;

    QTreeView* tree;
    QMenu* meRail,*meCat;
    QLineEdit* edSearch;

    QUndoStack* _undo;
    bool _changed=false;

    QList<QPointer<RulerWidget>> rulerWidgets;
public:
    explicit RailDBNavi(std::shared_ptr<RailDB> raildb, QWidget *parent = nullptr);
    auto* getTree() { return tree; }
    auto* getModel() { return model;}
    auto* undoStack() { return _undo; }

    /**
     * 退出线路数据库模式的操作。如果改变，询问是否保存；然后清空数据。
     * 返回：是否继续操作 （返回false表示用户点了取消，中断操作）
     */
    bool deactivate();

    /**
     * 关闭程序时的提示：少一个问题。
     */
    bool deactiveOnClose();

private:
    void initUI();
    void initContext();

    /**
     * 当前选中的线路。如果当前不是线路，返回空
     */
    std::shared_ptr<Railway> currentRailway();
    std::shared_ptr<RailCategory> currentCategory();
    ACI* currentItem();
    ACI* currentItemOrRoot();
    
    void clearDBUnchecked();
    void afterResetDB();
    bool saveQuestion();

signals:
    void focusInRailway(std::shared_ptr<Railway>, const std::deque<int>& idx);
    void exportRailwayToDiagram(std::shared_ptr<Railway>);

    /**
     * @brief changedFlagChanged
     * _changed记号变更的通知。
     */
    void changedFlagChanged(bool);
    void dbReset();

    /**
     * 退出DB模式；提示Main关闭相关页面
     */
    void deactivated();

    void importFromCurrent(const std::deque<int>& path, int path_count);

private slots:
    void showContextMenu(const QPoint& pos);
    
    void onCurrentChanged(const QModelIndex& cur, const QModelIndex& prev);
    void markChanged();
    void markUnchanged();

    void actExpand();
    void actCollapse();

    /**
     * 天窗编辑。直接更新，无需后续操作。操作压栈
     */
    void actChangeForbid(std::shared_ptr<Forbid> forbid, std::shared_ptr<Railway> data);

    void actChangeRulerName(std::shared_ptr<Ruler> ruler, const QString& name);
    void actUpdateRulerData(std::shared_ptr<Ruler> ruler, std::shared_ptr<Railway> data);
    void actRemoveRuler(std::shared_ptr<Ruler> ruler);

    void searchFullName();
    void searchPartName();
    void searchRailName();
    void searchResult(const std::deque<std::deque<int>>& paths);

public slots:
    void actEditRail();
    void refreshData();
    void actOpen();
    void actNewDB();
    void actSave();
    void actSaveAs();
    bool openDB(const QString& filename);
    void actSetAsDefaultFile();

    void actNewRail();
    void actNewSubcat();
    void actNewParallelCat();
    void actRemoveCategory();
    void actMoveCategory();

    /**
     * 在item所示的分类下新增分类。
     * 保证入参有效且为分类类型。操作压栈
     */
    void insertSubcatOf(RailDBNavi::ACI* it);

    void actRemoveRail();
    void actRuler();
    void actForbid();
    void actMoveRailway();
    void actExportToDiagram();
    void actExportRailToFile();
    void actExportCategoryToDiagramFile();
    void actExportCategoryToLib();

    /**
     * 从运行图文件导入线路：将运行图文件中读取到的所有线路，加入到当前分类下。
     * 如果名称冲突，自动重命名
     */
    void actImportFromDiagram();

    /**
     * 指定pyetlib文档，导入为新的分类
     */
    void actImportFromLib();

    void actRenameCategory();

    /**
     * 2022.02.07  从当前运行图导入
     * 操作主要交给RailDBContext实施
     */
    void actImportFromCurrent();

    /**
     * 从当前运行图导入线路
     */
    //void actImportFromCurrent();

    void openRulerWidget(std::shared_ptr<Ruler> ruler);
    void closeRulerWidget(std::shared_ptr<Ruler> ruler);
};


namespace qecmd {
    class RemoveRailDB :public QUndoCommand 
    {
        std::shared_ptr<Railway> railway;
        std::deque<int> path;
        RailDBModel* const model;
    public:
        RemoveRailDB(std::shared_ptr<Railway> railway_, const std::deque<int>& path_,
            RailDBModel* model_, QUndoCommand* parent = nullptr);
        virtual void undo()override;
        virtual void redo()override;
    };

    class InsertRailDB :public QUndoCommand
    {
        std::shared_ptr<Railway> railway;
        std::deque<int> path;
        RailDBModel* const model;
    public :
        InsertRailDB(std::shared_ptr<Railway> railway_, const std::deque<int>& path_,
            RailDBModel* model_, QUndoCommand* parent = nullptr);
        void undo()override;
        void redo()override;
    };

    /**
     * 2026.02.16: Move railway to a given category.
     * Parameters: the railway to move, the old and new path for the railway.
     * Currently, this is simply implemented by calling the Remove and Insert commands.
     */
    class MoveRailDB : public QUndoCommand
    {
    public:
        MoveRailDB(std::shared_ptr<Railway> railway, const std::deque<int>& oldPath,
			const std::deque<int>& newPath, RailDBModel* model, QUndoCommand* parent = nullptr);
    };

    class InsertCategory :public QUndoCommand
    {
        std::shared_ptr<RailCategory> cat;
        std::deque<int> path;    // 新增cat的path
        RailDBModel* const model;
    public:
        InsertCategory(std::shared_ptr<RailCategory> cat, const std::deque<int>& path,
            RailDBModel* model, QUndoCommand* parent = nullptr);
        void undo()override;
        void redo()override;
    };

    class RemoveCategory :public QUndoCommand {
        std::shared_ptr<RailCategory> cat;
        std::deque<int> path;
        RailDBModel* const model;
    public:
        RemoveCategory(std::shared_ptr<RailCategory> cat, const std::deque<int>& path,
            RailDBModel* model, QUndoCommand* parent = nullptr);
        void undo()override;
        void redo()override;
    };

    /**
     * 2026.02.16: Move category to another one. The moving operation is guaranteed to be valid.
     * Currently, just by calling Remove and Insert commands.
     */
    class MoveCategory : public QUndoCommand {
    public:
        MoveCategory(std::shared_ptr<RailCategory> cat, const std::deque<int>& oldPath,
            const std::deque<int>& newPath, RailDBModel* model, QUndoCommand* parent = nullptr);
    };

    class ImportRailsDB : public QUndoCommand
    {
        QList<std::shared_ptr<Railway>> rails;
        std::deque<int> path;   // 第一位的path
        RailDBModel* const model;
    public:
        ImportRailsDB(const QList<std::shared_ptr<Railway>>& rails,
            const std::deque<int>& path, RailDBModel* model, QUndoCommand* parent = nullptr);
        void undo()override;
        void redo()override;
    };

    class UpdateForbidDB :public QUndoCommand {
        std::shared_ptr<Forbid> forbid;
        std::shared_ptr<Railway> data;
    public:
        UpdateForbidDB(std::shared_ptr<Forbid> forbid, std::shared_ptr<Railway> data,
            QUndoCommand* parent = nullptr);
        void undo()override;
        void redo()override;
    };

    /**
     * 添加新的标尺 （直接添加在末尾），然后打开编辑窗口。
     */
    class AddNewRulerDB : public QUndoCommand {
        std::shared_ptr<Railway> railway;
        QString name;
        std::shared_ptr<Ruler> theRuler{};
        std::shared_ptr<Railway> theData{};
        RailDBNavi* const navi;   // 反向引用用于撤销时关闭页面
    public:
        AddNewRulerDB(std::shared_ptr<Railway> railway, const QString& name,
            RailDBNavi* navi, QUndoCommand* parent = nullptr);
        void undo()override;
        void redo()override;
    };

    class ChangeRulerNameDB :public QUndoCommand {
        std::shared_ptr<Ruler> ruler;
        QString name;
    public:
        ChangeRulerNameDB(std::shared_ptr<Ruler> ruler, const QString& name,
            QUndoCommand* parent = nullptr);
        void undo()override;
        void redo()override;
    };

    class UpdateRulerDataDB :public QUndoCommand {
        std::shared_ptr<Ruler> ruler;
        std::shared_ptr<Railway> data;
    public:
        UpdateRulerDataDB(std::shared_ptr<Ruler> ruler, std::shared_ptr<Railway> data,
            QUndoCommand* parent = nullptr);
        void undo()override;
        void redo()override;
    };

    /**
     * 删除标尺：处理数据变更，然后要求关闭窗口。
     * ordinate 形式上设置数据。但其实没多大用（db里面的ordinate基本没用）
     */
    class RemoveRulerDB :public QUndoCommand {
        std::shared_ptr<Ruler> ruler;
        std::shared_ptr<Railway> data;
        bool isOrdinate;
        RailDBNavi* navi;
    public:
        RemoveRulerDB(std::shared_ptr<Ruler> ruler, std::shared_ptr<Railway> data,
            bool isOrd, RailDBNavi* navi, QUndoCommand* parent = nullptr);
        void undo()override;
        void redo()override;
    };

    class UpdateCategoryNameDB : public QUndoCommand{
        std::shared_ptr<RailCategory> cat;
        QString name;
        std::deque<int> path;
        RailDBModel* const model;
    public:
        UpdateCategoryNameDB(std::shared_ptr<RailCategory> cat,
            const QString& name, const std::deque<int>& path, RailDBModel* model,
            QUndoCommand* parent = nullptr);
        void undo()override;
        void redo()override;
    };
}
