#pragma once

#include <QObject>
#include <QAction>
#include <QUndoCommand>
#include <QVector>
#include "SARibbonCategory.h"
#include <SARibbonGalleryGroup.h>

#include "data/diagram/diagram.h"

#include "util/buttongroup.hpp"


class MainWindow;
class TypeManager;

/**
 * @brief The ViewCategory class
 * Ribbon中显示页面的代理。作为Main的friend类
 */
class ViewCategory : public QObject
{
    Q_OBJECT;

    SARibbonCategory* cat;
    Diagram& diagram;
    MainWindow* mw;

    QAction* actLineCtrl;

    RadioButtonGroup<2, QVBoxLayout>* rdDirType;

    SARibbonGalleryGroup* group;
    SARibbonGallery* gall;
    
public:
    explicit ViewCategory(MainWindow* mw_,SARibbonCategory* cat_,
                          QObject *parent = nullptr);
    auto* category(){return cat;}

    /**
     * 这是撤销或重做时执行的，真实有效的操作
     */
    void commitTrainsShow(const QList<std::shared_ptr<TrainLine>>& lines, bool show);

    /**
     * 执行单车次的显示状态变化
     * seealso: commitTrainsShow
     * 区别在于不更新TrainList的显示状态。
     */
    void commitSingleTrainShow(const QList<std::shared_ptr<TrainLine>>& lines, bool show);

    /**
     * 执行由显示类型设置变化发起的列车显示情况变化。
     * 每条运行线的显示状态都取反
     */
    void commitTypeShow(const QList<std::shared_ptr<TrainLine>>& lines);

private:
    void initUI();

    /**
     * 这里负责压栈Undo，不执行真正的操作
     */
    void setDirTrainsShow(Direction dir, bool show);

    void setTrainShow(std::shared_ptr<TrainLine> line, bool show);
    //void setTrainShow(std::shared_ptr<TrainAdapter> adp, bool show);

    /*
     * 列车所属类型是否被隐藏。如果被隐藏，则使用显示上行操作时，它不会出现
     */
    bool typeIsShow(std::shared_ptr<Train> train)const;

    

    /**
     * 显示类型变化之后的统一处理，主要是更新TrainList等
     */
    void onTrainShowChanged();

signals:

private slots:
    void showDown();
    void showUp();
    void hideDown();
    void hideUp();
    void lineControlTriggered(bool on);

    void selectPassengers();
    void selectReversed();

    /**
     * 应用类型显示设置。将操作压栈
     */
    void applyTypeShow();

    void actShowConfig();

    /**
     * 点击确认修改配置。压栈操作
     */
    void onActConfigApplied(Config& cfg, const Config& newcfg, bool repaint);

    void actTypeConfig();
    void actTypeConfigDefault();

    void actTypeRegex();
    void actTypeRegexDefault();

    void actSystemJsonDialog();

public slots:
    
    /**
     * 刷新类型表。暂定暴力重来一遍就好
     */
    void refreshTypeGroup();

    /**
     * 实际执行结束，只负责重新铺图
     */
    void commitConfigChange(Config& cfg, bool repaint);

    /**
     * 由TrainListModel调起，设置显示状态，操作压栈
     */
    void actChangeSingleTrainShow(std::shared_ptr<Train> train, bool show);

    /**
     * 仅针对TrainCollection所属的TypeManager的情况...
     * 操作压栈，但还包含前处理，即把所有列车中存在的type对象弄进去
     */
    void actCollTypeSetChanged(TypeManager& manager,
        const QMap<QString, std::shared_ptr<TrainType>>& types,
        const QVector<QPair<std::shared_ptr<TrainType>, std::shared_ptr<TrainType>>>& modified);

    /**
     * 默认配置变化：不需要检查列车情况，直接压栈
     */
    void actDefaultTypeSetChanged(TypeManager& manager,
        const QMap<QString, std::shared_ptr<TrainType>>& types,
        const QVector<QPair<std::shared_ptr<TrainType>, std::shared_ptr<TrainType>>>& modified);

    /**
     * 类型正则表达式变更
     */
    void actCollTypeRegexChanged(TypeManager& manager,
        std::shared_ptr<TypeManager> data);

    void actDefaultTypeRegexChanged(TypeManager& manager,
        std::shared_ptr<TypeManager> data);

    void saveDefaultConfigs();
};



namespace qecmd {

    /**
     * 注意，全都按照TrainLine来处理。Adapter的，分包成Line。
     * 注意只把真正变化了的放进来。
     */
    class ChangeTrainShow :
        public QUndoCommand {
    protected:
        QList<std::shared_ptr<TrainLine>> lines;
        bool show;    // 原操作 （redo操作）是显示还是隐藏
        ViewCategory* const cat;
    public:
        ChangeTrainShow(const QList<std::shared_ptr<TrainLine>>& lines_, bool show_,
            ViewCategory* cat_, QUndoCommand* parent = nullptr);
        virtual void undo()override;
        virtual void redo()override;
    };

    /**
     * 单一列车显示状态的变更，主要由TrainListModel那边调起
     */
    class ChangeSingleTrainShow :public QUndoCommand
    {
        std::shared_ptr<Train> train;
        bool show;
        QList<std::shared_ptr<TrainLine>> lines;
        ViewCategory*const cat;
    public:
        ChangeSingleTrainShow(std::shared_ptr<Train> train_, bool show_,
            const QList<std::shared_ptr<TrainLine>>& lines_, ViewCategory* cat_,
            QUndoCommand* parent = nullptr);
        virtual void undo()override;
        virtual void redo()override;
    };

    /**
     * 通过修改类型设置，来控制列车运行线的显示。
     * 与ChangeTrainShow（通过行别设置）的区别在于，有一个半有效的状态，
     * 即选择的要显示的类型。
     * 注意一个操作可能使得一些列车显示，一些列车隐藏，所以与前面不一样
     * 加入的每一个TrainLine，都是其显示与否要取反的。
     */
    class ChangeTypeShow :
        public QUndoCommand {
        QList<std::shared_ptr<TrainLine>> lines;
        ViewCategory* const cat;
        Config& cfg;  //用来设置NotShowTypes
        QSet<QString> notShowTypes;
    public:
        ChangeTypeShow(const QList<std::shared_ptr<TrainLine>>& lines_,
            ViewCategory* cat_, Config& cfg_, QSet<QString> notShowTypes_,
            QUndoCommand* parent = nullptr);
        virtual void undo()override;
        virtual void redo()override;
    };

    class ChangeTypeSet : public QUndoCommand
    {
        TypeManager& manager;
        QMap<QString, std::shared_ptr<TrainType>> types;
        QVector<QPair<std::shared_ptr<TrainType>, std::shared_ptr<TrainType>>> modified;
        ViewCategory* const cat;
        const bool forDefault;
    public:
        ChangeTypeSet(TypeManager& manager_, 
            const QMap<QString, std::shared_ptr<TrainType>>& types_,
            const QVector<QPair<std::shared_ptr<TrainType>,
            std::shared_ptr<TrainType>>>& modified_,
            ViewCategory* cat_, bool forDefault_,
            QUndoCommand* parent=nullptr):
            QUndoCommand(QObject::tr("更新类型表"),parent),manager(manager_),
            types(types_),modified(modified_),
            cat(cat_),forDefault(forDefault_){}
        virtual void undo()override;
        virtual void redo()override;
    private:
        void commit();
    };

    class ChangeTypeRegex :public QUndoCommand {
        TypeManager& manager;
        std::shared_ptr<TypeManager> data;
        ViewCategory* const cat;
        const bool forDefault;
    public:
        ChangeTypeRegex(TypeManager& manager_, std::shared_ptr<TypeManager> data_,
            ViewCategory* cat_,bool forDefault_, QUndoCommand* parent=nullptr):
            QUndoCommand(QObject::tr("更新类型判定规则"),parent),
            manager(manager_),data(data_),cat(cat_),forDefault(forDefault_){}
        virtual void undo()override;
        virtual void redo()override;
    };
}