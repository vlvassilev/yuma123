#!/usr/bin/env python
from ncclient import manager
from ncclient.xml_ import *
import time
import sys, os

def main():
	print("""
#Description: Demonstrate that edit-config works.
#Procedure:
#1 - Create interface "bar" and commit.
#2 - Verify get-config returns the interfaces named "bar" and "foo" in <data>
#3 - Delete interface "foo"  and commit.
#4 - Verify get-config returns the interface named "bar" only in <data>
""")

	conn = manager.connect(host="127.0.0.1", port=830, username=os.getenv('USER'), password='admin', look_for_keys=True, timeout=10, device_params = {'name':'junos'}, hostkey_verify=False)
	print("Connected ...")

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
	print(len(names))
	assert(len(names)==1)
	print(names[0].text)
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
	print(len(names))
	assert(len(names)==2)

	name = result.xpath('//data/interfaces/interface[name=\'bar\']/name')
	print(name[0].text)
	assert(name[0].text=='bar')

	name = result.xpath('//data/interfaces/interface[name=\'foo\']/name')
	print(name[0].text)
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
	print(len(names))
	assert(len(names)==1)

	name = result.xpath('//data/interfaces/interface[name=\'bar\']/name')
	print(name[0].text)
	assert(name[0].text=='bar')


sys.exit(main())
