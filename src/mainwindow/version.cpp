#include "version.h"

namespace qespec {

const std::string_view VERSION = "V1.2.4";
const std::string_view LAST_VERSION = "V1.2.3";
const std::string_view DATE = "20221119";
const int RELEASE_CODE = 31;

const std::string_view DOC_URL = "https://qetrc.readthedocs.io";

const std::string_view UPDATE_LOG = R"(1. 添加标尺排图/贪心推线中，新增导入既有车次停站时长的功能。
2. 在贪心推线中，新增筛选列车功能，只考虑通过筛选的车次。
3. 修复线路反排操作导致的错误。)";
}
