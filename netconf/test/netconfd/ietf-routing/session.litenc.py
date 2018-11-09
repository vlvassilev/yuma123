#!/usr/bin/env python

import time
import sys, os
sys.path.append("../../litenc")
import litenc
import litenc_lxml
import lxml
from lxml import etree
from StringIO import StringIO
import argparse

# Hmm .. ?!
def yang_data_equal(e1, e2):
	if e1.tag != e2.tag:
		assert(False)
		return False
	if e1.text==None and "" != e2.text.strip():
		assert(False)
		return False
	if e2.text==None and "" != e1.text.strip():
		assert(False)
		return False
	if e1.text!=None and e2.text!=None and e1.text.strip() != e2.text.strip():
		assert(False)
		return False
	#if e1.tail != e2.tail:
		assert(False)
		return False
	if e1.attrib != e2.attrib:
		assert(False)
		print("diff attrib")
		return False
	if len(e1) != len(e2):
		print e1
		print len(e1)
		print e2
		print len(e2)
		assert(False)
		return False
	return all(yang_data_equal(c1, c2) for c1, c2 in zip(e1, e2))

def main():
	print("""
#Description: Testcase for RFC8022 ietf-routing module.
#Procedure:
#1 - <edit-config> configuration as in RFC8022 Appendix D.
#2 - <get> the /interfaces /interfaces-state /routing /routing-state and verify data is same as in RFC8022 Appendix D.
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

	print "[OK] Receiving <hello> =%(reply_xml)s:" % {'reply_xml':reply_xml}

	print("Connected ...")

	get_yang_library_rpc = """
<get>
  <filter type="subtree">
    <modules-state xmlns="urn:ietf:params:xml:ns:yang:ietf-yang-library"/>
  </filter>
</get>
"""

	print("<get> - /ietf-yang-library:modules-state ...")
	result = conn.rpc(get_yang_library_rpc, strip_ns=False)
	print lxml.etree.tostring(result, pretty_print=True, inclusive_ns_prefixes=True)
        namespaces = {"nc":"urn:ietf:params:xml:ns:netconf:base:1.0"}
	data = result.xpath('./nc:data', namespaces=namespaces)
	assert(len(data)==1)

	edit_config_rpc = """
<edit-config>
    <target>
      <candidate/>
    </target>
    <default-operation>merge</default-operation>
    <test-option>set</test-option>
    <config>
      <interfaces xmlns="urn:ietf:params:xml:ns:yang:ietf-interfaces" xmlns:nc="urn:ietf:params:xml:ns:netconf:base:1.0" nc:operation="replace">
        <interface>
          <name>eth0</name>
          <description>Uplink to ISP.</description>
          <type xmlns:ianaift="urn:ietf:params:xml:ns:yang:iana-if-type">ianaift:ethernetCsmacd</type>
          <ipv4 xmlns="urn:ietf:params:xml:ns:yang:ietf-ip">
            <address>
              <ip>192.0.2.1</ip>
              <prefix-length>24</prefix-length>
            </address>
          </ipv4>
        </interface>
      </interfaces>
    </config>
</edit-config>
"""

	print("<edit-config> - load example config to 'candidate' ...")
	result = conn.rpc(edit_config_rpc)
	print lxml.etree.tostring(result)
	ok = result.xpath('./ok')
	assert(len(ok)==1)

	commit_rpc = '<commit/>'

	print("<commit> - commit example config ...")
	result = conn.rpc(commit_rpc)
	print lxml.etree.tostring(result)
	ok = result.xpath('./ok')
	assert(len(ok)==1)



	get_example_data_rpc = """
<get>
  <filter type="subtree">
    <interfaces xmlns="urn:ietf:params:xml:ns:yang:ietf-interfaces"/>
    <interfaces-state xmlns="urn:ietf:params:xml:ns:yang:ietf-interfaces"/>
    <routing xmlns="urn:ietf:params:xml:ns:yang:ietf-routing"/>
    <routing-state xmlns="urn:ietf:params:xml:ns:yang:ietf-routing"/>
  </filter>
</get>
"""

	print("<get> - example data ...")
	result = conn.rpc(get_example_data_rpc, strip_ns=False)
	print lxml.etree.tostring(result, pretty_print=True, inclusive_ns_prefixes=True)
        namespaces = {"nc":"urn:ietf:params:xml:ns:netconf:base:1.0"}
	data = result.xpath('./nc:data', namespaces=namespaces)
	assert(len(data)==1)

	expected="""
<data xmlns="urn:ietf:params:xml:ns:netconf:base:1.0">
    <interfaces xmlns="urn:ietf:params:xml:ns:yang:ietf-interfaces">
      <interface>
        <name>eth0</name>
        <description>Uplink to ISP.</description>
        <type xmlns:ianaift="urn:ietf:params:xml:ns:yang:iana-if-type">ianaift:ethernetCsmacd</type>
        <ipv4 xmlns="urn:ietf:params:xml:ns:yang:ietf-ip">
          <address>
            <ip>192.0.2.1</ip>
            <prefix-length>24</prefix-length>
          </address>
        </ipv4>
      </interface>
    </interfaces>
    <interfaces-state xmlns="urn:ietf:params:xml:ns:yang:ietf-interfaces">
      <interface>
        <name>eth0</name>
        <description>Uplink to ISP.</description>
        <type xmlns:ianaift="urn:ietf:params:xml:ns:yang:iana-if-type">ianaift:ethernetCsmacd</type>
        <phys-address>00:0C:42:E5:B1:E9</phys-address>
        <oper-status>up</oper-status>
	<statistics>
          <discontinuity-time>2015-10-24T17:11:27+02:00</discontinuity-time>
        </statistics>
        <ipv4 xmlns="urn:ietf:params:xml:ns:yang:ietf-ip">
          <address>
            <ip>192.0.2.1</ip>
            <prefix-length>24</prefix-length>
          </address>
        </ipv4>
      </interface>
    </interfaces-state>
</data>
"""

	data_expected=etree.parse(StringIO(lxml.etree.tostring(lxml.etree.fromstring(expected), pretty_print=True)))
	data_received=etree.parse(StringIO(lxml.etree.tostring(data[0], pretty_print=True)))

	a = StringIO();
	b = StringIO();
	data_expected.write_c14n(a)
	data_received.write_c14n(b)

	if yang_data_equal(lxml.etree.fromstring(a.getvalue()), lxml.etree.fromstring(b.getvalue())):
		print "Bingo!"
		return 0
	else:
		print "Expected (A) different from received (B):"
		print "A:"
		print a.getvalue()
		print "B:"
		print b.getvalue()
		return 1

sys.exit(main())
