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
#Description: Send invalid configuration.  Check later to see if
# netconfd crashed.
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


        print("# send invalid value for union element of leaf-list")

	get_rpc = """
<edit-config xmlns="urn:ietf:params:xml:ns:netconf:base:1.0">
  <target>
    <candidate/>
  </target>
  <default-operation>merge</default-operation>
  <test-option>set</test-option>
  <config>
    <test
      xmlns:nc="urn:ietf:params:xml:ns:netconf:base:1.0"
      nc:operation="create"
      xmlns="http://yuma123.org/ns/test-leaflist-union-test">
      <address>this should not work</address>
    </test>
  </config>
</edit-config>
"""
	print("edit-config ...")
	result = conn.rpc(get_rpc)
	result=litenc_lxml.strip_namespaces(result)

	print(lxml.etree.tostring(result))

        return(0)

sys.exit(main())
