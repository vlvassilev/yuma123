// ---------------------------------------------------------------------------|
// Boost Test Framework
// ---------------------------------------------------------------------------|
#include <boost/test/unit_test.hpp>

// ---------------------------------------------------------------------------|
// Standard includes
// ---------------------------------------------------------------------------|
#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <sstream>

// ---------------------------------------------------------------------------|
// Boost Includes
// ---------------------------------------------------------------------------|
#include <boost/range/algorithm.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/phoenix/core.hpp>
#include <boost/phoenix/fusion/at.hpp> 
#include <boost/phoenix/bind.hpp>
#include <boost/phoenix/operator.hpp> 

// ---------------------------------------------------------------------------|
// Yuma Test Harness includes
// ---------------------------------------------------------------------------|
#include "test/support/fixtures/device-module-fixture.h"
#include "test/support/misc-util/log-utils.h"
#include "test/support/nc-query-util/nc-query-test-engine.h"
#include "test/support/nc-session/abstract-nc-session-factory.h"
#include "test/support/callbacks/sil-callback-log.h"

// ---------------------------------------------------------------------------|
// Yuma includes for files under test
// ---------------------------------------------------------------------------|

// ---------------------------------------------------------------------------|
// File wide namespace use and aliases
// ---------------------------------------------------------------------------|
namespace ph = boost::phoenix;
namespace ph_args = boost::phoenix::arg_names;
using namespace std;

// ---------------------------------------------------------------------------|
namespace YumaTest {

BOOST_FIXTURE_TEST_SUITE( candidate_dt_tests, DeviceModuleFixture )

// ---------------------------------------------------------------------------|
// Add some data and check that it was added correctly
// ---------------------------------------------------------------------------|
BOOST_AUTO_TEST_CASE( dt_value_create )
{
    DisplayTestDescrption( 
            "Demonstrate Creation of XPO Container Profile, Stream & "
            "Resource entries.",
            "Procedure: \n"
            "\t 1 - Create the  XPO container\n"
            "\t 2 - Add profile-1 to the XPO container\n"
            "\t 3 - Add stream-1 to the profile-1\n"
            "\t 4 - Add resourceNode-1 to stream-1\n"
            "\t 5 - Add a VR-1 to stream-1\n"
            "\t 6 - Configure VR-1\n"
            "\t 7 - Add a VRConnection-1 to stream-1\n"
            "\t 8 - Configure VRConnection-1\n"
            "\t 9 - Add StreamConnection-1 to the XPO container\n"
            "\t10 - Configure StreamConnection-1\n"
            "\t11 - Add Profile-2 Stream-2 ResourceNode-2 in one operation\n"
            "\t12 - Add Profile-3\n"
            "\t13 - Add Profile-4\n"
            "\t14 - Set the activeProfile to Profile-1\n"
            "\t15 - Check that the Yuma database contains all expected items\n"
            "\t16 - Flush the contents so the container is empty for the next test\n"
            );

    // RAII Vector of database locks 
    vector< unique_ptr< NCDbScopedLock > > locks = getFullLock( primarySession_ );

    // Reset logged callbacks
    cbChecker_->resetExpectedCallbacks();
    cbChecker_->resetModuleCallbacks("device_test");

    // Create top level container and populate
    BOOST_TEST_MESSAGE("Create and populate xpo container\n");
    createXpoContainer( primarySession_ );
    createXpoProfile( primarySession_, 1 );
    createProfileStreamItem( primarySession_, 1, 1 );
    createStreamItemResourceNode( primarySession_, 1, 1, 1 );
    createVRConnectionNode( primarySession_, 1, 1, 1 );
    configureVResource( primarySession_, 1, 1, 1, 
            ResourceNodeConfig{ 100,  string( "/card[0]/sdiConnector[0]" ) } );
    configureVRConnection( primarySession_, 1, 1, 1, 
            ConnectionItemConfig{ 100, 200, 300, 400, 500 } );

    BOOST_TEST_MESSAGE("Commit initial changes\n");
    commitChanges( primarySession_ );

    //TODO Check callbacks for commit

    BOOST_TEST_MESSAGE("Create a stream connection\n");
    createStreamConnection( primarySession_, 1, 1 );
    configureStreamConnection( primarySession_, 1, 1, 
            StreamConnectionItemConfig{ 100, 200, 300, 400, 500, 600, 700 } );

    BOOST_TEST_MESSAGE("Set Active Profile\n");
    setActiveProfile( primarySession_, 1 );
    commitChanges( primarySession_ );

    // TODO Check callbacks for commit
    // Check database contents
    checkCandidate();
    checkRunning();
    
    BOOST_TEST_MESSAGE("Create additional Stream Items\n");
    createProfileStreamItem( primarySession_, 1, 2 );
    createProfileStreamItem( primarySession_, 1, 3 );
    createProfileStreamItem( primarySession_, 1, 4 );
    commitChanges( primarySession_ );

    // TODO Check callbacks for commit
    // Check database contents
    checkCandidate();
    checkRunning();

    BOOST_TEST_MESSAGE("Delete Container\n");
    deleteXpoContainer( primarySession_ );
}

// ---------------------------------------------------------------------------|
// Add some data and apply some merge operations to update it
// ---------------------------------------------------------------------------|
BOOST_AUTO_TEST_CASE( dt_value_merge )
{
    DisplayTestDescrption( 
            "Demonstrate update of XPO container via merge operations.",
            "Procedure: \n"
            "\t 1 - Create the  XPO container\n"
            "\t 2 - Add profile-1 to the XPO container\n"
            "\t 3 - Add stream-1 to the profile-1\n"
            "\t 4 - Add resourceNode-1 to stream-1\n"
            "\t 5 - Add a VR-1 to stream-1\n"
            "\t 6 - Configure VR-1\n"
            "\t 7 - Add a VRConnection-1 to stream-1\n"
            "\t 8 - Configure VRConnection-1\n"
            "\t 9 - Add StreamConnection-1 to the XPO container\n"
            "\t10 - Configure StreamConnection-1\n"
            "\t11 - Add Profile-2 Stream-2 ResourceNode-2 in one operation\n"
            "\t12 - Add Profile-3\n"
            "\t13 - Add Profile-4\n"
            "\t14 - Set the activeProfile to Profile-1\n"
            "\t15 - Check that the Yuma database contains all expected items\n"
            "\t16 - Flush the contents so the container is empty for the next test\n"
            );

    // RAII Vector of database locks 
    vector< unique_ptr< NCDbScopedLock > > locks = getFullLock( primarySession_ );

    // Reset logged callbacks
    cbChecker_->resetExpectedCallbacks();
    cbChecker_->resetModuleCallbacks("device_test");

    BOOST_TEST_MESSAGE("Create and populate xpo container\n");
    // Create top level container and populate
    createXpoContainer( primarySession_ );
    createXpoProfile( primarySession_, 1 );
    createProfileStreamItem( primarySession_, 1, 1 );
    createStreamItemResourceNode( primarySession_, 1, 1, 1 );
    createVRConnectionNode( primarySession_, 1, 1, 1 );
    configureVResource( primarySession_, 1, 1, 1, 
            ResourceNodeConfig{ 100,  string( "/card[0]/sdiConnector[0]" ) } );
    configureVRConnection( primarySession_, 1, 1, 1, 
            ConnectionItemConfig{ 100, 200, 300, 400, 500 } );
    commitChanges( primarySession_ );

    createStreamConnection( primarySession_, 1, 1 );
    configureStreamConnection( primarySession_, 1, 1, 
            StreamConnectionItemConfig{ 100, 200, 300, 400, 500, 600, 700 } );
    setActiveProfile( primarySession_, 1 );
    commitChanges( primarySession_ );

    
    BOOST_TEST_MESSAGE("Create additional Stream Items\n");
    createProfileStreamItem( primarySession_, 1, 2 );
    createProfileStreamItem( primarySession_, 1, 3 );
    createProfileStreamItem( primarySession_, 1, 4 );
    commitChanges( primarySession_ );

    BOOST_TEST_MESSAGE("Merge Active Profile\n");

    cbChecker_->resetExpectedCallbacks();
    cbChecker_->resetModuleCallbacks("device_test");

    // Apply merge to active profile
    mergeActiveProfile( primarySession_, 7 );

    // Check callbacks
    vector<string> elements = {"activeProfile"};
    cbChecker_->updateLeaf("device_test",
                           "xpo",
                           elements);
    cbChecker_->checkCallbacks("device_test");                               
    cbChecker_->resetModuleCallbacks("device_test");
    cbChecker_->resetExpectedCallbacks();

    // Check database entries - pre and post commit
    checkCandidate();
    checkRunning();
    commitChanges( primarySession_ );
    checkCandidate();
    checkRunning();

    BOOST_TEST_MESSAGE("Delete Container\n");
    checkCandidate();
    deleteXpoContainer( primarySession_ );
}

// ---------------------------------------------------------------------------|

BOOST_AUTO_TEST_SUITE_END()

} // namespace YumaTest

