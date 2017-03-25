#!/usr/bin/env python
import time
import sys, os
sys.path.append("../../netconf-io")
import netconf
import netconf_lxml

def main():
	print("""
#Description: Verify netconf-* session-start, etc. notifications are implemented.
#Procedure:
#1 - Open session #1 and <create-subscription>
#2 - Open additional session #2 and verify <netconf-session-start> is sent with correct paramenters on #1.
#3 - TODO
""")

	port=830
	server="127.0.0.1"

	conn = netconf.netconf()
	ret = conn.connect('server=%(server)s port=%(port)d user=%(user)s dump-session=nc-session-log-' % {'server':server, 'port':port, 'user':os.getenv('USER')})
	if ret != 0:
		print "[FAILED] Connecting to server=%(server)s:" % {'server':server}
		return(-1)
	print "[OK] Connecting to server=%(server)s:" % {'server':server}
	conn_lxml=netconf_lxml.netconf_lxml(conn)
	ret = conn.send("""
<hello>
 <capabilities>
  <capability>urn:ietf:params:netconf:base:1.0</capability>\
 </capabilities>\
</hello>
""")
	if ret != 0:
		print("[FAILED] Sending <hello>")
		return(-1)
	(ret, reply_xml)=conn.receive()
	if ret != 0:
		print("[FAILED] Receiving <hello>")
		return(-1)

	print "[OK] Receiving <hello> =%(reply_xml)s:" % {'reply_xml':reply_xml}

	ret = conn.send("""
<rpc xmlns="urn:ietf:params:xml:ns:netconf:base:1.0" message-id="1">
 <create-subscription xmlns="urn:ietf:params:xml:ns:netconf:notification:1.0"/>
</rpc>
""")
	if ret != 0:
		print("[FAILED] Sending <create-subscription>")
		return(-1)

	(ret, reply_xml)=conn.receive()
	if ret != 0:
		print("[FAILED] Receiving <create-subscription> reply")
		return(-1)

	print "[OK] Receiving <create-subscription> reply =%(reply_xml)s:" % {'reply_xml':reply_xml}

	conn2 = netconf.netconf()
	ret = conn2.connect('server=%(server)s port=%(port)d user=%(user)s dump-session=nc-session-log-' % {'server':server, 'port':port, 'user':os.getenv('USER')})
	if ret != 0:
		print "[FAILED] Connecting to server=%(server)s:" % {'server':server}
		return(-1)
	print "[OK] Connecting to server=%(server)s:" % {'server':server}

	ret = conn2.send("""
<hello>
 <capabilities>
  <capability>urn:ietf:params:netconf:base:1.0</capability>\
 </capabilities>\
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

	(ret, notification_xml)=conn.receive()
	if ret != 0:
		print("[FAILED] Receiving <netconf-session-start> notification")
		return(-1)

	print "[OK] Receiving <netconf-session-start> notification =%(notification_xml)s:" % {'notification_xml':notification_xml}

	return(0)

sys.exit(main())
