#!/usr/bin/env python
import time
import sys, os
import lxml
import argparse
sys.path.append("../../litenc")
import litenc
import litenc_lxml

def main():
	print("""
#Description: Verify netconf-* session-start, etc. notifications are implemented.
#Procedure:
#1 - Open session #1 and <create-subscription>
#2 - Open additional session #2 and verify <netconf-session-start> is sent with correct paramenters on #1.
#3 - TODO
""")

	parser = argparse.ArgumentParser()
	parser.add_argument("--server", help="server name e.g. 127.0.0.1 or server.com (127.0.0.1 if not specified)")
	parser.add_argument("--user", help="username e.g. admin ($USER if not specified)")
	parser.add_argument("--port", help="port e.g. 830 (830 if not specified)")
	parser.add_argument("--password", help="password e.g. mypass123 (passwordless if not specified)")
	parser.add_argument("--with-filter-subtree", action='store_true', help="when present create-subscription has a filter to receive only netconf-session-end")

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

	conn = litenc.litenc()
	ret = conn.connect(server=server, port=port, user=user, password=password)
	if ret != 0:
		print "[FAILED] Connecting to server=%(server)s:" % {'server':server}
		return(-1)
	print "[OK] Connecting to server=%(server)s:" % {'server':server}
	conn_lxml=litenc_lxml.litenc_lxml(conn)
	ret = conn.send("""
<hello xmlns="urn:ietf:params:xml:ns:netconf:base:1.0">
 <capabilities>
  <capability>urn:ietf:params:netconf:base:1.0</capability>
 </capabilities>
</hello>
""")

#  <capability>urn:ietf:params:netconf:capability:notification:1.0</capability>
#  <capability>urn:ietf:params:netconf:capability:interleave:1.0</capability>
#  <capability>urn:ietf:params:xml:ns:yang:ietf-netconf-notifications?module=ietf-netconf-notifications&amp;revision=2012-02-06</capability>

	if ret != 0:
		print("[FAILED] Sending <hello>")
		return(-1)
	(ret, reply_xml)=conn.receive()
	if ret != 0:
		print("[FAILED] Receiving <hello>")
		return(-1)

	print "[OK] Receiving <hello> =%(reply_xml)s:" % {'reply_xml':reply_xml}

	filter=""
	if(args.with_filter_subtree):
		filter="""
<filter xmlns="urn:ietf:params:xml:ns:netconf:notification:1.0" xmlns:netconf="urn:ietf:params:xml:ns:netconf:base:1.0" netconf:type="subtree">
 <netconf-session-end xmlns="urn:ietf:params:xml:ns:yang:ietf-netconf-notifications"/>
</filter>"""

	ret = conn.send("""
<rpc xmlns="urn:ietf:params:xml:ns:netconf:base:1.0" message-id="1">
 <create-subscription xmlns="urn:ietf:params:xml:ns:netconf:notification:1.0">
 %(filter)s
</create-subscription>
</rpc>
"""%{'filter':filter})
	if ret != 0:
		print("[FAILED] Sending <create-subscription>")
		return(-1)

	(ret, reply_xml)=conn.receive()
	if ret != 0:
		print("[FAILED] Receiving <create-subscription> reply")
		return(-1)

	print "[OK] Receiving <create-subscription> reply =%(reply_xml)s:" % {'reply_xml':reply_xml}

	conn2 = litenc.litenc()
	ret = conn2.connect(server=server, port=port, user=user, password=password)
	if ret != 0:
		print "[FAILED] Connecting to server=%(server)s:" % {'server':server}
		return(-1)
	print "[OK] Connecting to server=%(server)s:" % {'server':server}

	ret = conn2.send("""
<hello xmlns="urn:ietf:params:xml:ns:netconf:base:1.0">
 <capabilities>
  <capability>urn:ietf:params:netconf:base:1.0</capability>
 </capabilities>
</hello>
""")
	if ret != 0:
		print("[FAILED] Sending <hello>")
		return(-1)
	(ret, reply_xml)=conn2.receive()
	if ret != 0:
		print("[FAILED] Receiving <hello>")
		return(-1)

	print "[OK] Receiving <hello> =%(reply_xml)s:" % {'reply_xml':reply_xml}

	conn2_lxml=litenc_lxml.litenc_lxml(conn2)
	ret = conn2_lxml.rpc("""
<close-session/>
""")
	if(not args.with_filter_subtree):
		notification_xml=conn_lxml.receive()
		if notification_xml == None:
			print("[FAILED] Receiving <netconf-session-start> notification")
			return(-1)

		print lxml.etree.tostring(notification_xml)

	notification_xml=conn_lxml.receive()
	if notification_xml == None:
		print("[FAILED] Receiving <netconf-session-end> notification")
		return(-1)

	print lxml.etree.tostring(notification_xml)

	notification_xml=litenc_lxml.strip_namespaces(notification_xml)
	match=notification_xml.xpath("/notification/netconf-session-end")
	assert(len(match)==1)

	print "[OK] Receiving <netconf-session-end>"

	return(0)

sys.exit(main())
