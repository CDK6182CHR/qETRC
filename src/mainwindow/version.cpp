#include "version.h"

namespace qespec {

const std::string_view VERSION = "V1.2.1";
const std::string_view LAST_VERSION = "V1.2.0";
const std::string_view DATE = "20220923";
const int RELEASE_CODE = 28;

const std::string_view DOC_URL = "https://qetrc.readthedocs.io";

const std::string_view UPDATE_LOG = R"(修复存在对应运行图页面的线路时，计算相关列车事件表、时刻诊断时，程序可能崩溃的问题。
)";
}
