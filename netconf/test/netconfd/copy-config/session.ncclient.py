#!/usr/bin/env python
from ncclient import manager
from ncclient.xml_ import *
import time
import sys, os

def main():
	print("""
#Description: Demonstrate that copy-config works.
#Procedure:
#1 - Copy configuration with single interface named "bar" to 'candidate' and commit.
#2 - Verify get-config returns the interface named "bar" in <data>
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
<copy-config xmlns="urn:ietf:params:xml:ns:netconf:base:1.0">
 <target>
  <candidate/>
 </target>
 <source>
  <config>
   <interfaces xmlns="urn:ietf:params:xml:ns:yang:ietf-interfaces">
    <interface>
     <name>bar</name>
     <type xmlns:ianaift="urn:ietf:params:xml:ns:yang:iana-if-type">ianaift:ethernetCsmacd</type>
    </interface>
   </interfaces>
  </config>
 </source>
</copy-config>
"""
	print("copy-config ...")
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
	print(names[0].text)
	assert(names[0].text=='bar')


sys.exit(main())
