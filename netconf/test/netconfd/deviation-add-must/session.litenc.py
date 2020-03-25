#!/usr/bin/env python

import time
import sys, os
sys.path.append("../../litenc")
import litenc
import litenc_lxml
import lxml

def main():
	print("""
#Description: Demonstrate that edit-config works.
#Procedure:
#1 - Create interface "foo" and commit. Verify commit succeeds.
#2 - Create interface "bar" and commit. Verify commit fails.
""")

	port=830
	server="127.0.0.1"

	conn_raw = litenc.litenc()
	ret = conn_raw.connect(server=server, port=port, user=os.getenv('USER'))
	if ret != 0:
		print "[FAILED] Connecting to server=%(server)s:" % {'server':server}
		return(-1)
	print "[OK] Connecting to server=%(server)s:" % {'server':server}
	conn=litenc_lxml.litenc_lxml(conn_raw)
	ret = conn_raw.send("""
<hello>
 <capabilities>
  <capability>urn:ietf:params:netconf:base:1.0</capability>\
 </capabilities>\
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

	print("edit-config - create 'bar' ...")
	result = conn.rpc(edit_rpc%{'interface':"bar"})

	print("commit ...")
	result = conn.rpc(commit_rpc)
	ok = result.xpath('//ok')
	assert(len(ok)!=1)

sys.exit(main())
