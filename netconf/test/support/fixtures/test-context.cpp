#include "test/support/fixtures/test-context.h"
#include "test/support/nc-query-util/nc-query-test-engine.h"

// ---------------------------------------------------------------------------|
namespace YumaTest 
{

// ---------------------------------------------------------------------------|
// initialise static data
std::shared_ptr<TestContext> TestContext::testContext_;

// ---------------------------------------------------------------------------|
TestContext::TestContext( 
        TargetDbConfig targetDbConfig,
        std::shared_ptr<AbstractNCSessionFactory> sessionFactory,
        std::shared_ptr<AbstractCBCheckerFactory> cbCheckerFactory )
    : targetDbConfig_( targetDbConfig )
    , queryEngine_( new NCQueryTestEngine() )
    , writeableDbName_( 
            targetDbConfig == TestContext::CONFIG_WRITEABLE_RUNNNIG ?
            "running" : "candidate" ) 
    , sessionFactory_( sessionFactory )
    , cbCheckerFactory_( cbCheckerFactory )
{
}

} // namespace YumaTest
