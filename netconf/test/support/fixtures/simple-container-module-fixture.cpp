// ---------------------------------------------------------------------------|
// Test Harness includes
// ---------------------------------------------------------------------------|
#include "test/support/fixtures/simple-container-module-fixture.h"

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
#include "test/support/checkers/string-presence-checkers.h"

// ---------------------------------------------------------------------------|
// File wide namespace use
// ---------------------------------------------------------------------------|
using namespace std;

// ---------------------------------------------------------------------------|
// TODO: Much of the fuctionality in this file will probably be needed
// TODO: by other tests - place it in a base class....
namespace 
{
// ---------------------------------------------------------------------------|

// ---------------------------------------------------------------------------|
/**
 * Simple utility functor that converts the supplied pair into xml NVP
 * strings and stores them in the supplied vector. This functor is
 * used to generate a list of values that should be present in
 * database content query replies.
 */
struct EntriesConvertFunctor
{
    /** 
     * constructor.
     *
     * \param target reference to the output vector of strings.
     */
    explicit EntriesConvertFunctor( vector<string>& target )
        : target_( target )
    {
    }

    /** 
     * Convert the supplied pair into a 2 xmNVP strings.
     *
     * \param val the map entry to convert.tags
     */
    void operator()( const std::pair<string, string>& val )
    {
        target_.push_back( "<theKey>" + val.first + "</theKey>" );
        target_.push_back( "<theVal>" + val.second + "</theVal>" );
    }

    /** reference to the ouput string vector. */
    vector<string>& target_;
};

// ---------------------------------------------------------------------------|
/**
 * Simple utility functor that converts any values not present in the
 * reference map into xml NVP strings and stores them in the supplied vector. 
 * This functor is used to generate a list of values that should NOT be present 
 * in database content query replies.
 */
struct AddDifferingEntriesFunctor
{
    /** Convenience Typedef */
    typedef YumaTest::SimpleContainerModuleFixture::SharedPtrEntryMap_T 
            SharedPtrEntryMap_T;
    /**
     * Constructor
     *
     * \param target reference to the output vector of strings.
     * \param refMap the reference map of values that should be present.
     */
    AddDifferingEntriesFunctor( vector<string>& target, 
                                const SharedPtrEntryMap_T refMap )
        : target_( target )
        , refMap_( refMap )
    {
    }

    /** 
     * Check if the supplied value is in the reference map. If it is
     * not format an xml NVP string and store it.
     *
     * \param val the map entry to check.
     */
    void operator()( const std::pair<string, string>& val )
    {
        auto pos = refMap_->find( val.first );
        if ( pos == refMap_->end() )
        {
            target_.push_back( "<theKey>" + val.first + "</theKey>"  );
        }
        else if ( pos->second != val.second )
        {
            target_.push_back( "<theVal>" + val.second + "</theVal>" );
        }
    }

    vector<string>& target_;           /// ref to the output string vector.
    const SharedPtrEntryMap_T refMap_; /// the reference map. 
};

}

// ---------------------------------------------------------------------------|
namespace YumaTest 
{

// ---------------------------------------------------------------------------|
SimpleContainerModuleFixture::SimpleContainerModuleFixture() 
    : BaseSuiteFixture()
    , wrBuilder_()
    , moduleName_( "simple_list_test" )
    , moduleNs_( "http://netconfcentral.org/ns/simple_list_test" )
    , containerName_( "simple_list" )
    , runningEntries_( new EntryMap_T() )
    , candidateEntries_( useCandidate() ? SharedPtrEntryMap_T( new EntryMap_T() )
                                        : runningEntries_ )
{
    // ensure the module is loaded
    queryEngine_->loadModule( primarySession_, "simple_list_test" );
}

// ---------------------------------------------------------------------------|
SimpleContainerModuleFixture::~SimpleContainerModuleFixture() 
{
}

// ---------------------------------------------------------------------------|
void SimpleContainerModuleFixture::runEditQuery( 
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
void SimpleContainerModuleFixture::createMainContainer(
    std::shared_ptr<AbstractNCSession> session )
{
    assert( session );
    string query = wrBuilder_.genTopLevelContainerText( 
            containerName_, moduleNs_, "create" );
    runEditQuery( session, query );
}

// ---------------------------------------------------------------------------|
void SimpleContainerModuleFixture::deleteMainContainer(
    std::shared_ptr<AbstractNCSession> session )
{
    string query = wrBuilder_.genTopLevelContainerText( 
            containerName_, moduleNs_, "delete" );
    runEditQuery( session, query );

    candidateEntries_->clear();
}

// ---------------------------------------------------------------------------|
void SimpleContainerModuleFixture::addEntry(
    std::shared_ptr<AbstractNCSession> session,
    const string& entryKeyStr )
{
    assert( session );

    string query = wrBuilder_.genModuleOperationText( containerName_, moduleNs_,
            wrBuilder_.genKeyOperationText( "theList", "theKey", entryKeyStr, "create" ) );
    runEditQuery( session, query );
}

// ---------------------------------------------------------------------------|
void SimpleContainerModuleFixture::addEntryValue(
    std::shared_ptr<AbstractNCSession> session,
    const string& entryKeyStr,
    const string& entryValStr )
{
    assert( session );

    string query = wrBuilder_.genModuleOperationText( containerName_, moduleNs_,
            wrBuilder_.genKeyParentPathText( "theList", "theKey", entryKeyStr,
                wrBuilder_.genOperationText( "theVal", entryValStr, "create" ) ) );
    
    runEditQuery( session, query );
}

// ---------------------------------------------------------------------------|
void SimpleContainerModuleFixture::addEntryValuePair(
    std::shared_ptr<AbstractNCSession> session,
    const string& entryKeyStr,
    const string& entryValStr )
{
    assert( session );
    addEntry( session, entryKeyStr );
    addEntryValue( session, entryKeyStr, entryValStr );

    (*candidateEntries_)[ entryKeyStr ] = entryValStr;
}

// ---------------------------------------------------------------------------|
void SimpleContainerModuleFixture::populateDatabase(
    const uint16_t numEntries )
{
    string baseKeyText( "entryKey" );
    string baseValText( "entryVal" );

    for ( uint16_t cnt=0; cnt < numEntries; ++cnt )
    {
        addEntryValuePair( primarySession_, 
                           baseKeyText+boost::lexical_cast<string>( cnt ),
                           baseValText+boost::lexical_cast<string>( cnt ) );
    }
}

// ---------------------------------------------------------------------------|
void SimpleContainerModuleFixture::editEntryValue(
    std::shared_ptr<AbstractNCSession> session,
    const string& entryKeyStr,
    const string& entryValStr )
{
    assert(session);
    string query = wrBuilder_.genModuleOperationText( containerName_, moduleNs_,
            wrBuilder_.genKeyParentPathText( "theList", "theKey", entryKeyStr,
                wrBuilder_.genOperationText( "theVal", entryValStr, "replace" ) ) );
    

    runEditQuery( session, query );

    (*candidateEntries_)[ entryKeyStr ] = entryValStr;
}

// ---------------------------------------------------------------------------|
void SimpleContainerModuleFixture::checkEntriesImpl(
    std::shared_ptr<AbstractNCSession> session,
    const string& targetDbName,
    const SharedPtrEntryMap_T refMap,
    const SharedPtrEntryMap_T otherMap )
{

    vector<string> expPresent{ "data" };
    vector<string> expNotPresent{ "error", "rpc-error" };

    // first check the candidate database
    //
    // Add all expected elements to expPresent
    EntriesConvertFunctor conv( expPresent );
    for_each( refMap->begin(), refMap->end(), conv );

    // now add all elements from the running that are not in the
    // candidate to the expectedNotPresent list
    if ( useCandidate() )
    {
        AddDifferingEntriesFunctor diff( expNotPresent, refMap );
        for_each( otherMap->begin(), otherMap->end(), diff );
    }

    StringsPresentNotPresentChecker checker( expPresent, expNotPresent );
    queryEngine_->tryGetConfigXpath( session, containerName_, targetDbName, 
                                     checker );
}

// ---------------------------------------------------------------------------|
void SimpleContainerModuleFixture::checkEntries(
    std::shared_ptr<AbstractNCSession> session)
{
    assert( session );

    if ( useCandidate() )
    {
        checkEntriesImpl( session, writeableDbName_, candidateEntries_, 
                          runningEntries_ );
    }
    
    checkEntriesImpl( session, "running", runningEntries_, candidateEntries_ );
}

// ---------------------------------------------------------------------------|
void SimpleContainerModuleFixture::commitChanges(
    std::shared_ptr<AbstractNCSession> session )
{
    BaseSuiteFixture::commitChanges( session );
    
    // copy the editted changes to the running changes
    if ( useCandidate() )
    {
        runningEntries_->clear();
        runningEntries_->insert( candidateEntries_->begin(), 
                                candidateEntries_->end() );
    }

}
// ---------------------------------------------------------------------------|
void SimpleContainerModuleFixture::discardChanges()
{
    // copy the editted changes to the running changes
    if ( useCandidate() )
    {
        candidateEntries_->clear();
        candidateEntries_->insert( runningEntries_->begin(), 
                                   runningEntries_->end() );
    }
}

} // namespace YumaTest
