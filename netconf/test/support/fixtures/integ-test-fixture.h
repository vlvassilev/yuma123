#ifndef __INTEG_TEST_FIXTURE__H
#define __INTEG_TEST_FIXTURE__H

#include "test/support/fixtures/abstract-global-fixture.h"
#include "test/support/nc-query-util/yuma-op-policies.h"
#include "test/support/fixtures/test-context.h"

namespace YumaTest 
{

// ---------------------------------------------------------------------------|
/**
 * This class is used to perform all global initialisation and teardown 
 * of Yuma. If any errors occur or are reported during initialisation
 * the test is terminated with an assertion.
 *
 * BOOST::TEST Fixtures do not have a way of configuring any internal
 * data at runtime. To allow configuration of the test fixture class 
 * all configuration data is passed compile time as a template
 * parameter.
 *
 * \tparam ArgTraits a structure containing any command line arguments the 
 *                   test harness should use to configure neconf.
 * \tparam YumaQueryOpLogPolicy the policy for generating Yuma OP log
 *                   filenames, which are written whenever the test harness 
 *                   injects a message into yuma.
 */
template < class SpoofedArgs, 
           class OpPolicy = MessageIdLogFilenamePolicy >
class IntegrationTestFixture : public AbstractGlobalTestFixture
{
   typedef SpoofedArgs Args;

public:
    /** 
     * Constructor. Initialise Yuma for system testing. This replaces
     * the functionality from cmn_init().
     */
    IntegrationTestFixture();

    /**
     * Destructor. Shutdown the Yuma system test environment.
     */
    ~IntegrationTestFixture();

private:
    /**
     * Configure the test context
     */
    void configureTestContext();

    /** 
     * Initialise the NCX engine. 
     * This function this simply calls ncx_init() and checks that it 
     * returned NO_ERR.  
     */
    void initialiseNCXEngine();

    /**
     * Load the base schemas.
     * This function loads the following base schemas:
     * <ul>
     *   <li>NCXMOD_YUMA_NETCONF - NETCONF data types and RPC methods.</li>
     *   <li>NCXMOD_NETCONFD - The netconf server boot parameter definition 
     *                        file.</li>
     * </ul>
     */
    void loadBaseSchemas();

    /**
     * Load the core schemas.
     * This function loads the following base schemas:
     * <ul>
     *   <li>NCXMOD_YUMA_NETCONF - NETCONF data types and RPC methods.</li>
     *   <li>NCXMOD_NETCONFD - The netconf server boot parameter definition 
     *                        file.</li>
     * </ul>
     */
    void loadCoreSchema();
    
    /**
     * Perform stage 1 Agt initialisation.
     * This function calls agt_init1 to perform stage 1 Agt initialisation.
     */
    void stage1AgtInitialisation();

    /**
     * Perform stage 2 Agt initialisation.
     * This function calls agt_init2 to perform stage 2 Agt initialisation.
     */
    void stage2AgtInitialisation();
};

} // namespace YumaTest

#endif // __INTEG_TEST_FIXTURE__H
