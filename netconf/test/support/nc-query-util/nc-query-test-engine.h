#ifndef __YUMA_TEST_TRANSFORM_ENGINE_H
#define __YUMA_TEST_TRANSFORM_ENGINE_H

// ---------------------------------------------------------------------------|
// Standard Includes
// ---------------------------------------------------------------------------|
#include <vector>
#include <string>
#include <cstdint>
#include <memory>

// ---------------------------------------------------------------------------|
// Test Harness includes
// ---------------------------------------------------------------------------|
#include "test/support/nc-query-util/nc-base-query-test-engine.h"

// ---------------------------------------------------------------------------|
namespace YumaTest
{
class AbstractYumaOpLogPolicy;
class AbstractNCQueryFactory;

// ---------------------------------------------------------------------------|
/**
 * Yuma-Test utility class for sending Netconf Messages to Yuma that
 * modify the current configuration.
 */
class NCQueryTestEngine : public NCBaseQueryTestEngine
{
public:
    /** 
     * Constructor.
     * The target DB name must be either 'running' or 'candidate'. Running
     * is only valid if Yuma is configured to allow modifications to the
     * running config.
     */
    NCQueryTestEngine();

    /** Destructor */
    virtual ~NCQueryTestEngine();

    /**
     * Load the specified module.
     * This method builds a NC-Query containing a load request,
     * injects the message into Yuma and parses the result to
     * determine if the module was loaded successfully.
     *
     * \tparam the type of results checker
     * \param moduleName the name of the module to load
     * \param session the session to use for injecting the message.
     * \param checker the results checker.
     */
    template< class Checker >
    void tryLoadModule( std::shared_ptr<AbstractNCSession> session,
                        const std::string& moduleName, 
                        Checker& checker )
    {
        // build a load module message for test.yang
        const std::string queryStr = getMessageBuilder().buildLoadMessage( 
                moduleName, session->allocateMessageId() );
        runQuery( session, queryStr, checker );
    }

    /**
     * Try to lock the database associated with this engine.
     *
     * \tparam the type of results checker
     * \param session the session to use for injecting the message.
     * \param target the target database name
     * \param checker the results checker.
     */
    template< class Checker >
    void tryLockDatabase( std::shared_ptr<AbstractNCSession> session,
                          const std::string& target, 
                          Checker& checker )
    {
        // build a lock module message for test.yang
        const std::string queryStr = getMessageBuilder().buildLockMessage( 
                session->allocateMessageId(), target );
        runQuery( session, queryStr, checker );
    }

    /**
     * Try to unlock the database associated with this engine.
     *
     * \tparam the type of results checker
     * \param session the session to use for injecting the message.
     * \param target the target database name
     * \param checker the results checker.
     */
    template< class Checker >
    void tryUnlockDatabase( std::shared_ptr<AbstractNCSession> session,
                            const std::string& target, 
                            Checker& checker )
    {
        // build an unlock module message for test.yang
        const std::string queryStr = getMessageBuilder().buildUnlockMessage( 
                session->allocateMessageId(), target );
        runQuery( session, queryStr, checker );
    }


    /**
     * Get the configuration filtered using the supplied xpath
     *
     * \tparam the type of results checker
     * \param session the session to use for injecting the message.
     * \param xPathFilterStr the XPath filter to apply
     * \param target the target database name
     * \param checker the results checker.
     */
    template< class Checker >
    void tryGetConfigXpath( std::shared_ptr<AbstractNCSession> session,
                            const std::string& xPathFilterStr,
                            const std::string& target, 
                            Checker& checker )
    {
        // build a load module message for test.yang
        const std::string queryStr = 
            getMessageBuilder().buildGetConfigMessageXPath( 
                xPathFilterStr, target, session->allocateMessageId() );
        runQuery( session, queryStr, checker );
    }
    
    /**
     * Edit the configuration using the supplied query
     *
     * \tparam the type of results checker
     * \param session the session to use for injecting the message.
     * \param query the edit config operation 
     * \param target the target database name
     * \param checker the results checker.
     */
    template< class Checker >
    void tryEditConfig( std::shared_ptr<AbstractNCSession> session,
                        const std::string& query,
                        const std::string& target, 
                        Checker& checker )
    {
        const std::string queryStr = 
            getMessageBuilder().buildEditConfigMessage( 
                query, target, session->allocateMessageId() );
        runQuery( session, queryStr, checker );
    }

    /**
     * Utility function for locking the database.
     *
     * \param session the session to use for injecting the message.
     * \param target the target database name
     */
    void unlock( std::shared_ptr<AbstractNCSession> session,
                 const std::string& target );

    /**
     * Utility function for unlocking the database.
     *
     * \param session the session to use for injecting the message.
     * \param target the target database name
     */
    void lock( std::shared_ptr<AbstractNCSession> session,
               const std::string& target );

    /**
     * Utility function for loading a module.
     *
     * \param session the session to use for injecting the message.
     * \param moduleName the name of the module to load 
     */
    void loadModule( std::shared_ptr<AbstractNCSession> session,
                     const std::string& moduleName );
    /**
     * Send a commit message.
     *
     * \param session the session to use for injecting the message.
     */
    void commit( std::shared_ptr<AbstractNCSession> session );
};    

// ---------------------------------------------------------------------------|
/**
 * RAII Lock guard for locking and unlocking of a database during
 * testing. Use this lock guard to ensure that the database is left
 * unlocked when the test exits even on test failure.
 */
class NCDbScopedLock
{
public:
    /** Constructor. This issues a lock command to the database. 
     *
     * \param engine shared_ptr to the query-test-engine.
     * \param session shared_ptr to the session.
     * \param target the target database name
     */
    NCDbScopedLock( std::shared_ptr< NCQueryTestEngine > engine, 
                    std::shared_ptr< AbstractNCSession > session,
                    const std::string& target );

    /**
     * Destructor.
     */
    ~NCDbScopedLock();

private:
    /** The engine that issues lock / unlock requests */
    std::shared_ptr< NCQueryTestEngine > engine_; 

    /** The session locking / unlocking the database */
    std::shared_ptr< AbstractNCSession > session_;

    /** The name of the target database */
    std::string target_;
};


} // namespace YumaTest

#endif // __YUMA_TEST_TRANSFORM_ENGINE_H
