#!/usr/bin/env python

import time
import sys, os
sys.path.append("../../litenc")
import litenc
import litenc_lxml
import lxml
import argparse

DEBUG = False

rpc_discard_changes = """
<discard-changes xmlns="urn:ietf:params:xml:ns:netconf:base:1.0"/>
"""

rpc_get_config_with_defaults = """
  <get-config xmlns="urn:ietf:params:xml:ns:netconf:base:1.0">
    <source>
      <candidate/>
    </source>
    <with-defaults xmlns="urn:ietf:params:xml:ns:yang:ietf-netconf-with-defaults">
      report-all
    </with-defaults>
  </get-config>
"""

rpc_create_test_simple = """
  <edit-config xmlns="urn:ietf:params:xml:ns:netconf:base:1.0">
    <target>
      <candidate/>
    </target>
    <default-operation>merge</default-operation>
    <test-option>set</test-option>
    <config>
      <test-%s
        xmlns:nc="urn:ietf:params:xml:ns:netconf:base:1.0"
        nc:operation="create"
        xmlns="urn:labn:params:xml:ns:yang:test-leaf-list-defaults"/>
    </config>
  </edit-config>
"""

rpc_create_test_case = """
  <edit-config xmlns="urn:ietf:params:xml:ns:netconf:base:1.0">
    <target>
      <candidate/>
    </target>
    <default-operation>merge</default-operation>
    <test-option>set</test-option>
    <config>
      <test-%s xmlns="urn:labn:params:xml:ns:yang:test-leaf-list-defaults">
        <should-have-a-value
          xmlns:nc="urn:ietf:params:xml:ns:netconf:base:1.0"
          nc:operation="create">1</should-have-a-value>
      </test-%s>
    </config>
  </edit-config>
"""

rpc_create_test_data = """
  <edit-config xmlns="urn:ietf:params:xml:ns:netconf:base:1.0">
    <target>
      <candidate/>
    </target>
    <default-operation>merge</default-operation>
    <test-option>set</test-option>
    <config>
      <test-%s xmlns="urn:labn:params:xml:ns:yang:test-leaf-list-defaults">
        <data
          xmlns:nc="urn:ietf:params:xml:ns:netconf:base:1.0"
          nc:operation="create">11</data>
      </test-%s>
    </config>
  </edit-config>
"""

# TESTS = list-of-test-details = [ <test-details>, ... ]
# test-details = { 'RPC': <rpc-xml>,
#                  'edit-config-results': <list-of-xpath-expression>,
#                  'expected-results': <list-of-xpath-expressions>,
#                  'unexpected-results': <list-of-xpath-expressions>,
#                  'name': <str> }

tests = [
	{
		'RPC': rpc_create_test_simple % ('one'),
		'expected-results': [ '//test-one/data=3' ],
		'unexpected-results': [],
		'name': 'Test 1 - ancestors are non-presence containers',
		'edit-config-results': None
	},
	{
		'RPC': rpc_create_test_simple % ('two'),
		'expected-results': [ '//test-two/data=3' ],
		'unexpected-results': [],
		'name': 'Test 2 - one ancestor is presence container',
		'edit-config-results': None
	},
	{
		'RPC': rpc_create_test_simple % ('three'),
		'expected-results': [ '//test-three/data=3' ],
		'unexpected-results': [],
		'name': 'Test 3 - case is default without child nodes, has empty leaf-list',
		'edit-config-results': None
	},
	{
		'RPC': rpc_create_test_case % tuple(['four']*2),
		'expected-results': [ '//test-four/data=3' ],
		'unexpected-results': [],
		'name': 'Test 4 - case is default with child nodes, has empty leaf-list',
		'edit-config-results': None
	},
	{
		'RPC': rpc_create_test_case % tuple(['five']*2),
		'expected-results': [ '//test-five/data=3' ],
		'unexpected-results': [],
		'name': 'Test 5 - case has child node and is not default',
		'edit-config-results': None
	},
	{
		'RPC': rpc_create_test_simple % ('six'),
		'expected-results': ['//test-six'],
		'unexpected-results': [ '//test-six/data=3' ],
		'name': 'Test 6 - case has no child node and is not default',
		'edit-config-results': None
	},
	{
		'RPC': rpc_create_test_simple % ('seven'),
		'expected-results': ['//test-seven'],
		'unexpected-results': [ '//test-seven/data=3' ],
		'name': 'Test 7 - leaf-list if-feature evaluates to false',
		'edit-config-results': None
	},
	{
		'RPC': rpc_create_test_simple % ('eight'),
		'expected-results': ['//test-eight'],
		'unexpected-results': [ '//test-eight/data=3' ],
		'name': 'Test 8 - leaf-list "when" evaluates to false',
		'edit-config-results': None
	},
	{
		'RPC': rpc_create_test_simple % ('nine'),
		'expected-results': [],
		'unexpected-results': [ '//test-nine', '//test-nine/data=3' ],
		'name': 'Test 9 - ancestor has if-feature that evaluates to false',
		'edit-config-results': ['//rpc-error/error-info/bad-element'],
	},
	{
		'RPC': rpc_create_test_simple % ('ten'),
		'expected-results': [ '//test-ten/data=3', '//test-ten/data=4', '//test-ten/data=5' ],
		'unexpected-results': [],
		'name': 'Test 10 - leaf-list has multiple default values.',
		'edit-config-results': None
	},
	{
		'RPC': rpc_create_test_data % tuple(['eleven']*2),
		'expected-results': [ '//test-eleven/data=11' ],
		'unexpected-results': [ '//test-eleven/data=3',
		                        '//test-eleven/data=4',
		                        '//test-evelen/data=5' ],
		'name': 'Test 11 - leaf-list has multiple default values and _one_ configured value',
		'edit-config-results': None
	}
]

# Examine the return value of _XSLTResultTree.xpath() and decide if
# evaluating the xpath expression reflects a successful test run.
def eval_xpath_results(reply, hdr, exp):
	rv = 0
	errmsg = '%s: failed to evaluate XPATH expression "%s"'
	res = reply.xpath(exp)
	if type(res) == type([]) and len(res) == 0:
		# res[*] --> <type 'lxml.etree._Element'>
		if DEBUG:
			print(errmsg % (hdr, exp))
		rv = 2
	elif type(res) == type(True) and res == False:
		if DEBUG:
			print(errmsg % (hdr, exp))
		rv = 2
	#elif type(res) == type(1.0):
	#elif type(res) == type(""):
	return rv

# returns (status, reply-xml)
def send_rpc(conn, rpcxml, expected=None):
	rv = 0
	reply = conn.rpc(rpcxml)
	if not reply:
		return (1, None)

	if expected is not None:
		for exp in expected:
			rv = eval_xpath_results(reply, 'send_rpc', exp)
			if rv > 0:
				break
	else:
		rv = eval_xpath_results(reply, 'send_rpc', '//ok')

	return (rv, reply)

def run_one_test(conn, desc):
	(rv, reply) = send_rpc(conn, rpc_discard_changes)
	if rv > 0:
		print('%s: sending discard-changes RPC failed.' % desc['name'])
		return (desc['name'], rv)

	(rv, reply) = send_rpc(conn, desc['RPC'], desc['edit-config-results'])
	if rv > 0:
		print('%s: sending test RPC failed.' % desc['name'])
		return (desc['name'], rv)

	(rv, reply) = send_rpc(conn, rpc_get_config_with_defaults, ['//data'])
	if rv > 0:
		print('%s: sending get-config RPC failed.' % desc['name'])

	else:
		for exp in desc['expected-results']:
			rv = eval_xpath_results(reply, desc['name'], exp)
			if rv > 0:
				break
		if rv == 0:
			for exp in desc['unexpected-results']:
				rv = eval_xpath_results(reply, desc['name'], exp)
				rv = 0 if rv > 0 else 2
				if rv > 0:
					break

	return (desc['name'], rv)

def run_tests(conn, alltests):
	results = []
	for t in alltests:
		results.append(run_one_test(conn, t))
	return results

def main():
	parser = argparse.ArgumentParser()
	parser.add_argument("--server", help="server name e.g. 127.0.0.1 or server.com (127.0.0.1 if not specified)")
	parser.add_argument("--user", help="username e.g. admin ($USER if not specified)")
	parser.add_argument("--port", help="port e.g. 830 (830 if not specified)")
	parser.add_argument("--password", help="password e.g. mypass123 (passwordless if not specified)")
	parser.add_argument("--debug",
	                    help="show debug messages (not yuma123 library debugging",
	                    action='store_true')

	args = parser.parse_args()

	if(args.server==None or args.server==""):
		server="127.0.0.1"
	else:
		server=args.server

	if(args.port==None or args.port==""):
		port=830
	else:
		port=int(args.port)

	if(args.user==None or args.user==""):
		user=os.getenv('USER')
	else:
		user=args.user

	if(args.password==None or args.password==""):
		password=None
	else:
		password=args.password

	if args.debug:
		DEBUG = True

	conn_raw = litenc.litenc()
	ret = conn_raw.connect(server=server, port=port, user=user, password=password)
	if ret != 0:
		print("[FAILED] Connecting to server=%s:" % {'server':server})
		return(-1)
	print("[OK] Connecting to server=%(server)s:" % {'server':server})
	conn=litenc_lxml.litenc_lxml(conn_raw, strip_namespaces=True)
	ret = conn_raw.send("""
<hello xmlns="urn:ietf:params:xml:ns:netconf:base:1.0">
 <capabilities>
  <capability>urn:ietf:params:netconf:base:1.0</capability>
 </capabilities>
</hello>
""")

	if ret != 0:
		print("[FAILED] Sending <hello>")
		return(-1)

	# receive <hello>
	result=conn.receive()

	results = run_tests(conn, tests)
	ok = True
	for x in results:
		print("%-70.70s : %s" % (x[0], 'PASS' if x[1] == 0 else 'FAIL'))
		if x[1] > 0:
			ok = False

	return 0 if ok else 1

sys.exit(main())
