#ifndef __SYSTEM_CALLBACK_CHECKER_H
#define __SYSTEM_CALLBACK_CHECKER_H

// ---------------------------------------------------------------------------|
// Test Harness includes
// ---------------------------------------------------------------------------|
#include "test/support/callbacks/callback-checker.h"

// ---------------------------------------------------------------------------|
// Standard includes
// ---------------------------------------------------------------------------|
#include <string>
#include <vector>

// ---------------------------------------------------------------------------|
namespace YumaTest
{

/**
 * Class to allow the skipping of callback checking during system tests.
 */
class SystemCBChecker : public CallbackChecker
{
public:
    /**
     * Add expected callbacks for adding a key value pair to a list.
     *
     * \param modName the name of the module from which the callbacks are expected.
     * \param containerName the name of the top level container.
     * \param listElement a vector representing the hierarchy of elements 
     * leading to the list that the pair will be added to.
     * \param key the key to be added.
     */
    virtual void addKey(const std::string& modName, 
                        const std::string& containerName,
                        const std::vector<std::string>& listElement,
                        const std::string& key);

    /**
     * Add expected callbacks for adding a key value pair to a list.
     *
     * \param modName the name of the module from which the callbacks are expected.
     * \param containerName the name of the top level container.
     * \param listElement a vector representing the hierarchy of elements 
     * leading to the list that the pair will be added to.
     * \param key the key to be added.
     * \param value the value to be added.
     */
    virtual void addKeyValuePair(const std::string& modName, 
                                 const std::string& containerName,
                                 const std::vector<std::string>& listElement,
                                 const std::string& key,
                                 const std::string& value);

    /**
     * Add expected callbacks for commiting a number of key value pairs to a list.
     *
     * \param modName the name of the module from which the callbacks are expected.
     * \param containerName the name of the top level container.
     * \param listElement a vector representing the hierarchy of elements 
     * leading to the list that the pair will be added to.
     * \param key the key to be added.
     * \param value the value to be added.
     * \param count the number of key value pairs to be added.
     */
    virtual void commitKeyValuePairs(const std::string& modName, 
                                     const std::string& containerName,
                                     const std::vector<std::string>& listElement,
                                     const std::string& key,
                                     const std::string& value,
                                     int count);

    /**
     * Check that the expected callbacks match those logged for a given module.
     *
     * \param modName the name of the module to check callbacks for.
     */
    virtual void checkCallbacks(const std::string& modName);
};

} // namespace YumaTest

#endif // __SYSTEM_CALLBACK_CHECKER_H

//------------------------------------------------------------------------------
// End of file
//------------------------------------------------------------------------------
