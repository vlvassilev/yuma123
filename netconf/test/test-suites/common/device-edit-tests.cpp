// ---------------------------------------------------------------------------|
// Boost Test Framework
// ---------------------------------------------------------------------------|
#include <boost/test/unit_test.hpp>

#include <cstdint>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>

// ---------------------------------------------------------------------------|
// Yuma Test Harness includes
// ---------------------------------------------------------------------------|
#include "test/support/fixtures/device-module-fixture.h"
#include "test/support/misc-util/log-utils.h"
#include "test/support/nc-query-util/nc-query-test-engine.h"
#include "test/support/checkers/string-presence-checkers.h"

// ---------------------------------------------------------------------------|
// Yuma includes for files under test
// ---------------------------------------------------------------------------|

// ---------------------------------------------------------------------------|
// File wide namespace use
// ---------------------------------------------------------------------------|
using namespace std;

// ---------------------------------------------------------------------------|
namespace YumaTest {

BOOST_FIXTURE_TEST_SUITE( device_get_test_suite_running, DeviceModuleFixture )

// ---------------------------------------------------------------------------|
// Simple get of database data
// ---------------------------------------------------------------------------|
BOOST_AUTO_TEST_CASE( get_dt_entries_using_xpath_filter )
{
    DisplayTestDescrption( 
            "Demonstrate retrieval of a module's entries.",
            "Procedure: \n"
            "\t1 - Query the entries for the 'device_test' module "
            "using an xpath filter" );
   
    vector<string> expPresent{ "data", "rpc-reply" };
    vector<string> expNotPresent{ "error", "rpc-error" };

    StringsPresentNotPresentChecker checker( expPresent, expNotPresent );
    queryEngine_->tryGetConfigXpath( primarySession_, "device_test", 
            "running", checker );
}

// ---------------------------------------------------------------------------|
// Demonstrate poorly formed xml messages are ignored
// ---------------------------------------------------------------------------|
BOOST_AUTO_TEST_CASE( edit_device_test_entries_badxml )
{
    DisplayTestDescrption( 
            "Demonstrate that poorly formed XML create requests are rejected.",
            "Procedure: \n"
            "\t1 - Create a query containing poorly formed xml (missing "
            "termination /\n"
            "\t2 - Inject the message and check that it was ignored.\n\n"
            "Note: originally this caused Yuma to exit with SIG SEGV" );

    // build the query
    stringstream query;

    query << "<theList xmlns:nc=\"urn:ietf:params:xml:ns:netconf:base:1.0\"\n"
            "    nc:operation=\"create\"\n"
            "    xmlns=\"http://netconfcentral.org/ns/device_test\">\n";
   
    vector<string> expPresent{};
    vector<string> expNotPresent{ "error" "ok", "rpc-reply", "rpc-error" };

    StringsPresentNotPresentChecker checker( expPresent, expNotPresent );
    queryEngine_->tryEditConfig( primarySession_, query.str(), "running",
                                 checker );
}

// ---------------------------------------------------------------------------|
// Add some data and check that it was added correctly
// ---------------------------------------------------------------------------|
BOOST_AUTO_TEST_CASE( edit_dt_entries_test1 )
{
    DisplayTestDescrption( 
            "Demonstrate modification of a simple container.",
            "Procedure: \n"
            "\t1 - Create the top level container\n"
            "\t2 - TODO\n"
            );

    // RAII Vector of database locks 
    vector< unique_ptr< NCDbScopedLock > > locks = getFullLock( primarySession_ );

    //Reset logged callbacks
    cbChecker_->resetModuleCallbacks("device_test");
    cbChecker_->resetExpectedCallbacks();

    BOOST_TEST_MESSAGE("Create xpo container\n");
    // Create top level container
    createXpoContainer( primarySession_ );

    // Check callbacks
    vector<string> xpo_elements = {};
    cbChecker_->updateContainer("device_test",
                                "xpo",
                                xpo_elements,
                                "create");
    cbChecker_->checkCallbacks("device_test");                             
    cbChecker_->resetModuleCallbacks("device_test");
    cbChecker_->resetExpectedCallbacks();


    BOOST_TEST_MESSAGE("Add a profile\n");
    // Add a profile
    createXpoProfile( primarySession_, 1 );

    // Check callbacks
    vector<string> elements = {"profile"};
    cbChecker_->addKey("device_test",
                       "xpo",
                       elements,
                       "id");
    cbChecker_->checkCallbacks("device_test");                               
    cbChecker_->resetModuleCallbacks("device_test");
    cbChecker_->resetExpectedCallbacks();            

    BOOST_TEST_MESSAGE("Add a stream item\n");
    // Add a stream item
    createProfileStreamItem( primarySession_, 1, 1 );

    // Check callbacks
    elements.push_back("stream");
    cbChecker_->addKey("device_test",
                       "xpo",
                       elements,
                       "id");
    cbChecker_->checkCallbacks("device_test");                               
    cbChecker_->resetModuleCallbacks("device_test");

    // Add a stream item
    createProfileStreamItem( primarySession_, 1, 2 );

    // Check callbacks
    cbChecker_->checkCallbacks("device_test");                               
    cbChecker_->resetModuleCallbacks("device_test");

    // Add a stream item
    createProfileStreamItem( primarySession_, 1, 3 );

    // Check callbacks
    cbChecker_->checkCallbacks("device_test");                               
    cbChecker_->resetModuleCallbacks("device_test");
    cbChecker_->resetExpectedCallbacks(); 
     
    // Check database      
    checkCandidate();

    // remove all entries
    deleteXpoContainer( primarySession_ );

    // Check database      
    checkCandidate();

    //TODO - extend to fully populate database - see device-edit-tests-candidate
}

// ---------------------------------------------------------------------------|
BOOST_AUTO_TEST_SUITE_END()

} // namespace YumaTest

