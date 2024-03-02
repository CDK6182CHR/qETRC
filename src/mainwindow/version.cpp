#include "version.h"

namespace qespec {

const std::string_view VERSION = "V1.6.0";
const std::string_view LAST_VERSION = "V1.5.5";
const std::string_view DATE = "20240302";
const int RELEASE_CODE = 49;

const std::string_view DOC_URL = "https://qetrc.readthedocs.io";
const std::string_view DOC_URL_PREFIX = "https://qetrc.readthedocs.io/zh_CN/latest";

const std::string_view UPDATE_LOG = R"(1. 调整运行线铺画方式。与此前版本相比，相同设置项下的运行线宽度会减小。可在类型管理器中调整。
2. 新增浮动交路连线、交路连线显示选项。新增交路连线可选标注车次或交路名。
3. 新增车次标注形式选项，并新增交路连线模式及相关设置项。
4. 优化交路中的虚拟车次、不完整时间范围运行图的铺画等。)";
}
