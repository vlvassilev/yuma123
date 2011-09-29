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

    // create the top level container
    createMainContainer( primarySession_ );

    // Add some entries
    //TODO

    //TODO
    //checkEntries( primarySession_ );

    // remove all entries
    deleteMainContainer( primarySession_ );

    //TODO
    //checkEntries( primarySession_ );
}

// ---------------------------------------------------------------------------|
BOOST_AUTO_TEST_SUITE_END()

} // namespace YumaTest

