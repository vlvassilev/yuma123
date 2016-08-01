#!/usr/bin/python
import sys, os
import netconf
import time
from xml.dom.minidom import parseString
import libxml2

def detect_rpc_error(reply_xml):
#<rpc-reply ...
#  <rpc-error>...
#  </rpc-error>
#</rpc-reply>
        dom = parseString(reply_xml)
        assert dom.documentElement.tagName == "rpc-reply"
        rpc_error = dom.getElementsByTagName("rpc-error")
        if len(rpc_error) == 0:
                return False
        return True

def main():
	print("""
Demonstrate ietf-system:system-restart RPC operation
#Procedure:
#1 - Send ietf-system:system-restart RPC.
#2 - Confirm there is no error.
#3 - Immediately attempt new connection.
#4 - Confirm the attempt fails.
#5 - Keep attempting to connect for up to 120 sec.
#6 - Confirm the attempt succeeds.
""")
	server=os.environ.get('YUMA_AGENT_IPADDR') #e.g. "192.168.209.31"
	port=os.environ.get('YUMA_AGENT_PORT') #e.g. "830"
	user=os.environ.get('YUMA_AGENT_USER') #e.g. "root"
	password=os.environ.get('YUMA_AGENT_PASSWORD') #e.g. "hadm1_123"
	my_netconf = netconf.netconf()

	sys.stderr.write("Connect to (server=%(server)s):\n" % {'server':server})
	ret=my_netconf.connect("server=%(server)s port=%(port)s user=%(user)s password=%(password)s" % {'server':server,'port':port,'user':user,'password':password})
	if ret != 0:
		sys.stderr.write("Connect: FAILED\n")
		return (-1)

	(ret, reply_xml) = my_netconf.rpc("""
<hello>
  <capabilities>
    <capability>urn:ietf:params:netconf:base:1.0</capability>
  </capabilities>
</hello>
""")
	if ret != 0:
		sys.stderr.write("Hello: FAILED\n")
		return (-1)

	system_restart_rpc = """
<rpc message-id="000" xmlns="urn:ietf:params:xml:ns:netconf:base:1.0">
  <system-restart xmlns="urn:ietf:params:xml:ns:yang:ietf-system"/>
</rpc>
"""
	(ret, reply_xml) = my_netconf.rpc(system_restart_rpc)
	if ret != 0:
		sys.stderr.write("Restart: FAILED\n")
		return (-1)

        if detect_rpc_error(reply_xml):
                sys.stderr.write("rpc_reply contains rpc-error: FAILED\n")
                sys.stderr.write(reply_xml)
                return (-1)

        time.sleep(1)

	(ret, reply_xml) = my_netconf.rpc(system_restart_rpc)
	if ret == 0:
		sys.stderr.write("Restart did not take effect: FAILED\n")
		return (-1)

        time.sleep(10)

	my_netconf = netconf.netconf()

	sys.stderr.write("Connect to (server=%(server)s):\n" % {'server':server})
	ret=my_netconf.connect("server=%(server)s port=%(port)s user=%(user)s password=%(password)s" % {'server':server,'port':port,'user':user,'password':password})
	if ret == 0:
		sys.stderr.write("Connect should have not succeeded: FAILED\n")
		return (-1)

	start = time.time()
	while((time.time()-start)<120):
		my_netconf = netconf.netconf()

		sys.stderr.write("Connect to (server=%(server)s):\n" % {'server':server})
		ret=my_netconf.connect("server=%(server)s port=%(port)s user=%(user)s password=%(password)s" % {'server':server,'port':port,'user':user,'password':password})
		if ret == 0:
			sys.stdout.write("Restart completed in %(seconds)d sec.\n" % {'seconds':time.time()-start})
			return 0

	sys.stderr.write("Timeout!\n")
	return(-1)

sys.exit(main())
