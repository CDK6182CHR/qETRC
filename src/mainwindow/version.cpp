#include "version.h"

namespace qespec {

const std::string_view VERSION = "V1.2.5";
const std::string_view LAST_VERSION = "V1.2.4";
const std::string_view DATE = "20221229";
const int RELEASE_CODE = 32;

const std::string_view DOC_URL = "https://qetrc.readthedocs.io";

const std::string_view UPDATE_LOG = R"(1. 新增线路拼接功能。
2. 在使用速度计算标尺功能中，新增选择应用区段的功能。
3. 从多线路文件导入线路时，新增可选导入部分线路。)";
}
