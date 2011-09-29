#ifndef __YUMA_DEVICE_MODULE_TEST_FIXTURE__H
#define __YUMA_DEVICE_MODULE_TEST_FIXTURE__H

// ---------------------------------------------------------------------------|
// Test Harness includes
// ---------------------------------------------------------------------------|
#include "test/support/fixtures/base-suite-fixture.h"
#include "test/support/msg-util/DeviceMessageBuilder.h"

// ---------------------------------------------------------------------------|
// Standard includes
// ---------------------------------------------------------------------------|
#include <vector>
#include <map>
#include <string>
#include <memory>

// ---------------------------------------------------------------------------|
namespace YumaTest 
{
class AbstractNCSession;

// ---------------------------------------------------------------------------|
/**
 * This class is used to perform test case initialisation.
 * It can be used on a per test case basis or on a per test suite
 * basis.
 */
struct DeviceModuleFixture : public BaseSuiteFixture
{
public:
    /** Convenience typedef */
    typedef std::map< std::string, std::string > EntryMap_T;

    /** Convenience typedef */
    typedef std::shared_ptr< EntryMap_T > SharedPtrEntryMap_T;

public:
    /** 
     * Constructor. 
     */
    DeviceModuleFixture();

    /**
     * Destructor. Shutdown the test.
     */
    ~DeviceModuleFixture();

    /** 
     * Create the top level container.
     *
     * \param session the session running the query
     */
    void createMainContainer( std::shared_ptr<AbstractNCSession> session );

    /** 
     * Delete the top level container.
     *
     * \param session the session running the query
     */
    void deleteMainContainer( std::shared_ptr<AbstractNCSession> session );

    //TODO Device specific operations

    /** The NCMessage builder for the writeable database */
    DeviceMessageBuilder wrBuilder_;

    const std::string moduleName_;      ///< The module name
    const std::string moduleNs_;        ///< the module namespace
    const std::string containerName_;   ///< the container name 

    SharedPtrEntryMap_T runningEntries_; /// Running Db Entries 
    SharedPtrEntryMap_T candidateEntries_; /// CandidateTarget Db Entries 
};

} // namespace YumaTest

#endif // __YUMA_DEVICE_MODULE_TEST_FIXTURE__H
