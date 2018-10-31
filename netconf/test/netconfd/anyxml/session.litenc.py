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
#Description: Demonstrate that duplicated list entries in edit-config are detected.
#Procedure:
#1 - Create interface "foo" and commit. Verify commit succeeds.
#2 - Create 2x duplicate interface "bar" and commit. Verify commit fails.
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
	(ret, reply_xml)=conn_raw.receive()
	if ret != 0:
		print("[FAILED] Receiving <hello>")
		return(-1)

	print "[OK] Receiving <hello> =%(reply_xml)s:" % {'reply_xml':reply_xml}


	print("Connected ...")

	namespaces={"nc":"urn:ietf:params:xml:ns:netconf:base:1.0",
        "test-anyxml":"http://yuma123.org/ns/test/netconfd/anyxml/test-anyxml",
        "one":"urn:1",
        "two":"urn:2"}

	ping_pong_rpc = """
<ping-pong xmlns="http://yuma123.org/ns/test/netconfd/anyxml/test-anyxml"><ping><one foo="blah" xmlns="urn:1"><two bar="blaer" xmlns="urn:2">2</two><!--<tree/>--></one></ping></ping-pong>
"""
	print("ping-pong ...")
	(ret, reply_xml)= conn_raw.rpc(ping_pong_rpc)
	print (reply_xml)

	request = lxml.etree.fromstring(ping_pong_rpc)

	reply = lxml.etree.fromstring(reply_xml)
	one_sent = request.xpath("./test-anyxml:ping/one:one", namespaces=namespaces)
	one_received = reply.xpath("./test-anyxml:pong/one:one", namespaces=namespaces)
	print one_sent
	print one_received
	assert(len(one_sent)==1)
	assert(len(one_received)==1)

sys.exit(main())
