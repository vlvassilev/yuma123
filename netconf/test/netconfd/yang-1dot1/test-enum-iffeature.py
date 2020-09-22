#!/usr/bin/env python

# Run netconfd with "--module=test-enum-iffeature.yang
# --feature-enable=extra-enum" to test for an expected
# success setting the leaf value and run netconfd with
# "--module=test-enum-iffeature.yang --feature-disable=extra-enum"
# to test for an expected failure.

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

rpc_create = """
  <edit-config xmlns="urn:ietf:params:xml:ns:netconf:base:1.0">
    <target>
      <candidate/>
    </target>
    <default-operation>merge</default-operation>
    <test-option>set</test-option>
    <config>
      <top xmlns="http://yuma123.org/ns/test-enum-iffeature">
        <the-test
          xmlns:nc="urn:ietf:params:xml:ns:netconf:base:1.0"
          nc:operation="create">test-enum</the-test>
      </top>
    </config>
  </edit-config>
"""

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


def main():
	print("#Description: Attempt to create a leaf with a conditional enum.")

	parser = argparse.ArgumentParser()
	parser.add_argument("--server", help="server name e.g. 127.0.0.1 or server.com (127.0.0.1 if not specified)")
	parser.add_argument("--user", help="username e.g. admin ($USER if not specified)")
	parser.add_argument("--port", help="port e.g. 830 (830 if not specified)")
	parser.add_argument("--password", help="password e.g. mypass123 (passwordless if not specified)")
	parser.add_argument("--expectfail",
	                    help="Feature is disabled and RPC is expected to fail",
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

	exp = None
	if args.expectfail:
		exp = ['//bad-value']

	rv, reply = send_rpc(conn, rpc_discard_changes)
	if rv > 0:
		return rv

	rv, reply = send_rpc(conn, rpc_create, exp)
	if rv > 0:
		return rv

	return 0

sys.exit(main())
