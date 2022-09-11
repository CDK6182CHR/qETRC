#include "version.h"

namespace qespec {

const std::string_view VERSION = "V1.2.0";
const std::string_view LAST_VERSION = "V1.1.12";
const std::string_view DATE = "20220912";
const int RELEASE_CODE = 27;

const std::string_view DOC_URL = "https://qetrc.readthedocs.io";

const std::string_view UPDATE_LOG = R"(1. 【重要逻辑更新】新增单双线数据和线路拓扑功能。
2. 在时刻诊断功能中新增里程标和时刻显示和定位到运行图功能。
3. 新增打开运行图时检查运行图中基线信息有无错误。
)";
}
