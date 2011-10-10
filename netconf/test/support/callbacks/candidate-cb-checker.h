#ifndef __CANDIDATE_CALLBACK_CHECKER_H
#define __CANDIDATE_CALLBACK_CHECKER_H

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
 * Support class for checking callback information when the candidate database
 * is in use.
 */
class CandidateCBChecker : public CallbackChecker
{
public:
    /**
     * Add expected callbacks for adding a key to a list.
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
     * Add expected callbacks for adding a leaf to a container or updating a
     * leaf in a container.
     *
     * \param modName the name of the module from which the callbacks are expected.
     * \param containerName the name of the top level container.
     * \param listElement a vector representing the hierarchy of elements 
     * leading to the leaf.
     * \param phase the specific edit operation which has beem invoked.
     */

    virtual void updateLeaf(const std::string& modName, 
                            const std::string& containerName,
                            const std::vector<std::string>& listElement,
                            const std::string& phase = "merge");

    /**
     * Add expected callbacks for creating a container or updating a container.
     *
     * \param modName the name of the module from which the callbacks are expected.
     * \param containerName the name of the top level container.
     * \param listElement a vector representing the hierarchy of elements 
     * leading to the container; an empty vector means the top level container is
     * being checked.
     * \param phase the specific edit operation which has beem invoked.
     */

    virtual void updateContainer(const std::string& modName, 
                                 const std::string& containerName,
                                 const std::vector<std::string>& listElement,
                                 const std::string& phase = "merge");

};

} // namespace YumaTest

#endif // __CANDIDATE_CALLBACK_CHECKER_H

//------------------------------------------------------------------------------
// End of file
//------------------------------------------------------------------------------
