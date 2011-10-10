#ifndef __YUMA_DEVICE_MODULE_TEST_FIXTURE__H
#define __YUMA_DEVICE_MODULE_TEST_FIXTURE__H

// ---------------------------------------------------------------------------|
// Test Harness includes
// ---------------------------------------------------------------------------|
#include "test/support/fixtures/base-suite-fixture.h"
#include "test/support/msg-util/xpo-query-builder.h"
#include "test/support/db-models/device-test-db.h"

// ---------------------------------------------------------------------------|
// Standard includes
// ---------------------------------------------------------------------------|
#include <vector>
#include <map>
#include <string>
#include <memory>

// ---------------------------------------------------------------------------|
namespace YumaTest 
{
class AbstractNCSession;

// ---------------------------------------------------------------------------|
/**
 * This class is used to perform test case initialisation.
 * It can be used on a per test case basis or on a per test suite
 * basis.
 */
struct DeviceModuleFixture : public BaseSuiteFixture
{
public:
    /** Convenience typedef */
    typedef std::map< std::string, std::string > EntryMap_T;

    /** Convenience typedef */
    typedef std::shared_ptr< EntryMap_T > SharedPtrEntryMap_T;

public:
    /** 
     * Constructor. 
     */
    DeviceModuleFixture();

    /**
     * Destructor. Shutdown the test.
     */
    ~DeviceModuleFixture();

    /**
     * Run an edit query.
     *
     * \param session the session running the query
     * \param query the query to run
     */
    void runEditQuery( std::shared_ptr<AbstractNCSession> session,
                       const std::string& query );

    /** 
     * Create the top level containers.
     *
     * \param session the session running the query
     */
    void createXpoContainer
            ( std::shared_ptr<AbstractNCSession> session );

    /** 
     * Delete the top level containers.
     *
     * \param session the session running the query
     */
    void deleteXpoContainer
            ( std::shared_ptr<AbstractNCSession> session,
            bool commitChange = true );

    //Device specific operations

    /** 
     * Create the active profile container.
     *
     * \param session the session running the query.
     * \param profileId the is of the profile.
     */
    void setActiveProfile(
            std::shared_ptr<YumaTest::AbstractNCSession> session,
        uint16_t profileId );

    /** 
     * Chenge the active profile container with a merge.
     *
     * \param session the session running the query.
     * \param profileId the is of the profile.
     */
    void mergeActiveProfile(
            std::shared_ptr<YumaTest::AbstractNCSession> session,
        uint16_t profileId );

    /** 
     * Create the xpo level container.
     *
     * \param session the session running the query.
     * \param profileId the is of the profile.
     */
    void createXpoProfile( 
            std::shared_ptr<YumaTest::AbstractNCSession> session,
            uint16_t profileId );

    /** 
     * Create a StreamItem for a profile.
     *
     * \param session the session running the query.
     * \param profileId the of the owning profile.
     * \param streamId the of the stream to create. 
     */
    void createProfileStreamItem( 
            std::shared_ptr<YumaTest::AbstractNCSession> session,
            uint16_t profileId,
            uint16_t streamId );

    /** 
     * Create a resource node .
     *
     * \param session the session running the query.
     * \param profileId the of the owning profile.
     * \param streamId the of the stream to create. 
     * \param resourceNodeId the id of the resource node to create.
     */
    void createStreamItemResourceNode( 
            std::shared_ptr<YumaTest::AbstractNCSession> session,
            uint16_t profileId,
            uint16_t streamId,
            uint16_t resourceNodeId );

    /** 
     * Create a VR connection node .
     *
     * \param session the session running the query.
     * \param profileId the of the owning profile.
     * \param streamId the of the stream to create. 
     * \param connectionId the id of the connection node to create.
     */
    void createVRConnectionNode( 
            std::shared_ptr<YumaTest::AbstractNCSession> session,
            uint16_t profileId,
            uint16_t streamId,
            uint16_t connectionId );

     /**
     * Configure a Virtual resource item.
     * This function generates the complete configuration for a
     * ResourceNode, items are configured depending on whether or
     * not they have been set in the supplied ResourceNodeConfig.
     *
     * \param session the session running the query.
     * \param profileId the of the owning profile.
     * \param streamId the of the stream to create. 
     * \param connectionId the id of the connection node to create.
     * \param config the configuration to generate
     */
    void configureVResource( 
            std::shared_ptr<YumaTest::AbstractNCSession> session,
            uint16_t profileId,
            uint16_t streamId,
            uint16_t resourceId,
            const YumaTest::ResourceNodeConfig& config );

    /**
     * Configure a virtual resource connection item.
     * This function generates the complete configuration for a
     * ConnectionItem, items are configured depending on whether or
     * not they have been set in the supplied ConnectionItemConfig.
     *
     * \param session the session running the query.
     * \param profileId the of the owning profile.
     * \param streamId the of the stream to create. 
     * \param connectionId the id of the connection node to create.
     * \param config the configuration to generate
     */
     void configureVRConnection( 
             std::shared_ptr<YumaTest::AbstractNCSession> session,
             uint16_t profileId,
             uint16_t streamId,
             uint16_t connectionId,
             const YumaTest::ConnectionItemConfig& config );
    /**
     * Configure a stream connection item.
     * This function generates the complete configuration for a
     * ConnectionItem, items are configured depending on whether or
     * not they have been set in the supplied ConnectionItemConfig.
     *
     * \param session the session running the query.
     * \param profileId the of the owning profile.
     * \param connectionId the id of the connection node to create.
     * \param config the configuration to generate
     */
     void configureStreamConnection( 
             std::shared_ptr<YumaTest::AbstractNCSession> session,
             uint16_t profileId,
             uint16_t connectionId,
             const YumaTest::StreamConnectionItemConfig& config );

    /** 
     * Store connection configuration data in a connection item.
     *
     * \tparam ConnectionItemType the connection item type
     * \tparam ConfigType the config item type
     * \param connectionItem the connection item being updated
     * \param config the new configuration.
     */
    template< class ConnectionItemType, class ConfigType >
    void storeConnectionItemConfig( 
        ConnectionItemType& connectionItem, 
        const ConfigType& config );

    /**
     * Create a new stream connection.
     *
     * \param session the session running the query.
     * \param profileId the of the owning profile.
     * \param id the id of the stream
     */
    void createStreamConnection(
            std::shared_ptr<YumaTest::AbstractNCSession> session,
            uint16_t profileId,
            uint16_t id );

    /**
     * Commit the changes.
     *
     * \param session  the session requesting the locks
     */
    virtual void commitChanges( 
            std::shared_ptr<YumaTest::AbstractNCSession> session );

    /**
     * Let the test harness know that changes should be discarded
     * (e.g. due to unlocking the database without a commit.
     */
    void discardChanges();

    /**
     * Utility function to ensure that the connection exists in the 
     * candidateEntries_ tree. If the item does not exist it will be created, 
     * initialised and added.
     *
     * \param profileId the of of the profile.
     * \param connectionId the of of the connection.
     */
    void ensureStreamConnectionExists( uint16_t profileId,
                                       uint16_t connectionId );

    /**
     * Utility function to ensure that the profile exists in the
     * candidateEntries_ tree. If the item does not exist it will be created, 
     * initialised and added.
     *
     * \param profileId the of of the connection.
     */
    void ensureProfileExists( uint16_t profileId );

    /**
     * Utility function to ensure that the stream exists for the 
     * profile in the candidateEntries_ tree. If the item does not exist
     * it will be created, initialised and added.
     *
     * \param profileId the of of the profile.
     * \param streamId the of of the connection.
     */
    void ensureProfileStreamExists( uint16_t profileId, 
                                    uint16_t streamId );

    /**
     * Utility function to ensure that the resource exists for the
     * stream of a profile in the candidateEntries_ tree. If the item does
     * not exist it will be created, initialised and added.
     *
     * \param profileId the of of the profile.
     * \param streamId the of of the connection.
     * \param resourceId the of of the resource.
     */
    void ensureProfileStreamResourceExists( uint16_t profileId,
                                            uint16_t streamId, 
                                            uint16_t resourceId );
    
    /**
     * Utility function to ensure that the connection exists for the
     * stream of a profile in the candidateEntries_ tree. If the item does
     * not exist it will be created, initialised and added.
     *
     * \param profileId the of of the connection.
     * \param streamId the of of the connection.
     * \param connectionId the of of the connection.
     */
    void ensureProfileStreamVRConnectionExists( uint16_t profileId,
                                                uint16_t streamId, 
                                                uint16_t connectionId );

    /** 
     * Get the contents of the candidate database and verify
     * that all entries are as expected.
     */
    void checkCandidate();


    /** 
     * Get the contents of the running database and verify
     * that all entries are as expected.
     */
    void checkRunning();


    /** The NCMessage builder for the writeable database */
    YumaTest::XPOQueryBuilder xpoBuilder_;
    //TODO Add builder for part1

    /// Running Db Entries
    std::shared_ptr<YumaTest::XPO3Container> runningEntries_;  
    /// CandidateTarget Db Entries
    std::shared_ptr<YumaTest::XPO3Container> candidateEntries_;  
};

} // namespace YumaTest

#endif // __YUMA_DEVICE_MODULE_TEST_FIXTURE__H
