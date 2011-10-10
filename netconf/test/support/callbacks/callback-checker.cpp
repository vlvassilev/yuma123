// ---------------------------------------------------------------------------|
// Test Harness includes
// ---------------------------------------------------------------------------|
#include "test/support/callbacks/callback-checker.h"

// ---------------------------------------------------------------------------|
// Boost includes
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
                                          const vector<string>& elementHierarchy,
                                          const string& operation,
                                          const string& type,
                                          const string& phase)
{
    string name = modName + "_" + containerName + "_"; 
    BOOST_FOREACH (const string& val, elementHierarchy)
    {
        name += val + "_";
    }
    name += operation;

    SILCallbackLog::CallbackInfo cbInfo(name, type, phase);

    expectedCallbacks_.push_back(cbInfo);
}
    
// ---------------------------------------------------------------------------|
void CallbackChecker::checkCallbacks(const std::string& modName)
{
    SILCallbackLog& cbLog = SILCallbackLog::getInstance();
    SILCallbackLog::ModuleCallbackData actualCallbacks = cbLog.getModuleCallbacks(modName);

    SILCallbackLog::ModuleCallbackData::iterator it_act, it_exp;
    for(it_act = actualCallbacks.begin(), it_exp = expectedCallbacks_.begin();
        it_act != actualCallbacks.end() && it_exp !=  expectedCallbacks_.end();
        ++it_act, ++it_exp)
    {
        BOOST_CHECK(*it_act == *it_exp);    
    } 

    BOOST_CHECK_MESSAGE(actualCallbacks.size() <= expectedCallbacks_.size(),
                        "Unexpected callbacks were logged"); 
    BOOST_CHECK_MESSAGE(actualCallbacks.size() >= expectedCallbacks_.size(),
                        "Further callbacks were expected"); 
}

// ---------------------------------------------------------------------------|
void CallbackChecker::resetExpectedCallbacks()
{
    expectedCallbacks_.clear();
}

// ---------------------------------------------------------------------------|
void  CallbackChecker::resetModuleCallbacks(const std::string& modName)
{
    SILCallbackLog& cbLog = SILCallbackLog::getInstance();
    cbLog.resetModuleCallbacks(modName);
}

} // namespace YumaTest
