// ---------------------------------------------------------------------------|
// Test Harness includes
// ---------------------------------------------------------------------------|
#include "test/support/callbacks/candidate-cb-checker.h"

// ---------------------------------------------------------------------------|
// File wide namespace use
// ---------------------------------------------------------------------------|
using namespace std;

// ---------------------------------------------------------------------------|
namespace YumaTest
{

// ---------------------------------------------------------------------------|
void CandidateCBChecker::addKey(const std::string& modName, 
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
}

// ---------------------------------------------------------------------------|
void CandidateCBChecker::addKeyValuePair(const std::string& modName, 
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
    elements.push_back(value); 
    addExpectedCallback(modName, containerName, elements, "edit", "validate", "");
    addExpectedCallback(modName, containerName, elements, "edit", "apply", "");
}

// ---------------------------------------------------------------------------|
void CandidateCBChecker::commitKeyValuePairs(const std::string& modName, 
                                             const std::string& containerName,
                                             const std::vector<std::string>& listElement,
                                             const std::string& key,
                                             const std::string& value,
                                             int count)
{
    vector<string> elements;
    addExpectedCallback(modName, containerName, elements, "edit", "apply", "");
    for (int i = 0; i < count; i++)
    {
        elements = listElement;
        addExpectedCallback(modName, containerName, elements, "edit", "apply", "");
        elements.push_back(key);
        addExpectedCallback(modName, containerName, elements, "edit", "apply", "");
        elements.pop_back();
        elements.push_back(value);
        addExpectedCallback(modName, containerName, elements, "edit", "apply", "");
    }
    elements.clear(); 
    addExpectedCallback(modName, containerName, elements, "edit", "commit", "create");
    for (int i = 0; i < count; i++)
    {
        elements = listElement;
        addExpectedCallback(modName, containerName, elements, "edit", "commit", "create");
        elements.push_back(key);
        addExpectedCallback(modName, containerName, elements, "edit", "commit", "create");
        elements.pop_back();
        elements.push_back(value);
        addExpectedCallback(modName, containerName, elements, "edit", "commit", "create");
    }
}

// ---------------------------------------------------------------------------|
void CandidateCBChecker::deleteKey(const std::string& modName, 
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
}

// ---------------------------------------------------------------------------|
void CandidateCBChecker::deleteKeyValuePair(const std::string& modName, 
                                            const std::string& containerName,
                                            const std::vector<std::string>& listElement,
                                            const std::string& key,
                                            const std::string& value)
{
    vector<string> elements(listElement);
    elements.push_back(value); 
    addExpectedCallback(modName, containerName, elements, "edit", "validate", "");
    addExpectedCallback(modName, containerName, elements, "edit", "apply", "");
    elements.pop_back(); 
    addExpectedCallback(modName, containerName, elements, "edit", "validate", "");
    elements.push_back(key); 
    addExpectedCallback(modName, containerName, elements, "edit", "validate", "");
    elements.pop_back(); 
    addExpectedCallback(modName, containerName, elements, "edit", "apply", "");
}

// ---------------------------------------------------------------------------|
void CandidateCBChecker::updateLeaf(const std::string& modName, 
                const std::string& containerName,
                const std::vector<std::string>& listElement,
                const std::string& phase)
{
    addExpectedCallback(modName, containerName, listElement, "edit", "validate", "");
    addExpectedCallback(modName, containerName, listElement, "edit", "apply", "");
}

// ---------------------------------------------------------------------------|
void CandidateCBChecker::updateContainer(const std::string& modName, 
                                         const std::string& containerName,
                                         const std::vector<std::string>& listElement,
                                         const std::string& phase)
{
    addExpectedCallback(modName, containerName, listElement, "edit", "validate", "");
    addExpectedCallback(modName, containerName, listElement, "edit", "apply", "");
}

} // namespace YumaTest
