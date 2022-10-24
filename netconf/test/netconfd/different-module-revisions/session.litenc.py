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
#Description: Demonstrate <get-schema> returns correct versions.
#Procedure:
#1 - <get-schema> ietf-yang-types, ietf-yang-types@2013-07-15 and ietf-yang-types@2010-09-29. Confirm ietf-yang-types@2010-09-29 is different.
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
	conn=litenc_lxml.litenc_lxml(conn_raw,strip_namespaces=True)
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
	(ret, reply_xml)=conn_raw.receive()
	if ret != 0:
		print("[FAILED] Receiving <hello>")
		return(-1)

	print("[OK] Receiving <hello> =%(reply_xml)s:" % {'reply_xml':reply_xml})

	print("Connected ...")

#check for /hello/capabilities/capability == urn:ietf:params:xml:ns:yang:ietf-yang-types?module=ietf-yang-types&amp;revision=2013-07-15
#check for /hello/capabilities/capability == urn:ietf:params:xml:ns:yang:ietf-yang-types?module=ietf-yang-types&amp;revision=2010-09-24

	get_schema_rpc = """
  <get-schema xmlns="urn:ietf:params:xml:ns:yang:ietf-netconf-monitoring">
    <identifier>ietf-yang-types</identifier>
  </get-schema>
"""
	print("get-schema - get ietf-yang-types ...")
	result = conn.rpc(get_schema_rpc)
	print(lxml.etree.tostring(result))
	error_app_tag = result.xpath('rpc-error/error-app-tag')
	assert(len(error_app_tag)==1)
	assert(error_app_tag[0].text=="data-not-unique")


	get_schema_rpc = """
  <get-schema xmlns="urn:ietf:params:xml:ns:yang:ietf-netconf-monitoring">
    <identifier>ietf-yang-types</identifier>
    <version>2013-07-15</version>
  </get-schema>
"""
	print("get-schema - get ietf-yang-types@2013-07-15 ...")
	result1 = conn.rpc(get_schema_rpc)
	print(lxml.etree.tostring(result1))
	data1 = result1.xpath('data')
	assert(len(data1)==1)

	get_schema_rpc = """
  <get-schema xmlns="urn:ietf:params:xml:ns:yang:ietf-netconf-monitoring">
    <identifier>ietf-yang-types</identifier>
    <version>2013-07-15</version>
  </get-schema>
"""
	print("get-schema - get ietf-yang-types@2013-07-15 ...")
	result2 = conn.rpc(get_schema_rpc)
	print(lxml.etree.tostring(result2))
	data2 = result2.xpath('data')
	assert(len(data2)==1)

	get_schema_rpc = """
  <get-schema xmlns="urn:ietf:params:xml:ns:yang:ietf-netconf-monitoring">
    <identifier>ietf-yang-types</identifier>
    <version>2010-09-24</version>
  </get-schema>
"""
	print("get-schema - get ietf-yang-types@2010-09-24 ...")
	result3 = conn.rpc(get_schema_rpc)
	print(lxml.etree.tostring(result3))
	data3 = result3.xpath('data')
	assert(len(data3)==1)

	if(lxml.etree.tostring(data1[0]) != lxml.etree.tostring(data2[0])):
		print("Error: Should be the same schema file")
		assert(0)

	if(lxml.etree.tostring(data2[0]) == lxml.etree.tostring(data3[0])):
		print("Error: Should be different schema file")
		assert(0)

	print("OK: All good.")

	return 0

sys.exit(main())
