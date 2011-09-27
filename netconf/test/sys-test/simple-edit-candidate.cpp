#define BOOST_TEST_MODULE SysTestSimpleEditCandidate

#include "configure-yuma-systest.h"

namespace YumaTest {

// ---------------------------------------------------------------------------|
// Initialise the spoofed command line arguments 
// ---------------------------------------------------------------------------|
const char* SpoofedArgs::argv[] = {
    ( "yuma-test" ),
    ( "--target=candidate" ),
    //( "--no-startup" ),         // ensure that no configuration from previous 
                                // tests is present
};

#include "define-yuma-systest-global-fixture.h"

} // namespace YumaTest
