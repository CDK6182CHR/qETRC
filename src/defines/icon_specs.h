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

#define QEICN_diagram_option QE_RSC_ICON(config.png)      // 运行图选项
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
#define QEICN_ruler_edit QE_RSC_ICON(ruler.png)   // 标尺编辑
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

#define QEICN_change_page QE_RSC_ICON(exchange.png)   // 切换页面
#define QEICN_edit_page QE_RSC_ICON(edit.png)  // 编辑运行图页面
#define QEICN_export_page QE_RSC_ICON(pdf.png)   // 导出运行图页面
#define QEICN_appearance_config_page QE_RSC_ICON(config.png)   // 显示设置 （运行图页面级别）
#define QEICN_auto_top_bottom_margin QE_RSC_ICON(adjust.png)   // 自动调整上下边距
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

#define QEICN_change_train QE_RSC_ICON(exchange.png)   // 切换列车
#define QEICN_show_train_line QE_RSC_ICON(trainline.png)   // 显示列车运行线
#define QEICN_train_line QE_RSC_ICON(line-manage.png)   // 高亮列车运行线
#define QEICN_train_events QE_RSC_ICON(clock.png)    // 列车事件表
#define QEICN_train_line_list QE_RSC_ICON(line-manage.png)   // 运行线一览
#define QEICN_ruler_diff QE_RSC_ICON(ruler.png)   // 标尺对照
#define QEICN_timetable_diagno_train QE_RSC_ICON(identify.png)   // 时刻诊断（列车）

#define QEICN_train_to_routing QE_STD_ICON(ArrowRight)   // 转到交路
#define QEICN_create_routing_train QE_RSC_ICON(new-file.png)   // 从列车创建交路
#define QEICN_add_train_to_routing QE_RSC_ICON(add.png)   // 将列车添加到交路

///////// 工具栏 列车编辑 (7) /////////

#define QEICN_apply_train_info QE_STD_ICON(DialogApplyButton)   // 应用 列车基本信息
#define QEICN_reset_train_info QE_STD_ICON(DialogCancelButton)  // 还原 列车基本信息
#define QEICN_train_to_path QE_STD_ICON(ArrowRight)   // 转到列车径路
#define QEICN_add_path_to_train QE_RSC_ICON(add.png)   // 添加径路到列车
#define QEICN_remove_path_from_train QE_STD_ICON(TrashIcon)   // 从列车移除径路

#define QEICN_interval_exchange QE_RSC_ICON(exchange.png)   // 区间换线
#define QEICN_timetable_adjust QE_RSC_ICON(adjust.png)   // 时刻平移
#define QEICN_timetable_correction QE_RSC_ICON(settings.png)  // 时刻修正
#define QEICN_timetable_simple_interp QE_RSC_ICON(add.png)   // 快速推定通过站时刻
#define QEICN_split_train QE_RSC_ICON(copy.png)   // 拆分车次
#define QEICN_merge_trains QE_RSC_ICON(joint.png)   // 合并车次

#define QEICN_edit_timetable QE_RSC_ICON(timetable.png)   // 编辑时刻表
#define QEICN_edit_train QE_RSC_ICON(edit.png)   // 编辑列车所有数据

#define QEICN_copy_train QE_RSC_ICON(copy.png)   // 创建列车副本
#define QEICN_del_train QE_STD_ICON(TrashIcon)   // 删除列车
#define QEICN_close_train_context QE_STD_ICON(DialogCloseButton)   // 关闭列车面板

///////// 工具栏 线路工具 (8) /////////

#define QEICN_change_rail QE_RSC_ICON(exchange.png)   // 切换线路
#define QEICN_edit_rail QE_RSC_ICON(rail.png)   // 基线编辑
#define QEICN_edit_skylight QE_RSC_ICON(forbid.png)   // 天窗编辑
#define QEICN_edit_ruler_rail QE_RSC_ICON(ruler.png)   // 标尺编辑 从线路进入
#define QEICN_add_ruler QE_RSC_ICON(add.png)   // 新建标尺

#define QEICN_reverse_rail QE_RSC_ICON(exchange1.png)   // 线路反排
#define QEICN_joint_rail QE_RSC_ICON(joint.png)   // 线路拼接
#define QEICN_section_count QE_RSC_ICON(counter.png)   // 断面对数表
#define QEICN_station_trains QE_RSC_ICON(timetable.png)   // 车站车次表
#define QEICN_station_events QE_RSC_ICON(electronic-clock.png)  // 车站事件表
#define QEICN_section_events QE_RSC_ICON(diagram.png)   // 断面事件表
#define QEICN_time_snap QE_RSC_ICON(clock.png)   // 运行快照 同一时刻下的所有列车状态

#define QEICN_station_tracks QE_RSC_ICON(rail.png)   // 股道分析
#define QEICN_rail_topo QE_RSC_ICON(diagram.png)  // 线路拓扑
#define QEICN_headway_list QE_RSC_ICON(h_expand.png)  // 列车间隔分析
#define QEICN_headway_summary QE_RSC_ICON(adjust-2.png)  // 列车间隔汇总

#define QEICN_create_page_from_rail QE_RSC_ICON(diagram.png)   // 快速创建单线路运行图页面
#define QEICN_del_railway QE_STD_ICON(TrashIcon)   // 删除基线
#define QEICN_copy_railway QE_RSC_ICON(copy.png)   // 创建基线副本
#define QEICN_export_railway_csv QE_RSC_ICON(copy.png)  // 导出基线CSV
#define QEICN_close_rail_context QE_STD_ICON(DialogCloseButton)  // 关闭基线面板

///////// 工具栏 标尺管理 (9) /////////

#define QEICN_change_ruler QE_RSC_ICON(exchange.png)   // 切换标尺
#define QEICN_edit_ruler QE_RSC_ICON(edit.png)    // 编辑标尺
#define QEICN_ruler_from_train QE_RSC_ICON(identify.png)   // 从车次提取标尺
#define QEICN_ruler_from_speed QE_RSC_ICON(clock.png)   // 从速度计算标尺

#define QEICN_merge_ruler QE_STD_ICON(ArrowBack)   // 合并标尺
#define QEICN_import_ruler_csv QE_RSC_ICON(ruler.png)   // 导入标尺 (CSV)
#define QEICN_export_ruler_csv QE_RSC_ICON(copy.png)    // 导出标尺 (CSV)
#define QEICN_ordinate_ruler QE_RSC_ICON(ruler.png)   // 设置为排图标尺

#define QEICN_copy_ruler QE_RSC_ICON(copy.png)   // 创建标尺副本
#define QEICN_del_ruler QE_STD_ICON(TrashIcon)   // 删除标尺
#define QEICN_close_ruler_context QE_STD_ICON(DialogCloseButton)   // 关闭标尺面板

///////// 工具栏 交路编辑 (0) /////////

#define QEICN_change_routing QE_RSC_ICON(exchange.png)   // 切换交路
#define QEICN_highlight_routing QE_RSC_ICON(trainline.png)   // 高亮显示
#define QEICN_edit_routing QE_RSC_ICON(edit.png)   // 交路编辑
#define QEICN_split_routing QE_RSC_ICON(copy.png)   // 交路拆分
#define QEICN_merge_routing QE_RSC_ICON(joint.png)   // 交路合并
#define QEICN_del_routing QE_STD_ICON(TrashIcon)   // 删除交路

#define QEICN_routing_diagram QE_RSC_ICON(routing-diagram.png)   // 交路图
#define QEICN_routing_mile QE_RSC_ICON(ruler.png)   // 交路里程表
#define QEICN_parse_routing QE_RSC_ICON(text.png)   // 文本解析
#define QEICN_identify_trains QE_RSC_ICON(identify.png)   // 识别车次
#define QEICN_close_routing_context QE_STD_ICON(DialogCloseButton)   // 关闭交路面板

///////// 工具栏 列车径路 (A) /////////

#define QEICN_change_path QE_RSC_ICON(exchange.png)   // 切换列车径路
#define QEICN_edit_path QE_RSC_ICON(edit.png)   // 编辑 列车径路

#define QEICN_edit_path_ruler QE_RSC_ICON(ruler.png)  // 编辑 列车径路标尺
#define QEICN_add_path_ruler QE_RSC_ICON(add.png)     // 新建 列车径路标尺

#define QEICN_path_trains QE_RSC_ICON(train.png)   // 列车表
#define QEICN_path_add_trains QE_RSC_ICON(add.png)   // 添加列车到径路
#define QEICN_path_clear_trains QE_STD_ICON(TabCloseButton)   // 清空列车

#define QEICN_del_path QE_STD_ICON(TrashIcon)   // 删除径路
#define QEICN_copy_path QE_RSC_ICON(copy.png)   // 创建径路副本
#define QEICN_close_path_context QE_STD_ICON(DialogCloseButton)  // 关闭径路面板

///////// 工具栏 路网管理 /////////

#define QEICN_rail_db_toggle QE_RSC_ICON(database.png)   // 线路数据库面板开关
#define QEICN_quick_path QE_RSC_ICON(diagram.png)   // 快速径路切片
#define QEICN_path_selector QE_RSC_ICON(polyline.png)   // 经由选择
#define QEICN_adjacent_list QE_STD_ICON(FileDialogContentsView)   // 邻接表
#define QEICN_raildb_include_diagram QE_RSC_ICON(rail.png)   // 线路数据库 有向图模型包含当前运行图线路
#define QEICN_refresh_net QE_STD_ICON(BrowserReload)  // 刷新线网

#define QEICN_export_rail_to_diagram QE_RSC_ICON(diagram.png)   // 导出线路到运行图
#define QEICN_export_rail_to_file QE_STD_ICON(FileIcon)   // 导出线路到文件
#define QEICN_del_rail_db QE_STD_ICON(TrashIcon)   // 删除线路 （数据库）

#define QEICN_ruler_db QE_RSC_ICON(ruler.png)   // 编辑标尺
#define QEICN_skylight_db QE_RSC_ICON(forbid.png)  // 编辑天窗
#define QEICN_close_db_context QE_STD_ICON(DialogCloseButton)  // 关闭线路数据库

////////////// 非工具栏图标 //////////////

#define QEICN_diagram_page_title QE_RSC_ICON(diagram.png)   // 运行图页面标题栏图标
#define QEICN_train_info_to_routing QE_STD_ICON(ArrowRight)   // 速览信息中 转到交路
#define QEICN_train_filter_info QE_STD_ICON(MessageBoxInformation)   // 列车筛选器中 关于预设筛选器的信息
#define QEICN_train_filter_info_widget QE_STD_ICON(MessageBoxInformation)  // 预设筛选器管理中 关于预设筛选器的信息

#define QEICN_config_transparent_help QE_STD_ICON(MessageBoxQuestion)   // 显示设置中 关于透明模式的信息
#define QEICN_config_link_mode_setup_info QE_STD_ICON(MessageBoxInformation)   // 显示设置中 连线模式的信息和自动设置
#define QEICN_config_inverse_color_info QE_STD_ICON(MessageBoxInformation)   // 显示设置中 反色的信息

#define QEICN_type_config_transparent_help QE_STD_ICON(MessageBoxQuestion)   // 类型管理中 关于透明模式的信息
#define QEICN_type_regex_transparent_help QE_STD_ICON(MessageBoxQuestion)   // 类型规则中 关于透明模式的信息

#define QEICN_paint_station_info_close QE_STD_ICON(DialogCloseButton)   // 铺画列车信息窗口的关闭按钮
#define QEICN_undo_stack_clean QE_STD_ICON(DialogSaveButton)   // 历史记录面板中 标记clean位置的图标

#define QEICN_ruler_combo_refresh QE_STD_ICON(BrowserReload)   // RulerCombo中的刷新

#define QEICN_train_editor_change QE_RSC_ICON(exchange.png)   // 列车编辑页面 切换车次
#define QEICN_train_editor_sync QE_RSC_ICON(exchange1.png)    // 列车编辑页面 保持与选择同步

#define QEICN_rail_start_mile_info QE_STD_ICON(MessageBoxInformation)  // 线路编辑页面 起始里程信息窗口

#define QEICN_routing_editor_change QE_RSC_ICON(exchange.png)   // 交路编辑页面 切换车次
#define QEICN_routing_editor_sync QE_RSC_ICON(exchange1.png)    // 交路编辑页面 保持与选择同步

#define QEICN_system_dialog_drag_info QE_STD_ICON(MessageBoxInformation)   // 全局配置选项中 关于拖动时刻的信息
