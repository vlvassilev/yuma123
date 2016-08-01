#!/usr/bin/python
import sys, os
import netconf
import time

def main():
	print("""
Demonstrate shutdown operation (WARNING: netconf server must be manualy restarted following this test).
#Procedure:
#1 - Shutdown
#2 - Try another query
#3 - Check that no response is received
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

	shutdown_rpc = """
<rpc message-id="000" xmlns="urn:ietf:params:xml:ns:netconf:base:1.0">
  <shutdown xmlns="http://netconfcentral.org/ns/yuma-system"/>
</rpc>
"""
	(ret, reply_xml) = my_netconf.rpc(shutdown_rpc)
	if ret != 0:
		sys.stderr.write("Shutdown: FAILED\n")
		return (-1)

	time.sleep(1)

	(ret, reply_xml) = my_netconf.rpc(shutdown_rpc)
	if ret == 0:
		sys.stderr.write("Shutdown did not take effect: FAILED\n")
		return (-1)

	return 0

sys.exit(main())
