#ifndef __CALLBACK_CHECKER_H
#define __CALLBACK_CHECKER_H

// ---------------------------------------------------------------------------|
// Test Harness includes
// ---------------------------------------------------------------------------|
#include "test/support/callbacks/sil-callback-log.h"

// ---------------------------------------------------------------------------|
// Standard includes
// ---------------------------------------------------------------------------|
#include <string>
#include <vector>

// ---------------------------------------------------------------------------|
namespace YumaTest
{

/**
 * Support class for checking callback information.
 */
class CallbackChecker
{
public:
    /** 
     * Constructor. 
     */
    CallbackChecker()
    {
    }
    
    /**
     * Destructor. Shutdown the test.
     */
    virtual ~CallbackChecker()
    {
    }

    /**
     * Add a callback to the log.
     *
     * \param modName the name of the module from which the callback is expected.
     * \param containerName the name of the top level container.
     * \param elementHierarchy a vector representing the hierarchy of elements 
     * leading to the one being operated on.
     * \param operation the operation being performed (e.g. get, edit, mro).
     * \param type the callback type.
     * \param phase the callback phase.
     */
    void addExpectedCallback(const std::string& modName, 
                             const std::string& containerName,
                             std::vector<std::string> elementHierarchy,
                             const std::string& operation,
                             const std::string& type,
                             const std::string& phase);

    /**
     * Check that the expected callbacks match those logged for a given module.
     *
     * \param modName the name of the module to check callbacks for.
     */
    void checkCallbacks(const std::string& modName);

    /**
     * Clear the expected callbacks.
     */
    void resetExpectedCallbacks();

private:
    SILCallbackLog::ModuleCallbackData expectedCallbacks_;
};

} // namespace YumaTest

#endif // __CALLBACK_CHECKER_H

//------------------------------------------------------------------------------
// End of file
//------------------------------------------------------------------------------
