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
#define QEICN_close_current QE_STD_ICON(DialogCloseButton)   // 关闭当前面板
#define QEICN_global_config QE_RSC_ICON(config.png)  // 全局配置选项下拉菜单
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
#define QEICN_add_diagram_page QE_RSC_ICON(add.png)   // 添加运行图页面

#define QEICN_global_refresh QE_STD_ICON(BrowserReload)   // 刷新
#define QEICN_rename_station QE_RSC_ICON(brush.png)   // 更改站名

#define QEICN_help QE_STD_ICON(DialogHelpButton)   // 帮助
#define QEICN_about QE_STD_ICON(MessageBoxInformation)  // 关于
#define QEICN_about_qt QE_STD_ICON(TitleBarMenuButton)  // 关于Qt
#define QEICN_exit_app QE_STD_ICON(BrowserStop)   // 退出程序

///////// 工具栏 线路 (2) /////////

#define QEICN_import_rails QE_RSC_ICON(add.png)   // 导入线路
#define QEICN_rail_edit QE_RSC_ICON(rail.png)     // 基线编辑
#define QEICN_train_path QE_RSC_ICON(polyline.png)   // 列车径路 管理面板
#define QEICN_ruler_edit QE_RSC_ICON(ruler.phg)   // 标尺编辑
#define QEICN_skylight_edit QE_RSC_ICON(forbid.png)  // 天窗编辑
#define QEICN_new_rail QE_RSC_ICON(new-file.png)   // 新建线路

#define QEICN_read_ruler_wizard QE_RSC_ICON(ruler_pen.png)   // 标尺综合；多车次标尺读取
#define QEICN_locate_to_diagram QE_STD_ICON(FileDialogContentsView)   // 定位到运行图

#define QEICN_rail_db QE_RSC_ICON(database.png)  // 线路数据库
#define QEICN_fast_path QE_RSC_ICON(diagram.png)  // 快速径路生成
#define QEICN_route_sel QE_RSC_ICON(polyline.png)  // 经由选择

///////// 工具栏 列车 (3) /////////

#define QEICN_train_batch_op QE_RSC_ICON(copy.png)  // 列车批量操作
#define QEICN_import_trains QE_RSC_ICON(add_train.png)  // 导入车次
#define QEICN_search_train QE_STD_ICON(FileDialogContentsView)   // 搜索车次
#define QEICN_new_train QE_RSC_ICON(add.png)   // 新建车次

#define QEICN_timetable_quick QE_RSC_ICON(clock.png)   // 速览时刻窗口
#define QEICN_train_info_quick QE_RSC_ICON(info.png)   // 速览信息窗口
#define QEICN_edit_filters QE_RSC_ICON(filter.png)     // 编辑预设的列车筛选器

#define QEICN_apply_pass_stations QE_STD_ICON(DialogApplyButton)   // 应用最大跨越站数
#define QEICN_weaken_unselect QE_RSC_ICON(weaken.png)   // 选择运行线时，背景虚化

#define QEICN_routing_edit QE_RSC_ICON(polyline.png)   // 交路编辑页面
#define QEICN_routing_batch_parse QE_RSC_ICON(text.png)  // 批量解析交路
#define QEICN_routing_batch_identify QE_RSC_ICON(identify.png)  // 批量识别交路中的车次

#define QEICN_compare_trains QE_RSC_ICON(compare.png)   // 车次对照
#define QEICN_timetable_diagon QE_RSC_ICON(identify.png)  // 时刻诊断
#define QEICN_compare_diagram QE_RSC_ICON(compare.png)  // 运行图对比
#define QEICN_section_count QE_RSC_ICON(counter.png)   // 区间对数表
#define QEICN_section_trains QE_RSC_ICON(train.png)   // 区间车次表
#define QEICN_interval_stat QE_RSC_ICON(data.png)   // 列车区间运行统计

#define QEICN_ruler_paint QE_RSC_ICON(ruler_pen.png)   // 标尺排图
#define QEICN_batch_copy_trains QE_RSC_ICON(copy.png)   // 批量复制车次
#define QEICN_timetable_interp QE_RSC_ICON(add.png)    // 时刻插值
#define QEICN_greedy_paint QE_RSC_ICON(ruler_pen.png)   // 贪心推线

///////// 工具栏 显示 (4) /////////

#define QEICN_line_level_hide QE_RSC_ICON(diagram.png)   // 运行线级别显示控制
#define QEICN_refresh_type_list QE_STD_ICON(BrowserReload)   // 刷新类型表
#define QEICN_apply_type_show QE_STD_ICON(DialogApplyButton)   // 应用显示类型设置
#define QEICN_advanced_show_filter QE_RSC_ICON(filter.png)   // 高级显示类型设置

#define QEICN_appearance_config_diagram QE_RSC_ICON(config.png)   // 显示设置（运行图文件级别）
#define QEICN_type_manager_diagram QE_RSC_ICON(settings.png)   // 类型管理
#define QEICN_type_regex QE_RSC_ICON(filter.png)   // 类型规则（正则表达式）


///////// 工具栏 运行图 (5) /////////

#define QEICN_edit_page QE_RSC_ICON(edit.png)  // 编辑运行图页面
#define QEICN_export_page QE_RSC_ICON(pdf.png)   // 导出运行图页面
#define QEICN_appearance_config_page QE_RSC_ICON(config.png)   // 显示设置 （运行图页面级别）
#define QEICN_switch_to_page QE_STD_ICON(FileDialogContentsView) // 转到运行图页面
#define QEICN_edit_rail_page QE_RSC_ICON(rail.png)   // 编辑线路（运行图页面）

#define QEICN_h_expand QE_RSC_ICON(h_expand.png)   // 水平放大
#define QEICN_h_shrink QE_RSC_ICON(h_shrink.png)   // 水平缩小
#define QEICN_v_expand QE_RSC_ICON(v_expand.png)   // 垂直放大
#define QEICN_v_shrink QE_RSC_ICON(v_shrink.png)   // 垂直缩小

#define QEICN_page_copy QE_RSC_ICON(copy.png)   // 运行图页面副本
#define QEICN_del_page QE_STD_ICON(TrashIcon)   // 删除运行图页面
#define QEICN_close_page_context QE_STD_ICON(DialogCloseButton)   // 关闭运行图页面菜单

///////// 工具栏 列车审阅 (6) /////////

#define QEICN_train_line QE_RSC_ICON(trainline.png)   // 高亮列车运行线
#define QEICN_train_events QE_RSC_ICON(clock.png)    // 列车事件表
#define QEICN_train_line_list QE_RSC_ICON(line-manage.png)   // 运行线一览
#define QEICN_ruler_diff QE_RSC_ICON(ruler.png)   // 标尺对照
#define QEICN_timetable_diagno_train QE_RSC_ICON(identify.png)   // 时刻诊断（列车）

///////// 工具栏 列车编辑 (7) /////////

#define QEICN_apply_train_info QE_STD_ICON(DialogApplyButton)   // 应用 列车基本信息
#define QEICN_reset_train_info QE_STD_ICON(DialogCancelButton)  // 还原 列车基本信息
#define QEICN_add_path_to_train QE_RSC_ICON(add.png)   // 添加径路到列车
#define QEICN_remove_path_from_train QE_STD_ICON(TrashIcon)   // 从列车移除径路

#define QEICN_interval_exchange QE_RSC_ICON(exchange.png)   // 区间换线
#define QEICN_timetable_adjust QE_RSC_ICON(adjust.png)   // 时刻平移
#define QEICN_timetable_correction QE_RSC_ICON(settings.png)  // 时刻修正
#define QEICN_timetable_simple_interp QE_RSC_ICON(add.png)   // 快速推定通过站时刻

#define QEICN_edit_timetable QE_RSC_ICON(timetable.png)   // 编辑时刻表
#define QEICN_edit_train QE_RSC_ICON(edit.png)   // 编辑列车所有数据

#define QEICN_copy_train QE_RSC_ICON(copy.png)   // 创建列车副本
#define QEICN_del_train QE_STD_ICON(TrashIcon)   // 删除列车
#define QEICN_close_train_context QE_STD_ICON(DialogCloseButton)   // 关闭列车面板

///////// 工具栏 线路工具 (8) /////////

#define QEICN_edit_rail QE_RSC_ICON(rail.png)   // 基线编辑
#define QEICN_edit_skylight QE_RSC_ICON(forbid.png)   // 天窗编辑
#define QEICN_edit_ruler QE_RSC_ICON(ruler.png)   // 标尺编辑
#define QEICN_add_ruler QE_RSC_ICON(add.png)   // 新建标尺

#define QEICN_reverse_rail QE_RSC_ICON(exchange1.png)   // 线路反排
#define QEICN_joint_rail QE_RSC_ICON(joint.png)   // 线路拼接
