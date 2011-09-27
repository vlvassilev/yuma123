// ---------------------------------------------------------------------------|
// Test Harness includes
// ---------------------------------------------------------------------------|
#include "test/support/fixtures/device-module-fixture.h"

// ---------------------------------------------------------------------------|
// Standard includes
// ---------------------------------------------------------------------------|
#include <iostream>
#include <memory>
#include <algorithm>
#include <cassert>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>

// ---------------------------------------------------------------------------|
// Test Harness includes
// ---------------------------------------------------------------------------|
#include "test/support/nc-query-util/nc-query-test-engine.h"
#include "test/support/nc-session/abstract-nc-session-factory.h"
#include "test/support/misc-util/log-utils.h"
#include "test/support/checkers/string-presence-checkers.h"

// ---------------------------------------------------------------------------|
// File wide namespace use
// ---------------------------------------------------------------------------|
using namespace std;

// ---------------------------------------------------------------------------|
namespace YumaTest 
{

// ---------------------------------------------------------------------------|
DeviceModuleFixture::DeviceModuleFixture() 
    : BaseSuiteFixture()
    , wrBuilder_()
    , moduleName_( "device_test" )
    , moduleNs_( "http://netconfcentral.org/ns/device_test" )
    , containerName_( "part1" )
    , runningEntries_( new EntryMap_T() )
    , candidateEntries_( useCandidate() ? SharedPtrEntryMap_T( new EntryMap_T() )
                                        : runningEntries_ )
{
    // ensure the module is loaded
    queryEngine_->loadModule( primarySession_, "device_test" );
}

// ---------------------------------------------------------------------------|
DeviceModuleFixture::~DeviceModuleFixture() 
{
}

// ---------------------------------------------------------------------------|
void DeviceModuleFixture::createMainContainer(
    std::shared_ptr<AbstractNCSession> session )
{
    assert( session );
    string query = wrBuilder_.genTopLevelContainerText( 
            containerName_, moduleNs_, "create" );
    runEditQuery( session, query );
}

// ---------------------------------------------------------------------------|
void DeviceModuleFixture::deleteMainContainer(
    std::shared_ptr<AbstractNCSession> session )
{
    string query = wrBuilder_.genTopLevelContainerText( 
            containerName_, moduleNs_, "delete" );
    runEditQuery( session, query );
    
    //TODO
    //candidateEntries_->clear();
}

// ---------------------------------------------------------------------------|

} // namespace YumaTest
