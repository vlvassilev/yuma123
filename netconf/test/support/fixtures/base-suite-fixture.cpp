// ---------------------------------------------------------------------------|
// Test Harness includes
// ---------------------------------------------------------------------------|
#include "test/support/fixtures/base-suite-fixture.h"

// ---------------------------------------------------------------------------|
// Standard includes
// ---------------------------------------------------------------------------|
#include <iostream>
#include <memory>
#include <cassert>

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
BaseSuiteFixture::BaseSuiteFixture() 
    : testContext_( TestContext::getTestContext() )
    , sessionFactory_( testContext_->sessionFactory_ )
    , primarySession_( sessionFactory_->createSession() )
    , queryEngine_( testContext_->queryEngine_ )
    , writeableDbName_( testContext_->writeableDbName_ )
{
    assert( queryEngine_ );
}

// ---------------------------------------------------------------------------|
BaseSuiteFixture::~BaseSuiteFixture() 
{
    DisplayCurrentTestResult();
}

// ---------------------------------------------------------------------------|
std::vector< std::unique_ptr< NCDbScopedLock > >
BaseSuiteFixture::getFullLock( std::shared_ptr<AbstractNCSession> session )
{
    std::vector< std::unique_ptr< NCDbScopedLock > >locks;

    //TODO: Handle startup based on configuration
    locks.push_back( std::unique_ptr< NCDbScopedLock >( new  
            NCDbScopedLock( queryEngine_, session, "running" ) ) );

    if( useCandidate() )
    {
        locks.push_back( std::unique_ptr< NCDbScopedLock >( new  
            NCDbScopedLock( queryEngine_, session, "candidate" ) ) );
    }

    return locks;
}

// ---------------------------------------------------------------------------|
void BaseSuiteFixture::commitChanges( 
        std::shared_ptr<AbstractNCSession> session )
{
    if( useCandidate() )
    {
        // send a commit
        queryEngine_->commit( primarySession_ );
    }
}

// ---------------------------------------------------------------------------|
void BaseSuiteFixture::runEditQuery( 
        shared_ptr<AbstractNCSession> session,
        const string& query )
{
    assert( session );
    vector<string> expPresent{ "ok" };
    vector<string> expNotPresent{ "error", "rpc-error" };
    
    StringsPresentNotPresentChecker checker( expPresent, expNotPresent );
    queryEngine_->tryEditConfig( session, query, writeableDbName_, 
                                 checker );
}

} // namespace YumaTest
