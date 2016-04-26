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
#Demonstrate <get> with xpath filter returns keys for each list container part of the result
##Procedure:
##1 - Call <get> with /interfaces-state/interface/statistics xpath filter
##2 - Verify each /interfaces-state/interface container returned has 'name' key
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

        rpc = """
<rpc message-id="1"
  xmlns="urn:ietf:params:xml:ns:netconf:base:1.0">
  <get xmlns="urn:ietf:params:xml:ns:netconf:base:1.0">
    <filter type="xpath" select="/interfaces-state/interface/statistics"/>
  </get>
</rpc>
"""
        (ret, reply_xml) = my_netconf.rpc(rpc)
        if ret != 0:
                sys.stderr.write("Create: FAILED\n")
                return (-1)
        if detect_rpc_error(reply_xml):
                sys.stderr.write("rpc_reply contains rpc-error: FAILED\n")
                sys.stderr.write(reply_xml)
                return (-1)

        doc = libxml2.parseDoc(reply_xml)
        ctxt = doc.xpathNewContext()
        ctxt.xpathRegisterNs("nc","urn:ietf:params:xml:ns:netconf:base:1.0")
        ctxt.xpathRegisterNs("if","urn:ietf:params:xml:ns:yang:ietf-interfaces")
        res_interface = ctxt.xpathEval('/nc:rpc-reply/nc:data/if:interfaces-state/if:interface')
        res_name = ctxt.xpathEval('/nc:rpc-reply/nc:data/if:interfaces-state/if:interface/if:name')
        if len(res_interface) != len(res_name):
            sys.stderr.write("The count of interface containers does not match the count of the interface/name keys\n")
            sys.stderr.write(reply_xml)
            return(-1)
        return 0

sys.exit(main())
