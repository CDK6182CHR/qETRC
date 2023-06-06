#include "version.h"

namespace qespec {

const std::string_view VERSION = "V1.3.6";
const std::string_view LAST_VERSION = "V1.3.5";
const std::string_view DATE = "20230606";
const int RELEASE_CODE = 40;

const std::string_view DOC_URL = "https://qetrc.readthedocs.io";

const std::string_view UPDATE_LOG = R"(1. 优化拖动调整时刻功能，新增铺画点突出显示。
2. 修正停站跨日所导致的时间计算错误问题。
3. 将区间车次表功能中的提示弹窗改为程序运行期间仅显示一次。)";
}
