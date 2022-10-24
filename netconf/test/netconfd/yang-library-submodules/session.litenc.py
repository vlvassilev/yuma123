#!/usr/bin/env python

import time
import sys, os
sys.path.append("../../litenc")
import litenc
import litenc_lxml
import lxml
import argparse

def main():
	print("""
#Description: Demonstrate that ietf-yang-library is implemented.
#Procedure:
#1 - Verify /modules-state/module[name='test-yang-library-submodules'] container is present.
""")

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
		print("[FAILED] Connecting to server=%(server)s:" % {'server':server})
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

		print("#1 - Verify /modules-state/module[name='test-yang-library-submodules'] container is present.")

	get_rpc = """
<get xmlns="urn:ietf:params:xml:ns:netconf:base:1.0">
 <filter type="subtree">
  <modules-state xmlns="urn:ietf:params:xml:ns:yang:ietf-yang-library">
   <module><name>test-yang-library-submodules</name></module>
  </modules-state>
 </filter>
</get>
"""
	print("get ...")
	result = conn.rpc(get_rpc)

	print(lxml.etree.tostring(result))
	name = result.xpath('data/modules-state/module/name')
	assert(len(name)==1)

	namespace = result.xpath('data/modules-state/module/namespace')
	assert(len(namespace)==1)
	assert(namespace[0].text=="http://yuma123.org/ns/test-yang-library-submodules")

	conformance_type = result.xpath('data/modules-state/module/conformance-type')
	assert(len(conformance_type)==1)

	submodule_name = result.xpath('data/modules-state/module/submodule/name')
	assert(len(submodule_name)==1)
	assert(submodule_name[0].text=="test-yang-library-submodules-sub1")

		return(0)

sys.exit(main())
