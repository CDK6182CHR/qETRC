#include "version.h"

namespace qespec {

const std::string_view VERSION = "V1.3.7";
const std::string_view LAST_VERSION = "V1.3.6";
const std::string_view DATE = "20230616";
const int RELEASE_CODE = 41;

const std::string_view DOC_URL = "https://qetrc.readthedocs.io";

const std::string_view UPDATE_LOG = R"(1. 优化列车间隔分析系统，支持车站对侧两事件之间的间隔。
2. 修改构建系统、更新支持库版本。
3. 首页右侧停靠面板默认设为自动隐藏。)";
}
