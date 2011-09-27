// ---------------------------------------------------------------------------|
// Yuma Test Harness includes
// ---------------------------------------------------------------------------|
#include "test/support/fixtures/integ-test-fixture.h"
#include "test/support/fixtures/test-context.h"
#include "test/support/misc-util/log-utils.h"
#include "test/support/nc-query-util/nc-query-test-engine.h"
#include "test/support/nc-query-util/yuma-op-policies.h"
#include "test/support/nc-session/spoof-nc-session-factory.h"

// ---------------------------------------------------------------------------|
// Standard includes
// ---------------------------------------------------------------------------|
#include <cassert>
#include <iostream>
#include <algorithm>
#include <memory>

// ---------------------------------------------------------------------------|
// Boost Test Framework
// ---------------------------------------------------------------------------|
#include <boost/test/unit_test.hpp>

// ---------------------------------------------------------------------------|
// Yuma includes for files under test
// ---------------------------------------------------------------------------|
#include "agt.h"
#include "ncx.h"
#include "ncxmod.h"

// ---------------------------------------------------------------------------|
// File wide namespace use and aliases
// ---------------------------------------------------------------------------|
using namespace std;

// ---------------------------------------------------------------------------|
namespace YumaTest
{

// ---------------------------------------------------------------------------|
// IntegrationTestFixture
// ---------------------------------------------------------------------------|
template<class SpoofedArgs, class OpPolicy>
IntegrationTestFixture<SpoofedArgs, OpPolicy>::IntegrationTestFixture()  
    : AbstractGlobalTestFixture( 
            ( sizeof( Args::argv )/sizeof( Args::argv[0] ) ),
            Args::argv )
{
    DisplayTestBreak( '=', true );
    BOOST_TEST_MESSAGE( "Initialising..." );

    // TODO: Ensure that the Traits target and Spoofed Args Targets match

    configureTestContext();
    initialiseNCXEngine();
    loadBaseSchemas();
    stage1AgtInitialisation();
    loadCoreSchema();
    stage2AgtInitialisation();
}

// ---------------------------------------------------------------------------|
template<class SpoofedArgs, class OpPolicy>
IntegrationTestFixture<SpoofedArgs, OpPolicy>::~IntegrationTestFixture()
{
    BOOST_TEST_MESSAGE( "IntegrationTestFixture() Cleaning Up..." );
    agt_cleanup();
    ncx_cleanup();
}

// ---------------------------------------------------------------------------|
template<class SpoofedArgs, class OpPolicy>
void IntegrationTestFixture<SpoofedArgs, OpPolicy>::configureTestContext()
{
   // TODO: Make these strings configurable - via environment
   // TODO: variables / test config file / traits?
   using std::shared_ptr;

   shared_ptr< AbstractYumaOpLogPolicy > queryLogPolicy( 
           new OpPolicy( "./yuma-op" ) );
   assert( queryLogPolicy );

   shared_ptr< AbstractNCSessionFactory > sessionFactory(
           new SpoofNCSessionFactory( queryLogPolicy ) ) ;

   shared_ptr< TestContext > testContext( 
           new TestContext( getTargetDbConfig(), sessionFactory ) );

   assert( testContext );

   TestContext::setTestContext( testContext );
}

// ---------------------------------------------------------------------------|
template<class SpoofedArgs, class OpPolicy>
void IntegrationTestFixture<SpoofedArgs, OpPolicy>::initialiseNCXEngine()
{
    BOOST_TEST_MESSAGE( "Initialising the NCX Engine..." );
    assert( "NCX Engine Failed to initialise" &&
            NO_ERR == ncx_init( FALSE, LOG_DEBUG_INFO, FALSE, 0, numArgs_, 
                                const_cast<char**>( argv_ ) ) ); 
}

// ---------------------------------------------------------------------------|
template<class SpoofedArgs, class OpPolicy>
void IntegrationTestFixture<SpoofedArgs, OpPolicy>::loadBaseSchemas()
{
    BOOST_TEST_MESSAGE( "Loading base schema..." );
    assert( "ncxmod_load_module( NCXMOD_YUMA_NETCONF ) failed!" &&
            NO_ERR == ncxmod_load_module( NCXMOD_YUMA_NETCONF, NULL, NULL, NULL ) );

    assert( "ncx_modload_module( NCXMOD_NETCONFD ) failed!" &&
            NO_ERR == ncxmod_load_module( NCXMOD_NETCONFD, NULL, NULL, NULL ) );
}

// ---------------------------------------------------------------------------|
template<class SpoofedArgs, class OpPolicy>
void IntegrationTestFixture<SpoofedArgs, OpPolicy>::loadCoreSchema()
{
    BOOST_TEST_MESSAGE( "Loading core schemas..." );
    agt_profile_t* profile = agt_get_profile();
    assert ( "agt_get_profile() returned a null profile" && profile );
    assert( "ncxmod_load_module( NCXMOD_WITH_DEFAULTS ) failed!" &&
            NO_ERR == ncxmod_load_module( NCXMOD_YUMA_NETCONF, NULL, 
                                          &profile->agt_savedevQ, NULL ) );

    assert( "ncx_modload_module( NCXMOD_NETCONFD failed!" &&
            NO_ERR == ncxmod_load_module( NCXMOD_NETCONFD, NULL, NULL, NULL ) );
}

// ---------------------------------------------------------------------------|
template<class SpoofedArgs, class OpPolicy>
void IntegrationTestFixture<SpoofedArgs, OpPolicy>::stage1AgtInitialisation()
{
    BOOST_TEST_MESSAGE( "AGT Initialisation stage 1..." );
    boolean showver = FALSE;
    help_mode_t showhelpmode = HELP_MODE_NONE;

    assert( "agt_init1() failed!" &&
            NO_ERR == agt_init1( numArgs_, const_cast<char**>( argv_ ),
                                 &showver, &showhelpmode ) );  
}

// ---------------------------------------------------------------------------|
template<class SpoofedArgs, class OpPolicy>
void IntegrationTestFixture<SpoofedArgs, OpPolicy>::stage2AgtInitialisation()
{
    BOOST_TEST_MESSAGE( "AGT Initialisation stage 2..." );

    assert( "agt_init2() failed!" && NO_ERR == agt_init2() );
}

} // namespace YumaTest
