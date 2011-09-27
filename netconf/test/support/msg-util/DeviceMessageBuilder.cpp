#include "test/support/msg-util/DeviceMessageBuilder.h"

#include <sstream>
#include <string>

#include "src/ncx/xml_util.h"

// ---------------------------------------------------------------------------!
using namespace std;

// ---------------------------------------------------------------------------!
// Anonymous Namespace
// ---------------------------------------------------------------------------!
namespace
{

// ---------------------------------------------------------------------------!
const char* IETF_BASE = "urn:ietf:params:netconf:base:1.0";
const char* IETF_NS = "urn:ietf:params:xml:ns:netconf:base:1.0";
const char* YUMA_NS = "http://netconfcentral.org/ns/yuma-system";

} // Anonymous namespace

namespace YumaTest
{

// ---------------------------------------------------------------------------!
DeviceMessageBuilder::DeviceMessageBuilder() : NCMessageBuilder() 
{
}

// ---------------------------------------------------------------------------!
DeviceMessageBuilder::~DeviceMessageBuilder()
{
}

} // namespace YumaTest
