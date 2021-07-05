#pragma once

#include <QObject>
#include <QAction>
#include <QUndoCommand>

#include "SARibbonCategory.h"
#include <SARibbonGalleryGroup.h>

#include "data/diagram/diagram.h"

#include "util/buttongroup.hpp"


class MainWindow;

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
    void setTrainShow(std::shared_ptr<TrainAdapter> adp, bool show);

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

public slots:
    
    /**
     * 刷新类型表。暂定暴力重来一遍就好
     */
    void refreshTypeGroup();


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
}