#!/usr/bin/env python
from ncclient import manager
from ncclient.xml_ import *
from ncclient.operations.rpc import RPCError
import time
import sys, os

def main():
	print("""
#Description: Demonstrate that edit-config works.
#Procedure:
#1 - Create interface "foo" and commit. Verify commit succeeds.
#2 - Create interface "bar" and commit. Verify commit fails.
""")

	conn = manager.connect(host="127.0.0.1", port=830, username=os.getenv('USER'), password='admin', look_for_keys=True, timeout=10, device_params = {'name':'junos'}, hostkey_verify=False)
	print("Connected ...")

	edit_rpc = """
<edit-config xmlns="urn:ietf:params:xml:ns:netconf:base:1.0">
 <target>
  <candidate/>
 </target>
 <default-operation>merge</default-operation>
 <test-option>set</test-option>
 <config>
  <interfaces xmlns="urn:ietf:params:xml:ns:yang:ietf-interfaces">
   <interface xmlns:nc="urn:ietf:params:xml:ns:netconf:base:1.0" nc:operation="create">
     <name>%(interface)s</name>
     <type xmlns:ianaift="urn:ietf:params:xml:ns:yang:iana-if-type">ianaift:ethernetCsmacd</type>
   </interface>
  </interfaces>
 </config>
</edit-config>
"""
	print("edit-config - create 'foo' ...")
	result = conn.rpc(edit_rpc%{'interface':"foo"})
	ok = result.xpath('//ok')
	assert(len(ok)==1)

	commit_rpc = """
<commit/>
"""
	print("commit ...")
	result = conn.rpc(commit_rpc)

	print("edit-config - create 'bar' ...")
	result = conn.rpc(edit_rpc%{'interface':"bar"})

	print("commit ...")
	result = conn.rpc(commit_rpc)
	ok = result.xpath('//ok')
	assert(len(ok)!=1)

sys.exit(main())
