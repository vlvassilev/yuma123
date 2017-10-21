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
#Description: Using sub-interfaces configure vlan bridge.
#Procedure:
#1 - Create interface "xe0", "ge0", "ge1" of type=ethernetCsmacd.
#2 - Create sub-interface "xe0-green" - s-vlan-id=1000, "ge1-green" - c-vlan-id=10 of type=ethSubInterface.
#3 - Create VLAN bridge with "ge0", "ge1-green" and "xe0-green".
#4 - Commit.
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

	commit_rpc = """
<commit/>
"""
	print("commit ...")
	result = conn.rpc(commit_rpc)

	edit_config_rpc = """
<edit-config>
   <target>
     <candidate/>
   </target>
   <default-operation>merge</default-operation>
   <test-option>set</test-option>
<config xmlns="urn:ietf:params:xml:ns:netconf:base:1.0">
  <interfaces xmlns="urn:ietf:params:xml:ns:yang:ietf-interfaces">
    <interface>
      <name>ge0</name>
      <type
        xmlns:ianaift="urn:ietf:params:xml:ns:yang:iana-if-type">ianaift:ethernetCsmacd</type>
    </interface>
    <interface>
      <name>ge1</name>
      <type
        xmlns:ianaift="urn:ietf:params:xml:ns:yang:iana-if-type">ianaift:ethernetCsmacd</type>
    </interface>
    <interface>
      <name>ge1-green</name>
      <type
        xmlns:if-cmn="urn:ietf:params:xml:ns:yang:ietf-interfaces-common">if-cmn:ethSubInterface</type>
      <encapsulation xmlns="urn:ietf:params:xml:ns:yang:ietf-interfaces-common">
        <flexible xmlns="urn:ietf:params:xml:ns:yang:ietf-flexible-encapsulation">
          <match>
            <dot1q-vlan-tagged>
              <outer-tag>
                <dot1q-tag>
                  <tag-type
                    xmlns:dot1q-types="urn:ieee:std:802.1Q:yang:ieee802-dot1q-types">dot1q-types:c-vlan</tag-type>
                  <vlan-id>10</vlan-id>
                </dot1q-tag>
              </outer-tag>
            </dot1q-vlan-tagged>
          </match>
        </flexible>
      </encapsulation>
      <forwarding-mode xmlns="urn:ietf:params:xml:ns:yang:ietf-interfaces-common" xmlns:if-cmn="urn:ietf:params:xml:ns:yang:ietf-interfaces-common">if-cmn:layer-2-forwarding</forwarding-mode>
      <parent-interface xmlns="urn:ietf:params:xml:ns:yang:ietf-interfaces-common">ge0</parent-interface>
    </interface>
    <interface>
      <name>xe0</name>
      <type
        xmlns:ianaift="urn:ietf:params:xml:ns:yang:iana-if-type">ianaift:ethernetCsmacd</type>
    </interface>
    <interface>
      <name>xe0-green</name>
      <type
        xmlns:if-cmn="urn:ietf:params:xml:ns:yang:ietf-interfaces-common">if-cmn:ethSubInterface</type>
      <encapsulation xmlns="urn:ietf:params:xml:ns:yang:ietf-interfaces-common">
        <flexible xmlns="urn:ietf:params:xml:ns:yang:ietf-flexible-encapsulation">
          <match>
            <dot1q-vlan-tagged>
              <outer-tag>
                <dot1q-tag>
                  <tag-type
                    xmlns:dot1q-types="urn:ieee:std:802.1Q:yang:ieee802-dot1q-types">dot1q-types:s-vlan</tag-type>
                  <vlan-id>1000</vlan-id>
                </dot1q-tag>
              </outer-tag>
              <second-tag>
                <dot1q-tag>
                  <tag-type
                    xmlns:dot1q-types="urn:ieee:std:802.1Q:yang:ieee802-dot1q-types">dot1q-types:c-vlan</tag-type>
                  <vlan-id>10</vlan-id>
                </dot1q-tag>
              </second-tag>
            </dot1q-vlan-tagged>
          </match>
        </flexible>
      </encapsulation>
      <forwarding-mode xmlns="urn:ietf:params:xml:ns:yang:ietf-interfaces-common" xmlns:if-cmn="urn:ietf:params:xml:ns:yang:ietf-interfaces-common">if-cmn:layer-2-forwarding</forwarding-mode>
      <parent-interface xmlns="urn:ietf:params:xml:ns:yang:ietf-interfaces-common">xe0</parent-interface>
    </interface>
  </interfaces>
  <nacm xmlns="urn:ietf:params:xml:ns:yang:ietf-netconf-acm"/>
  <vlans xmlns="http://example.com/ns/vlans">
    <vlan>
      <name>green</name>
      <interface>ge0</interface>
      <interface>ge1-green</interface>
      <interface>xe0-green</interface>
    </vlan>
  </vlans>
</config>
 </edit-config>
"""
	print("edit-config - create vlan ...")
	result = conn.rpc(edit_config_rpc)
	print lxml.etree.tostring(result)
	ok = result.xpath('ok')
	print result
	print ok
	print lxml.etree.tostring(result)
	assert(len(ok)==1)

	commit_rpc = """
<commit/>
"""
	print("commit ...")
	result = conn.rpc(commit_rpc)
	print lxml.etree.tostring(result)
	assert(len(ok)==1)

	edit_config_rpc = """
<edit-config>
   <target>
     <candidate/>
   </target>
   <default-operation>merge</default-operation>
   <test-option>set</test-option>
 <config>
  <interfaces xmlns="urn:ietf:params:xml:ns:yang:ietf-interfaces" xmlns:nc="urn:ietf:params:xml:ns:netconf:base:1.0" nc:operation="delete">
  </interfaces>
  <vlans xmlns="http://example.com/ns/vlans" xmlns:nc="urn:ietf:params:xml:ns:netconf:base:1.0" nc:operation="delete">
  </vlans>
 </config>
 </edit-config>
"""
	print("edit-config - clean-up ...")
	result = conn.rpc(edit_config_rpc)
	print lxml.etree.tostring(result)
	ok = result.xpath('ok')
	print result
	print ok
	print lxml.etree.tostring(result)
	assert(len(ok)==1)

	commit_rpc = """
<commit/>
"""
	print("commit ...")
	result = conn.rpc(commit_rpc)
	print lxml.etree.tostring(result)
	assert(len(ok)==1)


sys.exit(main())
