#include "version.h"

namespace qespec {

const std::string_view VERSION = "V1.5.2";
const std::string_view LAST_VERSION = "V1.5.1";
const std::string_view DATE = "20230930";
const int RELEASE_CODE = 45;

const std::string_view DOC_URL = "https://qetrc.readthedocs.io";
const std::string_view DOC_URL_PREFIX = "https://qetrc.readthedocs.io/zh_CN/latest";

const std::string_view UPDATE_LOG = R"(1. 修复批量复制车次中重复车次检测问题。
2. 修复部分情况下，选中含交路列车导致程序崩溃问题。
3. 修复导入按标尺排图的线路的相关问题。)";
}
