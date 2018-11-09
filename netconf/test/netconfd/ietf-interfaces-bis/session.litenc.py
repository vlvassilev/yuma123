#!/usr/bin/env python

import time
import sys, os
sys.path.append("../../litenc")
import litenc
import litenc_lxml
import lxml
from lxml import etree
#from litenc import strip_namespaces
from StringIO import StringIO
import argparse
from operator import attrgetter

def get_interface_name(interface):
	print("calling get_interface_name")
	print(interface.tag)

	name=interface.xpath('./name')
	assert(len(name)==1)
	print(name[0].text)
	return name[0].text

def strip_namespaces(tree):
	xslt='''<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
    <xsl:output method="xml" indent="no"/>

    <xsl:template match="/|comment()|processing-instruction()">
        <xsl:copy>
          <xsl:apply-templates/>
        </xsl:copy>
    </xsl:template>

    <xsl:template match="*">
        <xsl:element name="{local-name()}">
          <xsl:apply-templates select="@*|node()"/>
        </xsl:element>
    </xsl:template>

    <xsl:template match="@*">
        <xsl:attribute name="{local-name()}">
          <xsl:value-of select="."/>
        </xsl:attribute>
    </xsl:template>
    </xsl:stylesheet>
    '''
	xslt_doc = lxml.etree.fromstring(xslt)
	transform = lxml.etree.XSLT(xslt_doc)
	tree = transform(tree)
	return tree

# Hmm .. ?!
def yang_data_equal(e1, e2):
	print e1.tag
	print e2.tag
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
		print (e1.attrib)
		print (e2.attrib)
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
	for node in e1.findall("*"):  # searching top-level nodes only: node1, node2 ...
		node[:] = sorted(node, key=attrgetter("tag"))
	for node in e2.findall("*"):  # searching top-level nodes only: node1, node2 ...
		node[:] = sorted(node, key=attrgetter("tag"))
	for c1,c2 in zip(e1,e2):
		print c1
		print c2
		print("---")
		yang_data_equal(c1, c2)
	return 1
	#return all(yang_data_equal(c1, c2) for c1, c2 in sorted(zip(e1, e2)))

def main():
	print("""
#Description: Testcase for draft-ietf-netmod-rfc7223bis-00.txt ietf-interfaces module.
#Procedure:
#1 - <edit-config> configuration as in rfc7223bis Appendix D.
#2 - <get-data> the /interfaces container and verify data is same as in rfc7223bis Appendix E.
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
	#Copied from draft-ietf-netmod-rfc7223bis-00 Appendix D.
	edit_config_rpc = """
<edit-config>
    <target>
      <candidate/>
    </target>
    <default-operation>merge</default-operation>
    <test-option>set</test-option>
    <config>
      <interfaces xmlns="urn:ietf:params:xml:ns:yang:ietf-interfaces" xmlns:nc="urn:ietf:params:xml:ns:netconf:base:1.0"
           xmlns:ianaift="urn:ietf:params:xml:ns:yang:iana-if-type"
           xmlns:vlan="http://example.com/vlan"
           nc:operation="replace">
         <interface>
           <name>eth0</name>
           <type>ianaift:ethernetCsmacd</type>
           <enabled>false</enabled>
         </interface>

         <interface>
           <name>eth1</name>
           <type>ianaift:ethernetCsmacd</type>
           <enabled>true</enabled>
           <vlan:vlan-tagging>true</vlan:vlan-tagging>
         </interface>

         <interface>
           <name>eth1.10</name>
           <type>ianaift:l2vlan</type>
           <enabled>true</enabled>
           <vlan:base-interface>eth1</vlan:base-interface>
           <vlan:vlan-id>10</vlan:vlan-id>
         </interface>

         <interface>
           <name>lo1</name>
           <type>ianaift:softwareLoopback</type>
           <enabled>true</enabled>
         </interface>
       </interfaces>
    </config>
</edit-config>
"""

	print("<edit-config> - load Appendix D. example config to 'candidate' ...")
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
<get-data xmlns="urn:ietf:params:xml:ns:yang:ietf-netconf-datastores">
  <datastore xmlns:ds="urn:ietf:params:xml:ns:yang:ietf-datastores">ds:operational</datastore>
  <subtree-filter>
    <interfaces xmlns="urn:ietf:params:xml:ns:yang:ietf-interfaces"/>
  </subtree-filter>
  <with-origin/>
</get-data>
"""

	print("<get-data> - Appendix E. data ...")
	result = conn.rpc(get_example_data_rpc, strip_ns=False)
	print lxml.etree.tostring(result, pretty_print=True, inclusive_ns_prefixes=True)
        namespaces = {"ncds":"urn:ietf:params:xml:ns:yang:ietf-netconf-datastores"}
	data = result.xpath('./ncds:data', namespaces=namespaces)
	assert(len(data)==1)
        #Copy from draft-netconf-nmda-netconf-01
	expected="""
<data xmlns="urn:ietf:params:xml:ns:yang:ietf-netconf-datastores" xmlns:nc="urn:ietf:params:xml:ns:netconf:base:1.0">
  <interfaces
      xmlns="urn:ietf:params:xml:ns:yang:ietf-interfaces"
      xmlns:ianaift="urn:ietf:params:xml:ns:yang:iana-if-type"
      xmlns:vlan="http://example.com/vlan"
      xmlns:or="urn:ietf:params:xml:ns:yang:ietf-origin">
    <interface or:origin="or:intended">
      <name>eth0</name>
      <type>ianaift:ethernetCsmacd</type>
      <enabled>false</enabled>
      <admin-status>down</admin-status>
      <oper-status>down</oper-status>
      <if-index>2</if-index>
      <phys-address>00:01:02:03:04:05</phys-address>
      <statistics>
        <discontinuity-time>
          2013-04-01T03:00:00+00:00
        </discontinuity-time>
        <!-- counters now shown here -->
      </statistics>
    </interface>

    <interface or:origin="or:intended">
      <name>eth1</name>
      <type>ianaift:ethernetCsmacd</type>
      <enabled>true</enabled>
      <admin-status>up</admin-status>
      <oper-status>up</oper-status>
      <if-index>7</if-index>
      <phys-address>00:01:02:03:04:06</phys-address>
      <higher-layer-if>eth1.10</higher-layer-if>
      <statistics>
        <discontinuity-time>
          2013-04-01T03:00:00+00:00
        </discontinuity-time>
       <!-- counters now shown here -->
     </statistics>
      <vlan:vlan-tagging>true</vlan:vlan-tagging>
    </interface>
    <interface or:origin="or:intended">
      <name>eth1.10</name>
      <type>ianaift:l2vlan</type>
      <enabled>true</enabled>
      <admin-status>up</admin-status>
      <oper-status>up</oper-status>
      <if-index>9</if-index>
      <lower-layer-if>eth1</lower-layer-if>
      <statistics>
       <discontinuity-time>
         2013-04-01T03:00:00+00:00
        </discontinuity-time>
        <!-- counters now shown here -->
      </statistics>
      <vlan:base-interface>eth1</vlan:base-interface>
     <vlan:vlan-id>10</vlan:vlan-id>
    </interface>
    <!-- This interface is not configured -->
    <interface or:origin="or:system">
      <name>eth2</name>
      <type>ianaift:ethernetCsmacd</type>
      <admin-status>down</admin-status>
      <oper-status>down</oper-status>
      <if-index>8</if-index>
      <phys-address>00:01:02:03:04:07</phys-address>
      <statistics>
        <discontinuity-time>
          2013-04-01T03:00:00+00:00
        </discontinuity-time>
        <!-- counters now shown here -->
      </statistics>
     </interface>
     <interface or:origin="or:intended">
      <name>lo1</name>
      <type>ianaift:softwareLoopback</type>
      <enabled>true</enabled>
      <admin-status>up</admin-status>
      <oper-status>up</oper-status>
      <if-index>1</if-index>
      <statistics>
        <discontinuity-time>
          2013-04-01T03:00:00+00:00
        </discontinuity-time>
        <!-- counters now shown here -->
      </statistics>
    </interface>
  </interfaces>
</data>
"""

	expected = lxml.etree.fromstring(expected)

        #strip comments
	comments = expected.xpath('//comment()')
	for c in comments:
		p = c.getparent()
		p.remove(c)

        #strip namespaces
	data1 = expected.xpath('.',namespaces=namespaces)
	data_expected=strip_namespaces(data1[0])
	data_received=strip_namespaces(data[0])

	#sort schema lists by key in alphabetical key order - hardcoded /interfaces/interface[name]
	for node in data_expected.findall("./interfaces"):
		node[:] = sorted(node, key=lambda child: get_interface_name(child))
	for node in data_received.findall("./interfaces"):
		node[:] = sorted(node, key=lambda child: get_interface_name(child))

	#sort attributes
	a = StringIO();
	b = StringIO();
	data_expected.write_c14n(a)
	data_received.write_c14n(b)

	print("Expected:")
	print(a.getvalue())
	print("Received:")
	print(b.getvalue())
	if yang_data_equal(lxml.etree.fromstring(a.getvalue()), lxml.etree.fromstring(b.getvalue())):
		print "Bingo!"
		return 0
	else:
		print "Error: YANG data not equal!"
		return 1

sys.exit(main())
