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
#Description:
## Verify get-schema works for a submodule.
#Procedure:
## loaded modules and submodules should be represented to netconf-state/schema list and they should be also retrieved by get-schema rpc.
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
		print "[FAILED] Connecting to server=%(server)s:" % {'server':server}
		return(-1)
	print "[OK] Connecting to server=%(server)s:" % {'server':server}
	conn=litenc_lxml.litenc_lxml(conn_raw)
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



	get_rpc = """
<get xmlns="urn:ietf:params:xml:ns:netconf:base:1.0">
 <filter type="subtree">
  <netconf-state xmlns="urn:ietf:params:xml:ns:yang:ietf-netconf-monitoring">
   <schemas><schema><identifier>%(identifier)s</identifier></schema></schemas>
  </netconf-state>
 </filter>
</get>
"""

	for identifier in {"main-module", "sub-module"}:
		print("get %s."%(identifier))
		result = conn.rpc(get_rpc%{'identifier':identifier})
		ok = result.xpath('data/netconf-state/schemas/schema/identifier')
		print result
		# print ok
		# print lxml.etree.tostring(result)
		#assert(len(ok)==1)
		print "[OK] Retrieving a (sub)module"

		get_schema = """
<get-schema xmlns="urn:ietf:params:xml:ns:yang:ietf-netconf-monitoring">
  <identifier>%(identifier)s</identifier>
</get-schema>
"""
		print("")
		print("get-schema")

		print(get_schema%{'identifier':identifier})
		result = conn.rpc(get_schema%{'identifier':identifier})
		print lxml.etree.tostring(result)
		rpc_error = result.xpath('rpc-error')
		assert(len(rpc_error)==0)
		print "[OK] Retrieving a (sub)module"

sys.exit(main())
