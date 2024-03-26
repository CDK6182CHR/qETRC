/**
 * 2024.03.26  
 * This file provides macro definitions for the icons used in this program.
 * The file should only be included in related cpp files.
 * Here, QE stands for qETRC, ICN stands for icon. 
 */

#pragma once

#include <QApplication>
#include <QStyle>
#include <QIcon>

/**
 * For the icons in QStyle::standardIcon. 
 * The argument _name starts from SP_
 */
#define QE_STD_ICON(_name) qApp->style()->standardIcon(QStyle::SP_##_name)

#define QE_RSC_ICON_PREFIX ":/icons/"

/**
 * The icon using the files provided in rsc.
 */
#define QE_RSC_ICON(_path) QIcon(QE_RSC_ICON_PREFIX #_path)


/////////////////////////////// Begin: icon specifications ////////////////////////////

///////// 工具栏顶部工具条 /////////

#define QEICN_undo QE_RSC_ICON(undo.png)   // 撤销
#define QEICN_redo QE_RSC_ICON(redo.png)   // 重做
#define QEICN_customize QE_RSC_ICON(customize.svg)  // 自定义toolbar  (not used yet)


///////// 工具栏 开始 (1) /////////

#define QEICN_new_file QE_STD_ICON(FileIcon)            // 新建
#define QEICN_open_file QE_STD_ICON(DialogOpenButton)   // 打开
#define QEICN_save_file QE_STD_ICON(DialogSaveButton)   // 保存
#define QEICN_save_file_as QE_STD_ICON(DialogSaveAllButton)   // 另存为
#define QEICN_export_single_rail QE_RSC_ICON(ETRC-dynamic.png)  // 导出为单线路运行图

#define QEICN_navi QE_RSC_ICON(Graph-add.png)   // 运行图导航
#define QEICN_train_list QE_STD_ICON(FileDialogListView)   // 列车管理
#define QEICN_diagram_widget QE_RSC_ICON(diagram.png)      // 运行图窗口
#define QEICN_history QE_RSC_ICON(clock.png)   // 历史记录
#define QEICN_text_out_widget QE_RSC_ICON(text.png)  // 文本输出窗口
#define QEICN_issue_widget QE_STD_ICON(MessageBoxCritical)  // 问题窗口
