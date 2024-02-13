#include "version.h"

namespace qespec {

const std::string_view VERSION = "V1.5.5";
const std::string_view LAST_VERSION = "V1.5.4";
const std::string_view DATE = "20240213";
const int RELEASE_CODE = 48;

const std::string_view DOC_URL = "https://qetrc.readthedocs.io";
const std::string_view DOC_URL_PREFIX = "https://qetrc.readthedocs.io/zh_CN/latest";

const std::string_view UPDATE_LOG = R"(1. 新增在贪心推线功能中，固定指定站停车时间不变。
2. 在标尺排图、贪心推线中，新增批量设置停点功能。
3. 在标尺排图、贪心推线中，新增点击铺画点查看/调整停车时间。
4. 新增标尺数据到特定格式CSV文件的导入/导出。
5. 一些细节和问题修复。)";
}
