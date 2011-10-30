// ---------------------------------------------------------------------------|
// Test Harness includes
// ---------------------------------------------------------------------------|
#include "test/support/callbacks/running-cb-checker.h"

// ---------------------------------------------------------------------------|
// File wide namespace use
// ---------------------------------------------------------------------------|
using namespace std;

// ---------------------------------------------------------------------------|
namespace YumaTest
{

// ---------------------------------------------------------------------------!
RunningCBChecker::RunningCBChecker() 
   : CallbackChecker()
{
}

// ---------------------------------------------------------------------------!
RunningCBChecker::~RunningCBChecker()
{
}

// ---------------------------------------------------------------------------|
void RunningCBChecker::addKey(const std::string& modName, 
                              const std::string& containerName,
                              const std::vector<std::string>& listElement,
                              const std::string& key)
{
    vector<string> elements(listElement);
    addExpectedCallback(modName, containerName, elements, "edit", "validate", "");
    elements.push_back(key); 
    addExpectedCallback(modName, containerName, elements, "edit", "validate", "");
    elements.pop_back(); 
    addExpectedCallback(modName, containerName, elements, "edit", "apply", "");
    elements.push_back(key); 
    addExpectedCallback(modName, containerName, elements, "edit", "apply", "");
    elements.pop_back(); 
    addExpectedCallback(modName, containerName, elements, "edit", "commit", "create");
    elements.push_back(key); 
    addExpectedCallback(modName, containerName, elements, "edit", "commit", "create");
}

// ---------------------------------------------------------------------------|
void RunningCBChecker::addKeyValuePair(const std::string& modName, 
                                       const std::string& containerName,
                                       const std::vector<std::string>& listElement,
                                       const std::string& key,
                                       const std::string& value)
{
    vector<string> elements(listElement);
    addExpectedCallback(modName, containerName, elements, "edit", "validate", "");
    elements.push_back(key); 
    addExpectedCallback(modName, containerName, elements, "edit", "validate", "");
    elements.pop_back(); 
    addExpectedCallback(modName, containerName, elements, "edit", "apply", "");
    elements.push_back(key); 
    addExpectedCallback(modName, containerName, elements, "edit", "apply", "");
    elements.pop_back(); 
    addExpectedCallback(modName, containerName, elements, "edit", "commit", "create");
    elements.push_back(key); 
    addExpectedCallback(modName, containerName, elements, "edit", "commit", "create");
    elements.pop_back(); 
    elements.push_back(value); 
    addExpectedCallback(modName, containerName, elements, "edit", "validate", "");
    addExpectedCallback(modName, containerName, elements, "edit", "apply", "");
    addExpectedCallback(modName, containerName, elements, "edit", "commit", "create");
}

// ---------------------------------------------------------------------------|
void RunningCBChecker::commitKeyValuePairs(const std::string& modName, 
                                           const std::string& containerName,
                                           const std::vector<std::string>& listElement,
                                           const std::string& key,
                                           const std::string& value,
                                           int count)
{
    // Do nothing as commits are not performed separately when using writeable running.
}

// ---------------------------------------------------------------------------|
void RunningCBChecker::deleteKey(const std::string& modName, 
                                 const std::string& containerName,
                                 const std::vector<std::string>& listElement,
                                 const std::string& key)
{
    vector<string> elements(listElement);
    addExpectedCallback(modName, containerName, elements, "edit", "validate", "");
    elements.push_back(key); 
    addExpectedCallback(modName, containerName, elements, "edit", "validate", "");
    elements.pop_back(); 
    addExpectedCallback(modName, containerName, elements, "edit", "apply", "");

    //TODO - Add commit callbacks
}

// ---------------------------------------------------------------------------|
void RunningCBChecker::deleteKeyValuePair(const std::string& modName, 
                                          const std::string& containerName,
                                          const std::vector<std::string>& listElement,
                                          const std::string& key,
                                          const std::string& value)
{
    vector<string> elements(listElement);
    elements.push_back(value); 
    addExpectedCallback(modName, containerName, elements, "edit", "validate", "");
    addExpectedCallback(modName, containerName, elements, "edit", "apply", "");
    addExpectedCallback(modName, containerName, elements, "edit", "commit", "delete");
    elements.pop_back(); 
    addExpectedCallback(modName, containerName, elements, "edit", "validate", "");
    elements.push_back(key); 
    addExpectedCallback(modName, containerName, elements, "edit", "validate", "");
    elements.pop_back(); 
    addExpectedCallback(modName, containerName, elements, "edit", "apply", "");
    addExpectedCallback(modName, containerName, elements, "edit", "commit", "delete");
}

// ---------------------------------------------------------------------------|
void RunningCBChecker::updateLeaf(const std::string& modName, 
                const std::string& containerName,
                const std::vector<std::string>& listElement,
                const std::string& phase)
{
    addExpectedCallback(modName, containerName, listElement, "edit", "validate", "");
    addExpectedCallback(modName, containerName, listElement, "edit", "apply", "");
    addExpectedCallback(modName, containerName, listElement, "edit", "commit", phase);
}

// ---------------------------------------------------------------------------|
void RunningCBChecker::updateContainer(const std::string& modName, 
                                         const std::string& containerName,
                                         const std::vector<std::string>& listElement,
                                         const std::string& phase)
{
    addExpectedCallback(modName, containerName, listElement, "edit", "validate", "");
    addExpectedCallback(modName, containerName, listElement, "edit", "apply", "");
    addExpectedCallback(modName, containerName, listElement, "edit", "commit", phase);
}

} // namespace YumaTest
