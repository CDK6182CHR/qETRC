#include "version.h"

namespace qespec {

const std::string_view VERSION = "V1.4.0";
const std::string_view LAST_VERSION = "V1.3.7";
const std::string_view DATE = "20230625";
const int RELEASE_CODE = 42;

const std::string_view DOC_URL = "https://qetrc.readthedocs.io";
const std::string_view DOC_URL_PREFIX = "https://qetrc.readthedocs.io/zh_CN/latest";

const std::string_view UPDATE_LOG = R"(1. 升级Qt库版本至6.5.1，更新一些尺寸设置。
2. 重新设计运行图纵线设置系统，增大设置自由度。
3. 新增类型管理器在当前运行图和默认设置间相互应用的功能。
4. 新增显示设置和类型管理器的“透明模式”。)";
}
