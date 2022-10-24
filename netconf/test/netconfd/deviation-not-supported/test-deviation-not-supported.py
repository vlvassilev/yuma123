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

rpc_test1a = """
  <edit-config xmlns="urn:ietf:params:xml:ns:netconf:base:1.0">
    <target>
      <candidate/>
    </target>
    <default-operation>merge</default-operation>
    <test-option>set</test-option>
    <config>
      <top xmlns="urn:labn:params:xml:ns:yang:test-deviation-not-supported-1">
        <supported-value
          xmlns:nc="urn:ietf:params:xml:ns:netconf:base:1.0"
          nc:operation="create">1</supported-value>
      </top>
    </config>
  </edit-config>
"""

rpc_test1b = """
  <edit-config xmlns="urn:ietf:params:xml:ns:netconf:base:1.0">
    <target>
      <candidate/>
    </target>
    <default-operation>merge</default-operation>
    <test-option>set</test-option>
    <config>
      <top xmlns="urn:labn:params:xml:ns:yang:test-deviation-not-supported-1">
        <unsupported-value
          xmlns:nc="urn:ietf:params:xml:ns:netconf:base:1.0"
          nc:operation="create">1</unsupported-value>
      </top>
    </config>
  </edit-config>
"""

rpc_test2a = """
  <edit-config xmlns="urn:ietf:params:xml:ns:netconf:base:1.0">
    <target>
      <candidate/>
    </target>
    <default-operation>merge</default-operation>
    <test-option>set</test-option>
    <config>
      <top xmlns="urn:labn:params:xml:ns:yang:test-deviation-not-supported-1">
        <supported-value
          xmlns:nc="urn:ietf:params:xml:ns:netconf:base:1.0"
          nc:operation="create">1</supported-value>
      </top>
    </config>
  </edit-config>
"""

rpc_test2b = """
  <edit-config xmlns="urn:ietf:params:xml:ns:netconf:base:1.0">
    <target>
      <candidate/>
    </target>
    <default-operation>merge</default-operation>
    <test-option>set</test-option>
    <config>
      <top xmlns="urn:labn:params:xml:ns:yang:test-deviation-not-supported-1">
        <unsupported-value
          xmlns:nc="urn:ietf:params:xml:ns:netconf:base:1.0"
          nc:operation="create">1</unsupported-value>
      </top>
    </config>
  </edit-config>
"""

rpc_test3 = """
  <edit-config xmlns="urn:ietf:params:xml:ns:netconf:base:1.0">
    <target>
      <candidate/>
    </target>
    <default-operation>merge</default-operation>
    <test-option>set</test-option>
    <config>
      <interfaces xmlns="urn:ietf:params:xml:ns:yang:ietf-interfaces">
        <interface>
          <name>eth0</name>
          <type
            xmlns:ianaift="urn:ietf:params:xml:ns:yang:iana-if-type">ianaift:ethernetCsmacd</type>
          <ipv4 xmlns="urn:ietf:params:xml:ns:yang:ietf-ip">
            <forwarding
              xmlns:nc="urn:ietf:params:xml:ns:netconf:base:1.0"
              nc:operation="create">true</forwarding>
          </ipv4>
        </interface>
      </interfaces>
    </config>
  </edit-config>
"""

rpc_test4a = """
  <edit-config xmlns="urn:ietf:params:xml:ns:netconf:base:1.0">
    <target>
      <candidate/>
    </target>
    <default-operation>merge</default-operation>
    <test-option>set</test-option>
    <config>
      <system xmlns="urn:ietf:params:xml:ns:yang:ietf-system">
        <contact
          xmlns:nc="urn:ietf:params:xml:ns:netconf:base:1.0"
          nc:operation="create">me</contact>
      </system>
    </config>
  </edit-config>
"""

rpc_test4b = """
  <edit-config xmlns="urn:ietf:params:xml:ns:netconf:base:1.0">
    <target>
      <candidate/>
    </target>
    <default-operation>merge</default-operation>
    <test-option>set</test-option>
    <config>
      <system xmlns="urn:ietf:params:xml:ns:yang:ietf-system">
        <location
          xmlns:nc="urn:ietf:params:xml:ns:netconf:base:1.0"
          nc:operation="create">here</location>
      </system>
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
		'RPC': rpc_test1a,
		'expected-results': [ ],
		'unexpected-results': [],
		'name': 'Test 1a - internal deviation: set leaf',
		'edit-config-results': None
	},
	{
		'RPC': rpc_test1b,
		'expected-results': [],
		'unexpected-results': [],
		'name': 'Test 1b - internal deviation: set unsupported leaf',
		'edit-config-results': [ '//bad-element' ]
	},
	{
		'RPC': rpc_test2a,
		'expected-results': [ ],
		'unexpected-results': [],
		'name': 'Test 2a - external deviation: set leaf',
		'edit-config-results': None
	},
	{
		'RPC': rpc_test2b,
		'expected-results': [],
		'unexpected-results': [],
		'name': 'Test 2b - external deviation: set unsupported leaf',
		'edit-config-results': [ '//bad-element' ]
	},
	{
		'RPC': rpc_test3,
		'expected-results': [],
		'unexpected-results': [],
		'name': 'Test 3 - node path with multiple module prefixes',
		'edit-config-results': [ '//bad-element' ]
	},
	{
		'RPC': rpc_test4a,
		'expected-results': [ ],
		'unexpected-results': [],
		'name': 'Test 4a - add deviation to "base" module: set leaf',
		'edit-config-results': None
	},
	{
		'RPC': rpc_test4b,
		'expected-results': [],
		'unexpected-results': [],
		'name': 'Test 4b - add deviation to "base" module: set unsupported leaf',
		'edit-config-results': [ '//bad-element' ]
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
			print((errmsg % (hdr, exp)))
		rv = 2
	elif type(res) == type(True) and res == False:
		if DEBUG:
			print((errmsg % (hdr, exp)))
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
		print(('%s: sending discard-changes RPC failed.' % desc['name']))
		return (desc['name'], rv)

	(rv, reply) = send_rpc(conn, desc['RPC'], desc['edit-config-results'])
	if rv > 0:
		print(('%s: sending test RPC failed.' % desc['name']))
		return (desc['name'], rv)

	return (desc['name'], rv)

def run_tests(conn, alltests):
	results = []
	for t in alltests:
		results.append(run_one_test(conn, t))
	return results

def main():
	print("#Description: Attempt to create a leaf with a conditional identity.")

	parser = argparse.ArgumentParser()
	parser.add_argument("--server", help="server name e.g. 127.0.0.1 or server.com (127.0.0.1 if not specified)")
	parser.add_argument("--user", help="username e.g. admin ($USER if not specified)")
	parser.add_argument("--port", help="port e.g. 830 (830 if not specified)")
	parser.add_argument("--password", help="password e.g. mypass123 (passwordless if not specified)")

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

	conn_raw = litenc.litenc()
	ret = conn_raw.connect(server=server, port=port, user=user, password=password)
	if ret != 0:
		print(("[FAILED] Connecting to server=%s:" % {'server':server}))
		return(-1)
	print(("[OK] Connecting to server=%(server)s:" % {'server':server}))
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
		print(("%-70.70s : %s" % (x[0], 'PASS' if x[1] == 0 else 'FAIL')))
		if x[1] > 0:
			ok = False

	send_rpc(conn, rpc_discard_changes)

	return 0 if ok else 1

sys.exit(main())
