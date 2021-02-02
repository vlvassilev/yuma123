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

rpc_get_modules_state = """
  <get xmlns="urn:ietf:params:xml:ns:netconf:base:1.0">
    <filter type="xpath" select="/modules-state"/>
  </get>
"""

# TESTS = list-of-test-details = [ <test-details>, ... ]
# test-details = { 'RPC': <rpc-xml>,
#                  'edit-config-results': <list-of-xpath-expression>,
#                  'expected-results': <list-of-xpath-expressions>,
#                  'unexpected-results': <list-of-xpath-expressions>,
#                  'name': <str> }

tests = [
	{
		'RPC': rpc_get_modules_state,
		'expected-results': [],
		'unexpected-results': [],
		'name': 'Check modules-state for deviation info',
		'edit-config-results': [ '//rpc-reply' ]
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

def load_client_module_list(filename):
	with open(filename) as f:
		lines = f.readlines()
	f.close()
	return [x.strip() for x in lines]

def parse_server_module_list(reply):
	modlist = []
	modules = reply.xpath('//module')
	for m in modules:
		name = m.find('name')
		revision = m.find('revision')
		modlist.append('%s@%s' % (name.text, revision.text))
	return modlist

def compare_module_lists(server_module_list, client_module_list):
	# Check that every module in the server list appears in the
	# client list.
	ok = True
	for servermod in server_module_list:
		# One exception . . .
		if servermod.find('ietf-netconf@') == 0:
			pass
		elif not servermod in client_module_list:
			ok = False
			print('Client is missing module %s' % (servermod))
	return ok

def main():
	print("#Description: compare list of supported modules reported by server to modules loaded by client.")

	parser = argparse.ArgumentParser()
	parser.add_argument("--server", help="server name e.g. 127.0.0.1 or server.com (127.0.0.1 if not specified)")
	parser.add_argument("--user", help="username e.g. admin ($USER if not specified)")
	parser.add_argument("--port", help="port e.g. 830 (830 if not specified)")
	parser.add_argument("--password", help="password e.g. mypass123 (passwordless if not specified)")
	parser.add_argument("--module-list", help="Client's module list - one module@revision per line")

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

	if args.module_list == None or args.module_list == "":
		module_list_fn = 'tmp/modlist'
	else:
		module_list_fn = args.module_list

	client_module_list = load_client_module_list(module_list_fn)
	if client_module_list == None:
		print("[FAILED] load client modules list")
		return(-1)

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

	# get the modules list
	(rv, reply) = send_rpc(conn, tests[0]['RPC'], tests[0]['edit-config-results'])
	if rv > 0:
		print('%s: sending RPC failed.' % tests[0]['name'])

	server_module_list = parse_server_module_list(reply)
	ok = compare_module_lists(server_module_list, client_module_list)
	return 0 if ok else 1

sys.exit(main())
