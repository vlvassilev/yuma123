#!/usr/bin/env python
from ncclient import manager
from ncclient.xml_ import *
import time
import sys, os
import argparse

def main():
	print("""
#Description: Demonstrate that edit-config works.
#Procedure:
#1 - Create interface "foo" and commit.
#2 - Verify get-config returns the interface named "foo" in <data>
#3 - Create interface "bar" and commit.
#4 - Verify get-config returns the interfaces named "bar" and "foo" in <data>
#5 - Delete interface "foo"  and commit.
#6 - Verify get-config returns the interface named "bar" only in <data>
#7 - Delete interface "bar"  and commit.
#8 - Verify get-config returns no interfaces in <data>
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
		look_for_keys=True
	else:
		password=args.password
		look_for_keys=False

	conn = manager.connect(host=server, port=port, username=user, password=password, look_for_keys=look_for_keys, timeout=10, device_params = {'name':'junos'}, hostkey_verify=False)
	print("Connected ...")

	rpc = """
<edit-config xmlns="urn:ietf:params:xml:ns:netconf:base:1.0">
 <target>
  <candidate/>
 </target>
 <default-operation>merge</default-operation>
 <test-option>set</test-option>
 <config>
  <interfaces xmlns="urn:ietf:params:xml:ns:yang:ietf-interfaces">
   <interface xmlns:nc="urn:ietf:params:xml:ns:netconf:base:1.0" nc:operation="create">
     <name>foo</name>
     <type xmlns:ianaift="urn:ietf:params:xml:ns:yang:iana-if-type">ianaift:ethernetCsmacd</type>
   </interface>
  </interfaces>
 </config>
</edit-config>
"""
	print("edit-config - create 'foo' ...")
	result = conn.rpc(rpc)

	rpc = """
<commit/>
"""
	print("commit ...")
	result = conn.rpc(rpc)

	rpc = """
<get-config  xmlns="urn:ietf:params:xml:ns:netconf:base:1.0">
 <source>
  <candidate/>
 </source>
 <filter type="xpath" select="/interfaces"/>
</get-config>
"""
	print("get-config ...")
	result = conn.rpc(rpc)

	names = result.xpath('//data/interfaces/interface/name')
	print((len(names)))
	assert(len(names)==1)
	print((names[0].text))
	assert(names[0].text=='foo')


	rpc = """
<edit-config xmlns="urn:ietf:params:xml:ns:netconf:base:1.0">
 <target>
  <candidate/>
 </target>
 <default-operation>merge</default-operation>
 <test-option>set</test-option>
 <config>
  <interfaces xmlns="urn:ietf:params:xml:ns:yang:ietf-interfaces">
   <interface xmlns:nc="urn:ietf:params:xml:ns:netconf:base:1.0" nc:operation="create">
     <name>bar</name>
     <type xmlns:ianaift="urn:ietf:params:xml:ns:yang:iana-if-type">ianaift:ethernetCsmacd</type>
   </interface>
  </interfaces>
 </config>
</edit-config>
"""
	print("edit-config - create 'bar' ...")
	result = conn.rpc(rpc)

	rpc = """
<commit/>
"""
	print("commit ...")
	result = conn.rpc(rpc)

	rpc = """
<get-config  xmlns="urn:ietf:params:xml:ns:netconf:base:1.0">
 <source>
  <running/>
 </source>
 <filter type="xpath" select="/interfaces"/>
</get-config>
"""
	print("get-config ...")
	result = conn.rpc(rpc)

	names = result.xpath('//data/interfaces/interface/name')
	print((len(names)))
	assert(len(names)==2)

	name = result.xpath('//data/interfaces/interface[name=\'bar\']/name')
	print((name[0].text))
	assert(name[0].text=='bar')

	name = result.xpath('//data/interfaces/interface[name=\'foo\']/name')
	print((name[0].text))
	assert(name[0].text=='foo')

	rpc = """
<edit-config xmlns="urn:ietf:params:xml:ns:netconf:base:1.0">
 <target>
  <candidate/>
 </target>
 <default-operation>merge</default-operation>
 <test-option>set</test-option>
 <config>
  <interfaces xmlns="urn:ietf:params:xml:ns:yang:ietf-interfaces">
   <interface xmlns:nc="urn:ietf:params:xml:ns:netconf:base:1.0" nc:operation="delete">
     <name>foo</name>
   </interface>
  </interfaces>
 </config>
</edit-config>
"""
	print("edit-config - delete 'foo' ...")
	result = conn.rpc(rpc)

	rpc = """
<commit/>
"""
	print("commit ...")
	result = conn.rpc(rpc)

	rpc = """
<get-config  xmlns="urn:ietf:params:xml:ns:netconf:base:1.0">
 <source>
  <running/>
 </source>
 <filter type="xpath" select="/interfaces"/>
</get-config>
"""
	print("get-config ...")
	result = conn.rpc(rpc)

	names = result.xpath('//data/interfaces/interface/name')
	print((len(names)))
	assert(len(names)==1)

	name = result.xpath('//data/interfaces/interface[name=\'bar\']/name')
	print((name[0].text))
	assert(name[0].text=='bar')

	rpc = """
<edit-config xmlns="urn:ietf:params:xml:ns:netconf:base:1.0">
 <target>
  <candidate/>
 </target>
 <default-operation>merge</default-operation>
 <test-option>set</test-option>
 <config>
  <interfaces xmlns="urn:ietf:params:xml:ns:yang:ietf-interfaces">
   <interface xmlns:nc="urn:ietf:params:xml:ns:netconf:base:1.0" nc:operation="delete">
     <name>bar</name>
   </interface>
  </interfaces>
 </config>
</edit-config>
"""
	print("edit-config - delete 'bar' ...")
	result = conn.rpc(rpc)

	rpc = """
<commit/>
"""
	print("commit ...")
	result = conn.rpc(rpc)

	rpc = """
<get-config  xmlns="urn:ietf:params:xml:ns:netconf:base:1.0">
 <source>
  <running/>
 </source>
 <filter type="xpath" select="/interfaces"/>
</get-config>
"""
	print("get-config ...")
	result = conn.rpc(rpc)

	names = result.xpath('//data/interfaces/interface/name')
	print((len(names)))
	assert(len(names)==0)


sys.exit(main())
