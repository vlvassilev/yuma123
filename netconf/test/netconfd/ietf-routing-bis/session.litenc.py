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

def json2xml(json_config):
	#TODO
	return ""
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
	if e1.text==None and e2.text==None:
		print("empty container")
	else:
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
#Description: Testcase for draft-ietf-netmod-rfc8022bis-04 ietf-routing module.
#Procedure:
#1 - <edit-config> configuration as in rfc8022bis Appendix D.
#2 - <get-data> the /interfaces and /routing containers and verify data is same as in rfc8022bis Appendix E.
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
	#Copied from draft-ietf-netmod-rfc8022bis-00 Appendix E.
	json_config = """
   {
     "ietf-interfaces:interfaces": {
       "interface": [
         {
           "name": "eth0",
           "type": "iana-if-type:ethernetCsmacd",
           "description": "Uplink to ISP.",
           "phys-address": "00:0C:42:E5:B1:E9",
           "oper-status": "up",
           "statistics": {
             "discontinuity-time": "2015-10-24T17:11:27+02:00"
           },
           "ietf-ip:ipv4": {
             "forwarding": true,
             "mtu": 1500,
             "address": [
               {
                 "ip": "192.0.2.1",
                 "prefix-length": 24
               }
             ],
           },
           "ietf-ip:ipv6": {
             "forwarding": true,
             "mtu": 1500,
             "address": [
               {
                 "ip": "2001:0db8:0:1::1",
                 "prefix-length": 64
               }
             ],
             "autoconf": {
               "create-global-addresses": false
             }
             "ietf-ipv6-unicast-routing:
                ipv6-router-advertisements": {
               "send-advertisements": false
             }
           }
         },
         {
           "name": "eth1",
           "type": "iana-if-type:ethernetCsmacd",
           "description": "Interface to the internal network.",
           "phys-address": "00:0C:42:E5:B1:EA",
           "oper-status": "up",
           "statistics": {
             "discontinuity-time": "2015-10-24T17:11:29+02:00"
           },
           "ietf-ip:ipv4": {
             "forwarding": true,
             "mtu": 1500,
             "address": [
               {
                 "ip": "198.51.100.1",
                 "prefix-length": 24
               }
             ],
           },
           "ietf-ip:ipv6": {
             "forwarding": true,
             "mtu": 1500,
             "address": [
               {
                 "ip": "2001:0db8:0:2::1",
                 "prefix-length": 64
               }
             ],
             "autoconf": {
               "create-global-addresses": false
             },
             "ietf-ipv6-unicast-routing:
                ipv6-router-advertisements": {
               "send-advertisements": true,
               "prefix-list": {
                 "prefix": [
                   {
                     "prefix-spec": "2001:db8:0:2::/64"
                   }
                 ]
               }
             }
           }
         }
       ]
     },

     "ietf-routing:routing": {
       "router-id": "192.0.2.1",
       "control-plane-protocols": {
         "control-plane-protocol": [
           {
             "type": "ietf-routing:static",
             "name": "st0",
             "description":
               "Static routing is used for the internal network.",
             "static-routes": {
               "ietf-ipv4-unicast-routing:ipv4": {
                 "route": [
                   {
                     "destination-prefix": "0.0.0.0/0",
                     "next-hop": {
                       "next-hop-address": "192.0.2.2"
                     }
                   }
                 ]
               },
               "ietf-ipv6-unicast-routing:ipv6": {
                 "route": [
                   {
                     "destination-prefix": "::/0",
                     "next-hop": {
                       "next-hop-address": "2001:db8:0:1::2"
                     }
                   }
                 ]
               }
             }
           }
         ]
       }
       "ribs": {
         "rib": [
           {
             "name": "ipv4-master",
             "address-family":
               "ietf-ipv4-unicast-routing:ipv4-unicast",
             "default-rib": true,
             "routes": {
               "route": [
                 {
                   "ietf-ipv4-unicast-routing:destination-prefix":
                     "192.0.2.1/24",
                   "next-hop": {
                     "outgoing-interface": "eth0"
                   },
                   "route-preference": 0,
                   "source-protocol": "ietf-routing:direct",
                   "last-updated": "2015-10-24T17:11:27+02:00"
                 },
                 {
                   "ietf-ipv4-unicast-routing:destination-prefix":
                     "198.51.100.0/24",
                   "next-hop": {
                     "outgoing-interface": "eth1"
                   },
                   "source-protocol": "ietf-routing:direct",
                   "route-preference": 0,
                   "last-updated": "2015-10-24T17:11:27+02:00"
                 },
                 {
                   "ietf-ipv4-unicast-routing:destination-prefix":
                     "0.0.0.0/0",
                   "source-protocol": "ietf-routing:static",
                   "route-preference": 5,
                   "next-hop": {
                     "ietf-ipv4-unicast-routing:next-hop-address":
                       "192.0.2.2"
                   },
                   "last-updated": "2015-10-24T18:02:45+02:00"
                 }
               ]
             }
           },
           {
             "name": "ipv6-master",
             "address-family":
               "ietf-ipv6-unicast-routing:ipv6-unicast",
             "default-rib": true,
             "routes": {
               "route": [
                 {
                   "ietf-ipv6-unicast-routing:destination-prefix":
                     "2001:db8:0:1::/64",
                   "next-hop": {
                     "outgoing-interface": "eth0"
                   },
                   "source-protocol": "ietf-routing:direct",
                   "route-preference": 0,
                   "last-updated": "2015-10-24T17:11:27+02:00"
                 },
                 {
                   "ietf-ipv6-unicast-routing:destination-prefix":
                     "2001:db8:0:2::/64",
                   "next-hop": {
                     "outgoing-interface": "eth1"
                   },
                   "source-protocol": "ietf-routing:direct",
                   "route-preference": 0,
                   "last-updated": "2015-10-24T17:11:27+02:00"
                 },
                 {
                   "ietf-ipv6-unicast-routing:destination-prefix":
                     "::/0",
                   "next-hop": {
                     "ietf-ipv6-unicast-routing:next-hop-address":
                       "2001:db8:0:1::2"
                   },
                   "source-protocol": "ietf-routing:static",
                   "route-preference": 5,
                   "last-updated": "2015-10-24T18:02:45+02:00"
                 }
               ]
             }
           }
         ]
       }
     },
   }
"""
	edit_config_rpc = """
<edit-config>
    <target>
      <candidate/>
    </target>
    <default-operation>merge</default-operation>
    <test-option>set</test-option>
    <config>
    %(xml_config)
    </config>
</edit-config>
""" % {'xml_config':json2xml(json_config)}

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
    <routing xmlns="urn:ietf:params:xml:ns:yang:ietf-routing"/>
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

	expected="""
<data xmlns="urn:ietf:params:xml:ns:yang:ietf-netconf-datastores" xmlns:nc="urn:ietf:params:xml:ns:netconf:base:1.0">
      <routing
        xmlns="urn:ietf:params:xml:ns:yang:ietf-routing"
        xmlns:or="urn:ietf:params:xml:ns:yang:ietf-origin">

        <router-id or:origin="or:intended">192.0.2.1</router-id>
        <control-plane-protocols or:origin="or:intended">
          <control-plane-protocl>
            <type>ietf-routing:static</type>
            <name></name>
            <static-routes>
              <ietf-ipv4-unicast-routing:ipv4>
                <route>
                  <destination-prefix>0.0.0.0/0</destination-prefix>
                  <next-hop>
                    <next-hop-address>192.0.2.2</next-hop-address>
                  </next-hop>
                </route>
              </ietf-ipv4-unicast-routing:ipv4>
              <ietf-ipv6-unicast-routing:ipv6>
                <route>
                  <destination-prefix>::/0</destination-prefix>
                  <next-hop>
                    <next-hop-address>2001:db8:0:1::2</next-hop-address>
                  </next-hop>
                </route>
              </ietf-ipv6-unicast-routing:ipv6>
            </static-routes>
          </control-plane-protocl>
        </control-plane-protocols>

        <ribs>
          <rib or:origin="or:intended">
            <name>ipv4-master</name>
            <address-family>
              ietf-ipv4-unicast-routing:ipv4-unicast
            </address-family>
            <default-rib>true</default-rib>
            <routes>
              <route>
                <ietf-ipv4-unicast-routing:destination-prefix>
                  192.0.2.1/24
                </ietf-ipv4-unicast-routing:destination-prefix>
                <next-hop>
                  <outgoing-interface>eth0</outgoing-interface>

                </next-hop>
                <route-preference>0</route-preference>
                <source-protocol>ietf-routing:direct</source-protocol>
                <last-updated>2015-10-24T17:11:27+02:00</last-updated>
              </route>
              <route>
                <ietf-ipv4-unicast-routing:destination-prefix>
                  98.51.100.0/24
                </ietf-ipv4-unicast-routing:destination-prefix>
                <next-hop>
                  <outgoing-interface>eth1</outgoing-interface>
                </next-hop>
                <route-preference>0</route-preference>
                <source-protocol>ietf-routing:direct</source-protocol>
                <last-updated>2015-10-24T17:11:27+02:00</last-updated>
              </route>
              <route>
                <ietf-ipv4-unicast-routing:destination-prefix>0.0.0.0/0
                </ietf-ipv4-unicast-routing:destination-prefix>
                <next-hop>
                  <ietf-ipv4-unicast-routing:next-hop-address>192.0.2.2
                  </ietf-ipv4-unicast-routing:next-hop-address>
                </next-hop>
                <route-preference>5</route-preference>
                <source-protocol>ietf-routing:static</source-protocol>
                <last-updated>2015-10-24T18:02:45+02:00</last-updated>
              </route>
            </routes>
          </rib>
          <rib or:origin="or:intended">
            <name>ipv6-master</name>
            <address-family>
              ietf-ipv6-unicast-routing:ipv6-unicast
            </address-family>
            <default-rib>true</default-rib>
            <routes>
              <route>
                <ietf-ipv6-unicast-routing:destination-prefix>
                  2001:db8:0:1::/64
                </ietf-ipv6-unicast-routing:destination-prefix>
                <next-hop>
                  <outgoing-interface>eth0</outgoing-interface>
                </next-hop>
                <route-preference>0</route-preference>
                <source-protocol>ietf-routing:direct</source-protocol>
                <last-updated>2015-10-24T17:11:27+02:00</last-updated>
              </route>
              <route>
                <ietf-ipv6-unicast-routing:destination-prefix>
                  2001:db8:0:2::/64
                </ietf-ipv6-unicast-routing:destination-prefix>
                <next-hop>
                  <outgoing-interface>eth1</outgoing-interface>
                </next-hop>
                <route-preference>0</route-preference>
                <source-protocol>ietf-routing:direct</source-protocol>
                <last-updated>2015-10-24T17:11:27+02:00</last-updated>
              </route>
              <route>
                <ietf-ipv6-unicast-routing:destination-prefix>::/0
                </ietf-ipv6-unicast-routing:destination-prefix>
                <next-hop>
                  <ietf-ipv6-unicast-routing:next-hop-address>
                    2001:db8:0:1::2
                  </ietf-ipv6-unicast-routing:next-hop-address>
                </next-hop>
                <route-preference>5</route-preference>
                <source-protocol>ietf-routing:static</source-protocol>
                <last-updated>2015-10-24T18:02:45+02:00</last-updated>
              </route>
            </routes>
          </rib>
        </ribs>
      </routing>
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
