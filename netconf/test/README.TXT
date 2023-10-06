The Legacy Yuma Test Harness
===============================================================================

ATTENTION: This file is relevant for the libboost-test based test framework
which is the legacy test system which manages the testcases in test/integ-tests
and test/sys-tests.

The majority of testcases that are actively used are under test/netconfd and
test/yangcli and do not use libboost-test. They are following the Autotools
test suite script based convention and launched with the `make test` command.

If you are interested in the legacy Yuma Test Harness keep reading.

1: Pre-requisites:
    Operating System	Ubuntu 10.04 or later	
    Compiler	        GCC 4.4 or later	
    Boost	        Boost 1.47 or later
    Lib Xml 	        libxml2 / libxml2-dev
    Python	        2.6 Python interpreter
    Libbz2-dev	        1.05-4ubuntu0.1	
    Doxygen		Source code documentation tool
    Graphviz		Graphical Visualisation Tool used by Doxygen
    Texlive-font-utils  Text formatting used by doxygen
    Libncursesw5	5.7 Shared libraries for terminal handling
    libssh2-1-dev	1.2.2-1 SSH2 ssh2 client-side libary

2: The Integration Test Harness

The Integration test harness is built as a stand alone executable that 
includes most of the Yuma agt and ncx sources (which make up the system 
under test). This test harness runs on the local host machine and can be
 used to quickly verify the behaviour of netconfd.

To build and run this test harness:
    1: cd to netconf/test
    2: make
    3: sudo make install
    4: cd integ-tests
    5: `mkdir yuma-op` directory used to store output
    6: make check

Note: tests can be run individually, by specifying the name of the
test suite and test on the command line as follows:
    ./test-simple-edit-running \
        --run_test=simple_get_test_suite_running/edit_slt_entries_test_delete

For more information on test harness command line options see:
    http://www.boost.org/doc/libs/1_47_0/libs/test/doc/html/utf/user-guide/runtime-config.html

3: The System Test Harness

The System test harness is a stand alone program that is capable of 
running full NETCONF sessions against a full Yuma/Netconf Server
(the system under test). The System test harness provides a fast 
way of verifying the behaviour of a full Yuma/Netconf system.
It behaves in the same way as a real Netconf client. To use 
this test harness the Netconf Server must have the appropriate
YANG modules installed. The SIL modules for the YANG libraries
can be loaded, but this is not required.

To make sure the proper modules get loaded, either configure
the netconfd --modpath parameter, set the YUMA_MODPATH environment
variable, or put the YANG files a directory called 'modules' in
your HOME directory.

To build and run this test harness:
    1: cd netconf/test
    2: make
    3: sudo make install
    5: start the server and make sure at least these flags are set:
       --module=device_test
       --module=simple_list_test
       --module=simple_yang_test
       --access-control=off OR --superuser=<user>
       --target=candidate OR --target=running
       --no-startup
    6: cd sys-test
    7: `mkdir yuma-op` directory used to store output
    8: Configure the session variables:
       export YUMA_AGENT_IPADDR=127.0.0.1
       export YUMA_AGENT_PORT=830
       export YUMA_AGENT_USER=root
       export YUMA_AGENT_PASSWORD=****
    9: run either all tests with `make check` or individual tests e.g. ./test-shutdown


