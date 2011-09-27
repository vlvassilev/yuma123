// ---------------------------------------------------------------------------|
// Test Harness includes
// ---------------------------------------------------------------------------|
#include "test/support/callbacks/callback-checker.h"

// ---------------------------------------------------------------------------|
// Standard includes
// ---------------------------------------------------------------------------|
#include <boost/foreach.hpp>

// ---------------------------------------------------------------------------|
// Boost Test Framework
// ---------------------------------------------------------------------------|
#include <boost/test/unit_test.hpp>

// ---------------------------------------------------------------------------|
// File wide namespace use
// ---------------------------------------------------------------------------|
using namespace std;

// ---------------------------------------------------------------------------|
namespace YumaTest
{

// ---------------------------------------------------------------------------|
void CallbackChecker::addExpectedCallback(const string& modName,
                                          const string& containerName,
                                          vector<string> elementHierarchy,
                                          const string& operation,
                                          const string& type,
                                          const string& phase)
{
    //TODO Currently ignores commit callbacks to allow test suites to
    //TODO work on running or candidate dbs.
    //TODO Create hierarchy to deal with these differences as well as not 
    //TODO check callbacks for system tests.
    if (phase != "")
        return;

    string name = "y_" + modName + "_" + containerName + "_"; 
    BOOST_FOREACH (string& val, elementHierarchy)
    {
        name += val + "_";
    }
    name += operation;

    SILCallbackLog::CallbackInfo cbInfo;
    cbInfo.cbName = name;
    cbInfo.cbType = type;
    cbInfo.cbPhase = phase;

    expectedCallbacks_.push_back(cbInfo);
}
    
// ---------------------------------------------------------------------------|
void CallbackChecker::checkCallbacks(const std::string& modName)
{
    SILCallbackLog& cbLog = SILCallbackLog::getInstance();
    SILCallbackLog::ModuleCallbackData actualCallbacks = cbLog.getModuleCallbacks(modName);

    //TODO Currently ignores commit callbacks to allow test suites to
    //TODO work on running or candidate dbs.
    //TODO Create hierarchy to deal with these differences as well as not 
    //TODO check callbacks for system tests.
    int commits = 0;
    SILCallbackLog::ModuleCallbackData::iterator it;
    for(it = actualCallbacks.begin(); it != actualCallbacks.end(); ++it)
    {
        if (it->cbPhase != "")
            ++commits;
    }

    SILCallbackLog::ModuleCallbackData::iterator it_act, it_exp;
    for(it_act = actualCallbacks.begin(), it_exp = expectedCallbacks_.begin();
        it_act != actualCallbacks.end() && it_exp !=  expectedCallbacks_.end();
        ++it_act, ++it_exp)
    {
        //TODO Currently ignores commit callbacks.
        if (it_act->cbPhase != "")
        {
            ++it_act;
            continue;
        }

        BOOST_CHECK(*it_act == *it_exp);    
    } 

    //TODO Currently ignores commit callbacks.
    BOOST_CHECK_MESSAGE(actualCallbacks.size() - commits <= expectedCallbacks_.size(),
                        "Unexpected callbacks were logged"); 
    BOOST_CHECK_MESSAGE(actualCallbacks.size() - commits >= expectedCallbacks_.size(),
                        "Further callbacks were expected"); 
}

// ---------------------------------------------------------------------------|
void CallbackChecker::resetExpectedCallbacks()
{
    expectedCallbacks_.clear();
}

} // namespace YumaTest
