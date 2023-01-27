#include "version.h"

namespace qespec {

const std::string_view VERSION = "V1.3.1";
const std::string_view LAST_VERSION = "V1.3.0";
const std::string_view DATE = "20230124";
const int RELEASE_CODE = 34;

const std::string_view DOC_URL = "https://qetrc.readthedocs.io";

const std::string_view UPDATE_LOG = R"(1. 重新设计列车筛选器相关功能；
2. 新增列车运行统计功能；
3. 明确标尺排图/贪心推线中的营业站设置规则。)";
}
