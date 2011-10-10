// ---------------------------------------------------------------------------|
// Test Harness includes
// ---------------------------------------------------------------------------|
#include "test/support/fixtures/device-module-fixture.h"

// ---------------------------------------------------------------------------|
// Standard includes
// ---------------------------------------------------------------------------|
#include <iostream>
#include <memory>
#include <algorithm>
#include <cassert>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>

// ---------------------------------------------------------------------------|
// Test Harness includes
// ---------------------------------------------------------------------------|
#include "test/support/nc-query-util/nc-query-test-engine.h"
#include "test/support/nc-session/abstract-nc-session-factory.h"
#include "test/support/misc-util/log-utils.h"
#include "test/support/checkers/xml-content-checker.h" 
#include "test/support/checkers/string-presence-checkers.h"

// ---------------------------------------------------------------------------|
// File wide namespace use
// ---------------------------------------------------------------------------|
using namespace std;

// ---------------------------------------------------------------------------|
namespace YumaTest 
{

// ---------------------------------------------------------------------------|
DeviceModuleFixture::DeviceModuleFixture() 
    : BaseSuiteFixture()
    , xpoBuilder_( "http://netconfcentral.org/ns/device_test" )
    , runningEntries_( new XPO3Container() )
    , candidateEntries_( useCandidate()
            ? shared_ptr<XPO3Container>( new XPO3Container )
            : runningEntries_ )
{
    // ensure the module is loaded
    queryEngine_->loadModule( primarySession_, "device_test" );
}

// ---------------------------------------------------------------------------|
DeviceModuleFixture::~DeviceModuleFixture() 
{
}

// ---------------------------------------------------------------------------|
void DeviceModuleFixture::ensureProfileExists( const uint16_t profileId )
{
    if ( candidateEntries_->profiles_.end() == 
         candidateEntries_->profiles_.find( profileId ) )
    {
        Profile newProf;
        newProf.id_ = profileId;

        SharedPtrEntryMap_T runningEntries_; /// Running Db Entries 
        candidateEntries_->profiles_.insert( make_pair( profileId, newProf ) );
    }
}

// ---------------------------------------------------------------------------|
void DeviceModuleFixture::ensureStreamConnectionExists( 
        const uint16_t profileId,
        const uint16_t connectionId )
{
    ensureProfileExists( profileId );

    if ( candidateEntries_->profiles_[ profileId ].streamConnections_.end() == 
         candidateEntries_->profiles_[ profileId ].streamConnections_.find
                ( connectionId ) )
    {
        StreamConnectionItem conn;
        conn.id_ = connectionId;
        candidateEntries_->profiles_[ profileId ].streamConnections_.insert(
                make_pair( connectionId, conn ) );
    }
}

// ---------------------------------------------------------------------------|
void DeviceModuleFixture::ensureProfileStreamExists( 
        const uint16_t profileId,
        const uint16_t streamId )
{
    ensureProfileExists( profileId );

    if ( candidateEntries_->profiles_[ profileId ].streams_.end() == 
         candidateEntries_->profiles_[ profileId ].streams_.find( streamId ) )
    {
        StreamItem newStream;
        newStream.id_ = streamId;

        candidateEntries_->profiles_[ profileId ].streams_.insert( 
                make_pair( streamId, newStream ) );
    }
}

// ---------------------------------------------------------------------------|
void DeviceModuleFixture::ensureProfileStreamResourceExists( 
        const uint16_t profileId,
        const uint16_t streamId, 
        const uint16_t resourceId )
{
    ensureProfileStreamExists( profileId, streamId );

    if ( candidateEntries_->profiles_[ profileId ]
         .streams_[streamId].resourceDescription_.end() == 
         candidateEntries_->profiles_[ profileId ]
         .streams_[streamId].resourceDescription_.find( resourceId ) )
    {
        ResourceNode res;
        res.id_ = resourceId;

        candidateEntries_->profiles_[ profileId ].streams_[ streamId ]
            .resourceDescription_.insert( make_pair( resourceId, res ) );
    }
}

// ---------------------------------------------------------------------------|
void DeviceModuleFixture::ensureProfileStreamVRConnectionExists( 
        const uint16_t profileId,
        const uint16_t streamId, 
        const uint16_t connectionId )
{
    ensureProfileStreamExists( profileId, streamId );

    if ( candidateEntries_->profiles_[ profileId ]
         .streams_[streamId].resourceConnections_.end() == 
         candidateEntries_->profiles_[ profileId ]
         .streams_[streamId].resourceConnections_.find( connectionId ) )
    {
        ConnectionItem conn;
        conn.id_ = connectionId;
        candidateEntries_->profiles_[ profileId ].streams_[ streamId ]
             .resourceConnections_.insert( make_pair( connectionId, conn ) );
    }
}

// ---------------------------------------------------------------------------|
void DeviceModuleFixture::runEditQuery( 
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
void DeviceModuleFixture::createXpoContainer(
    std::shared_ptr<AbstractNCSession> session )
{
    string query = xpoBuilder_.genXPOQuery( "create" );
    runEditQuery( session, query );
}

// ---------------------------------------------------------------------------|
void DeviceModuleFixture::deleteXpoContainer(
    std::shared_ptr<AbstractNCSession> session,
    const bool commitChange )
{
    string query = xpoBuilder_.genXPOQuery( "delete" );
    runEditQuery( session, query );

    candidateEntries_->clear();
    if ( commitChange )
    {
        commitChanges( session );
    }
}

// ---------------------------------------------------------------------------|
void DeviceModuleFixture::createXpoProfile(
    std::shared_ptr<AbstractNCSession> session,
    const uint16_t profileId )
{
    string query = xpoBuilder_.genProfileQuery( profileId, "create" );
    runEditQuery( session, query );

    ensureProfileExists( profileId );
}

// ---------------------------------------------------------------------------|
void DeviceModuleFixture::setActiveProfile(
    std::shared_ptr<AbstractNCSession> session,
    const uint16_t profileId )
{
    string query = xpoBuilder_.genSetActiveProfileIdQuery( profileId, "replace" );
    runEditQuery( session, query );

    // @TODO: add check for failure if the selected profile does not
    // @TODO: exist, when this is supported by the real database.
    candidateEntries_->activeProfile_ = profileId;
}

// ---------------------------------------------------------------------------|
void DeviceModuleFixture::mergeActiveProfile(
    std::shared_ptr<AbstractNCSession> session,
    const uint16_t profileId )
{
    string query = xpoBuilder_.genSetActiveProfileIdQuery( profileId, "merge" );
    runEditQuery( session, query );

    // @TODO: add check for failure if the selected profile does not
    // @TODO: exist, when this is supported by the real database.
    candidateEntries_->activeProfile_ = profileId;
}

// ---------------------------------------------------------------------------|
void DeviceModuleFixture::createProfileStreamItem(
    std::shared_ptr<AbstractNCSession> session,
    const uint16_t profileId,
    const uint16_t streamId )
{
    string query = xpoBuilder_.genProfileStreamItemQuery( profileId, streamId, "create" );
    runEditQuery( session, query );

    ensureProfileStreamExists( profileId, streamId );
}

// ---------------------------------------------------------------------------|
void DeviceModuleFixture::createStreamItemResourceNode( 
    std::shared_ptr<AbstractNCSession> session,
    const uint16_t profileId,
    const uint16_t streamId,
    const uint16_t resourceNodeId )
{
    string query = xpoBuilder_.genProfileChildQuery( profileId, streamId,
            "resourceNode", resourceNodeId, "create" );
    runEditQuery( session, query );

    ensureProfileStreamResourceExists( profileId, streamId, resourceNodeId );
}

// ---------------------------------------------------------------------------|
void DeviceModuleFixture::createVRConnectionNode( 
    std::shared_ptr<AbstractNCSession> session,
    const uint16_t profileId,
    const uint16_t streamId,
    const uint16_t connectionId )
{
    string query = xpoBuilder_.genProfileChildQuery( profileId, streamId,
            "resourceConnection", connectionId, "create" );
    runEditQuery( session, query );

    ensureProfileStreamVRConnectionExists( profileId, streamId, connectionId );

}

// ---------------------------------------------------------------------------|
void DeviceModuleFixture::configureVResource( 
    std::shared_ptr<AbstractNCSession> session,
    const uint16_t profileId,
    const uint16_t streamId,
    const uint16_t resourceId,
    const ResourceNodeConfig& config )
{
    string query =  xpoBuilder_.configureVResourceNode( config, "create" );
    query = xpoBuilder_.addResourceNodePath( profileId, streamId, resourceId, 
            query );
    runEditQuery( session, query );

    ensureProfileStreamResourceExists( profileId, streamId, resourceId );
    if ( config.resourceType_ )
    {
        candidateEntries_->profiles_[ profileId ].streams_[ streamId ]
            .resourceDescription_[ resourceId ].resourceType_ = *config.resourceType_;
    }

    if ( config.physicalPath_ )
    {
        candidateEntries_->profiles_[ profileId ].streams_[ streamId ]
            .resourceDescription_[ resourceId ].physicalPath_ = *config.physicalPath_;
    }
}

// ---------------------------------------------------------------------------|
template< class ConnectionItemType, class ConfigType >
void DeviceModuleFixture::storeConnectionItemConfig( 
    ConnectionItemType& connectionItem, 
    const ConfigType& config )
{
    if ( config.sourceId_ )
    {
        connectionItem.sourceId_ = *config.sourceId_;
    }

    if ( config.sourcePinId_ )
    {
        connectionItem.sourcePinId_ = *config.sourcePinId_;
    }

    if ( config.destinationId_ )
    {
        connectionItem.destinationId_ = *config.destinationId_;
    }

    if ( config.destinationPinId_ )
    {
        connectionItem.destinationPinId_ = *config.destinationPinId_;
    }

    if ( config.bitrate_ )
    {
        connectionItem.bitrate_ = *config.bitrate_;
    }
}

// ---------------------------------------------------------------------------|
void DeviceModuleFixture::configureVRConnection( 
    std::shared_ptr<AbstractNCSession> session,
    const uint16_t profileId,
    const uint16_t streamId,
    const uint16_t connectionId,
    const ConnectionItemConfig& config )
{
    string query =  xpoBuilder_.configureResourceConnection( config, "create" );
    query = xpoBuilder_.addVRConnectionNodePath( profileId, streamId, 
            connectionId, query );
    runEditQuery( session, query );

    ensureProfileStreamVRConnectionExists( profileId, streamId, connectionId );
    storeConnectionItemConfig( candidateEntries_->profiles_[ profileId ]
            .streams_[ streamId ].resourceConnections_[connectionId]
            , config );
}

// ---------------------------------------------------------------------------|
void DeviceModuleFixture::configureStreamConnection( 
    std::shared_ptr<AbstractNCSession> session,
    const uint16_t profileId,
    const uint16_t connectionId,
    const StreamConnectionItemConfig& config )
{
    string query =  xpoBuilder_.configureStreamConnection( config, "create" );
    query = xpoBuilder_.addStreamConnectionPath( profileId, connectionId, query );
    runEditQuery( session, query );

    ensureStreamConnectionExists( profileId, connectionId );

    StreamConnectionItem& connItem = 
        candidateEntries_->profiles_[ profileId ].streamConnections_[connectionId];

    storeConnectionItemConfig( connItem, config );

    if ( config.sourceStreamId_ )
    {
        connItem.sourceStreamId_ = *config.sourceStreamId_;
    }

    if ( config.destinationStreamId_ )
    {
        connItem.destinationStreamId_ = *config.destinationStreamId_;
    }
}

// ---------------------------------------------------------------------------|
void DeviceModuleFixture::createStreamConnection(
    std::shared_ptr<AbstractNCSession> session,
    const uint16_t profileId,
    const uint16_t streamId )
{
    string query = xpoBuilder_.genStreamConnectionQuery( profileId, 
            streamId, "create" );
    runEditQuery( session, query );

    ensureStreamConnectionExists( profileId, streamId );
}

// ---------------------------------------------------------------------------|
void DeviceModuleFixture::commitChanges(
    std::shared_ptr<AbstractNCSession> session )
{
    BaseSuiteFixture::commitChanges( session );

    if ( useCandidate() )
    {
        *runningEntries_ = *candidateEntries_;
    }
}

// ---------------------------------------------------------------------------|
void DeviceModuleFixture::discardChanges()
{
    if ( useCandidate() )
    {
        *candidateEntries_ = *runningEntries_;
    }
}

// ---------------------------------------------------------------------------|
void DeviceModuleFixture::checkCandidate()
{
    XMLContentChecker<XPO3Container> checker( candidateEntries_ );
    queryEngine_->tryGetConfigXpath( primarySession_, "xpo", 
            writeableDbName_, checker );
}

// ---------------------------------------------------------------------------|

void DeviceModuleFixture::checkRunning()
{
    XMLContentChecker<XPO3Container> checker( runningEntries_ );
    queryEngine_->tryGetConfigXpath( primarySession_, "xpo", 
            "running", checker );
}
// ---------------------------------------------------------------------------|

} // namespace YumaTest
