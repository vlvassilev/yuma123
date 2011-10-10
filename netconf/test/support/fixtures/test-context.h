#ifndef __YUMA_TEST_CONTEXT_H
#define __YUMA_TEST_CONTEXT_H

// ---------------------------------------------------------------------------|
// Standard includes
// ---------------------------------------------------------------------------|
#include <memory>
#include <string>

// ---------------------------------------------------------------------------|
namespace YumaTest 
{
class AbstractNCSessionFactory;
class AbstractCBCheckerFactory;
class NCQueryTestEngine;

/**
 * BOOST Test does not have any way of accessing global test suite
 * fixture configuration. This class provides a 'singleton' like way
 * of accessing any test configration data set up by the global test
 * fixture.
 */
struct TestContext
{
    enum TargetDbConfig
    {
        CONFIG_WRITEABLE_RUNNNIG,
        CONFIG_USE_CANDIDATE,
    };

    /** 
     * Contructor
     *
     * \param targetDbConfig the name of the target database 
     *                 (either 'running' or 'candidate' )
     * \param queryFactory the factory to use when creating NCQueries.
     */
    TestContext( TargetDbConfig targetDbConfig,
                 std::shared_ptr<AbstractNCSessionFactory> queryFactory,
                 std::shared_ptr<AbstractCBCheckerFactory> cbCheckerFactory );

    /**
     * Set the new test context.
     *
     * \param newContext the new test context.
     */
    static void setTestContext( std::shared_ptr<TestContext> newContext )
    {
        testContext_ = newContext; 
    }

    /**
     * Get the test context.
     *
     * \return the test context.
     */
    static std::shared_ptr<TestContext> getTestContext()
    {
        return testContext_; 
    }

    /** The configuration */
    TargetDbConfig targetDbConfig_;

    /** the engine used for injecting queries */
    std::shared_ptr<NCQueryTestEngine> queryEngine_;

    /** the name of the writable database (either running or candidate) */
    std::string writeableDbName_;

    /** the session factory. */
    std::shared_ptr<AbstractNCSessionFactory> sessionFactory_;

    /** the cb checker factory. */
    std::shared_ptr<AbstractCBCheckerFactory> cbCheckerFactory_;

private:    
    /** The current test context */
    static std::shared_ptr<TestContext> testContext_;

};

} // namespace YumaTest


#endif // __YUMA_TEST_CONTEXT_H
