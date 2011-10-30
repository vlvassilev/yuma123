#ifndef __YUMA_QUERY_SUITE_FIXTURE__H
#define __YUMA_QUERY_SUITE_FIXTURE__H

// ---------------------------------------------------------------------------|
// Test Harness includes
// ---------------------------------------------------------------------------|
#include "test/support/fixtures/test-context.h"
#include "test/support/fixtures/base-suite-fixture.h"
#include "test/support/msg-util/NCMessageBuilder.h"
#include "test/support/callbacks/callback-checker.h"

// ---------------------------------------------------------------------------|
// Standard includes
// ---------------------------------------------------------------------------|
#include <vector>
#include <memory>
#include <map>

// ---------------------------------------------------------------------------|
// File wide namespace use
// ---------------------------------------------------------------------------|
using namespace std;

// ---------------------------------------------------------------------------|
namespace YumaTest 
{
class YumaQueryOpLogPolicy;
class NCQueryTestEngine;
class NCDbScopedLock;
class AbstractNCSession;
class AbstractNCSessionFactory;
class CallbackChecker;
class AbstractCBCheckerFactory;

// ---------------------------------------------------------------------------|
/**
 * This class is used to perform simple test case initialisation.
 * It can be used on a per test case basis or on a per test suite
 * basis. This fixture provides basic functionality for querying Yuma.
 */
class QuerySuiteFixture : public BaseSuiteFixture
{
public:
    /** 
     * Constructor. 
     */
    QuerySuiteFixture();

    /** 
     * Constructor. 
     * \param builder an NCMessageBuilder
     */
    QuerySuiteFixture( std::shared_ptr< NCMessageBuilder> builder);
    
    /**
     * Destructor. Shutdown the test.
     */
    virtual ~QuerySuiteFixture();

    /** 
     * This function is used to obtain a full lock of the system under
     * test. 
     *
     * If the configuration is writeable running the 'startup' and
     * 'running' databases are locked.
     *
     * If the configuration is candidate the 'startup', 'running' and
     * 'candidate' databases are locked.
     *
     * \param session  the session requesting the locks
     * \return a vector of RAII locks. 
     */
    std::vector< std::unique_ptr< NCDbScopedLock > >
    getFullLock( std::shared_ptr<AbstractNCSession> session );

    /**
     * Commit the changes.
     * If the configuration is CONFIG_USE_CANDIDATE a commit message
     * is sent to Yuma.
     *
     * \param session  the session requesting the locks
     */
    virtual void commitChanges( std::shared_ptr<AbstractNCSession> session );

    /**
     * Run an edit query.
     *
     * \param session the session running the query
     * \param query the query to run
     */
    void runEditQuery( std::shared_ptr<AbstractNCSession> session,
                       const std::string& query );

    /**
     * Run an edit query.
     *
     * \param session the session running the query.
     * \param query the query to run.
     * \param failReson the expected fail reason.
     */
    void runFailedEditQuery( std::shared_ptr<AbstractNCSession> session,
                       const std::string& query,
                       const std::string& failReason );

    /** the session factory. */
    std::shared_ptr<AbstractNCSessionFactory> sessionFactory_;

    /** the cb checker factory. */
    std::shared_ptr<AbstractCBCheckerFactory> cbCheckerFactory_;
    
    /** Each test always has one session */
    std::shared_ptr<AbstractNCSession> primarySession_;

    /** Each test always has one cb checker */
    std::shared_ptr<CallbackChecker> cbChecker_;

    /** Utility XML message builder */
    std::shared_ptr<NCMessageBuilder> messageBuilder_;

    /** The Query Engine */
    std::shared_ptr<NCQueryTestEngine> queryEngine_;
};

} // namespace YumaTest

// ---------------------------------------------------------------------------|

#endif // __YUMA_QUERY_SUITE_FIXTURE__H

