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
#Description: Demonstrate that deref() works.
#Procedure:
#1 - Create interfaces foo enabled=true and bar enabled=false.
#2 - Validate management-interface/name=foo can be created.
#3 - Validate management-interface/name=bar can not be created.
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
		print("[FAILED] Connecting to server=%(server)s:" % {'server':server})
		return(-1)
	print("[OK] Connecting to server=%(server)s:" % {'server':server})
	conn=litenc_lxml.litenc_lxml(conn_raw, strip_namespaces=True)
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

	print("[OK] Receiving <hello> =%(reply_xml)s:" % {'reply_xml':reply_xml})


	print("Connected ...")

	edit_config_rpc = """
<edit-config>
    <target>
      <candidate/>
    </target>
    <default-operation>merge</default-operation>
    <test-option>set</test-option>
    <config>
      <interfaces xmlns="urn:ietf:params:xml:ns:yang:ietf-interfaces" xmlns:nc="urn:ietf:params:xml:ns:netconf:base:1.0" nc:operation="delete"/>
    </config>
  </edit-config>
"""
	print("edit-config - delete /interfaces ...")
	result = conn.rpc(edit_config_rpc)

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
    <config>
      <interfaces xmlns="urn:ietf:params:xml:ns:yang:ietf-interfaces">
        <interface xmlns:nc="urn:ietf:params:xml:ns:netconf:base:1.0" nc:operation="create">
          <name>foo</name>
          <type
            xmlns:ianaift="urn:ietf:params:xml:ns:yang:iana-if-type">ianaift:ethernetCsmacd</type>
          <enabled>true</enabled>
        </interface>
        <interface xmlns:nc="urn:ietf:params:xml:ns:netconf:base:1.0" nc:operation="create">
          <name>bar</name>
          <type
            xmlns:ianaift="urn:ietf:params:xml:ns:yang:iana-if-type">ianaift:ethernetCsmacd</type>
          <enabled>false</enabled>
        </interface>
      </interfaces>
    </config>
  </edit-config>
"""
	print("edit-config - create  'foo and bar' ...")
	result = conn.rpc(edit_config_rpc)
	print(lxml.etree.tostring(result))
	ok = result.xpath('ok')
	print(result)
	print(ok)
	print(lxml.etree.tostring(result))
	assert(len(ok)==1)

	commit_rpc = """
<commit/>
"""
	print("commit ...")
	result = conn.rpc(commit_rpc)
	print(lxml.etree.tostring(result))
	ok = result.xpath('//ok')
	assert(len(ok)==1)

	edit_config_rpc = """
<edit-config>
    <target>
      <candidate/>
    </target>
    <default-operation>merge</default-operation>
    <test-option>set</test-option>
    <config>
        <mgmt-interface xmlns="http://yuma123.org/ns/test-xpath-deref"><name>foo</name><type xmlns:ianaift="urn:ietf:params:xml:ns:yang:iana-if-type">ianaift:ethernetCsmacd</type></mgmt-interface>
    </config>
  </edit-config>
"""
	print("edit-config - create mgmt-interface=foo ...")
	result = conn.rpc(edit_config_rpc)
	print(lxml.etree.tostring(result))
	ok = result.xpath('//ok')
	assert(len(ok)==1)

	print("commit ...")
	result = conn.rpc(commit_rpc)
	print(lxml.etree.tostring(result))
	ok = result.xpath('//ok')
	assert(len(ok)==1)
	result = conn.rpc(commit_rpc)
	print(lxml.etree.tostring(result))
	ok = result.xpath('//ok')
	assert(len(ok)==1)

	edit_config_rpc = """
<edit-config>
    <target>
      <candidate/>
    </target>
    <default-operation>merge</default-operation>
    <test-option>set</test-option>
    <config>
        <mgmt-interface xmlns="http://yuma123.org/ns/test-xpath-deref"><name>foo</name><type xmlns:ianaift="urn:ietf:params:xml:ns:yang:iana-if-type">ianaift:ethernetCsmacd</type></mgmt-interface>
    </config>
  </edit-config>
"""
	print("edit-config - delete mgmt-interface ...")
	result = conn.rpc(edit_config_rpc)

	print("commit ...")
	result = conn.rpc(commit_rpc)
	print(lxml.etree.tostring(result))
	ok = result.xpath('//ok')
	assert(len(ok)==1)

	edit_config_rpc = """
<edit-config>
    <target>
      <candidate/>
    </target>
    <default-operation>merge</default-operation>
    <test-option>set</test-option>
    <config>
        <mgmt-interface xmlns="http://yuma123.org/ns/test-xpath-deref"><name>bar</name><type>ianaift:ethernetCsmacd</type></mgmt-interface>
    </config>
  </edit-config>
"""
	print("edit-config - create mgmt-interface=bar ...")
	result = conn.rpc(edit_config_rpc)
	print(lxml.etree.tostring(result))
	ok = result.xpath('//ok')
	assert(len(ok)==1)

	print("commit ...")
	result = conn.rpc(commit_rpc)
	print(lxml.etree.tostring(result))
	ok = result.xpath('//ok')
	assert(len(ok)!=1)

sys.exit(main())
