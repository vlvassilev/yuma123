// ---------------------------------------------------------------------------|
// Test Harness includes
// ---------------------------------------------------------------------------|
#include "test/support/fixtures/query-suite-fixture.h"

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
#include "test/support/callbacks/abstract-cb-checker-factory.h"
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
QuerySuiteFixture::QuerySuiteFixture() 
    : BaseSuiteFixture()
    , sessionFactory_( testContext_->sessionFactory_ )
    , cbCheckerFactory_( testContext_->cbCheckerFactory_ )
    , primarySession_( sessionFactory_->createSession() )
    , cbChecker_( cbCheckerFactory_->createChecker() )
    , messageBuilder_( new NCMessageBuilder() )
    , queryEngine_( new NCQueryTestEngine( messageBuilder_ ) )
{
    assert( queryEngine_ );
}

// ---------------------------------------------------------------------------|
QuerySuiteFixture::QuerySuiteFixture( shared_ptr<NCMessageBuilder> builder ) 
    : BaseSuiteFixture()
    , sessionFactory_( testContext_->sessionFactory_ )
    , cbCheckerFactory_( testContext_->cbCheckerFactory_ )
    , primarySession_( sessionFactory_->createSession() )
    , cbChecker_( cbCheckerFactory_->createChecker() )
    , messageBuilder_( builder )
    , queryEngine_( new NCQueryTestEngine( messageBuilder_ ) )
{
    assert( queryEngine_ );
}

// ---------------------------------------------------------------------------|
QuerySuiteFixture::~QuerySuiteFixture() 
{
}

// ---------------------------------------------------------------------------|
std::vector< std::unique_ptr< NCDbScopedLock > >
QuerySuiteFixture::getFullLock( std::shared_ptr<AbstractNCSession> session )
{
    vector< std::unique_ptr< NCDbScopedLock > >locks;

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
void QuerySuiteFixture::commitChanges( 
        std::shared_ptr<AbstractNCSession> session )
{
    if( useCandidate() )
    {
        // send a commit
        queryEngine_->commit( primarySession_ );
    }
}

// ---------------------------------------------------------------------------|
void QuerySuiteFixture::runEditQuery( 
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

// ---------------------------------------------------------------------------|
void QuerySuiteFixture::runFailedEditQuery( 
        shared_ptr<AbstractNCSession> session,
        const string& query,
        const string& failReason )
{
    assert( session );
    vector<string> expNotPresent{ "ok" };
    vector<string> expPresent{ "error", "rpc-error", failReason };
    
    StringsPresentNotPresentChecker checker( expPresent, expNotPresent );
    queryEngine_->tryEditConfig( session, query, writeableDbName_, 
                                 checker );
}

} // namespace YumaTest

