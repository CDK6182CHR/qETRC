#include "version.h"

namespace qespec {

const std::string_view VERSION = "V1.5.0";
const std::string_view LAST_VERSION = "V1.4.0";
const std::string_view DATE = "20230823";
const int RELEASE_CODE = 42;

const std::string_view DOC_URL = "https://qetrc.readthedocs.io";
const std::string_view DOC_URL_PREFIX = "https://qetrc.readthedocs.io/zh_CN/latest";

const std::string_view UPDATE_LOG = R"(1. 【重要更新】新增列车径路数据项，支持相关数据管理及按径路铺画运行线。
2. 新增文本输出窗口和铺画问题列表窗口，默认浮动于窗口下方。)";
}
