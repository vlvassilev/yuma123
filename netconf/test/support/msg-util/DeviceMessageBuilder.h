#ifndef __YUMA_DEVICE_MESSAGE_BUILDER__H
#define __YUMA_DEVICE_MESSAGE_BUILDER__H

#include "test/support/msg-util/NCMessageBuilder.h"

#include <string>
#include <cstdint>

namespace YumaTest
{

// ---------------------------------------------------------------------------!
/**
 * Utility class for building device specific Netconf XML Messages.
 */
class DeviceMessageBuilder : public NCMessageBuilder
{
public:
    /** Constructor */
    explicit DeviceMessageBuilder();
 
    /** Destructor */
    virtual ~DeviceMessageBuilder();

//TODO Device specific message construction 
    
};

} // namespace YumaTest

#endif // __YUMA_DEVICE_MESSAGE_BUILDER__H
