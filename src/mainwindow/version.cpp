#include "version.h"

namespace qespec {

const std::string_view VERSION = "V1.3.5";
const std::string_view LAST_VERSION = "V1.3.4";
const std::string_view DATE = "20230604";
const int RELEASE_CODE = 39;

const std::string_view DOC_URL = "https://qetrc.readthedocs.io";

const std::string_view UPDATE_LOG = R"(1. 新增拖动调整时刻功能（对通过站，同时按住Ctrl调整到达时刻，按住Alt调整出发时刻，否则同时调整）。
2. 在交路编辑中新增刷新按钮。)";
}
