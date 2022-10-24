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
#Description: Read and edit anyxml.
#Procedure:
#1 - Read /c/a and validate
#2 - Replace /c/a read back and validate
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

	print("[OK] Receiving <hello> =%(reply_xml)s:" % {'reply_xml':reply_xml})
	conn=litenc_lxml.litenc_lxml(conn_raw)


	print("Connected ...")

	namespaces={"nc":"urn:ietf:params:xml:ns:netconf:base:1.0",
        "test-anyxml":"http://yuma123.org/ns/test/netconfd/anyxml/test-anyxml",
        "one":"urn:1",
        "two":"urn:2"}

	edit_config_rpc = """
<edit-config>
    <target>
      <candidate/>
    </target>
    <default-operation>merge</default-operation>
    <test-option>set</test-option>
    <config>
      <c xmlns="http://yuma123.org/ns/test/netconfd/anyxml/test-anyxml" xmlns:nc="urn:ietf:params:xml:ns:netconf:base:1.0" nc:operation="replace">
        <a>
          <one foo="blah" xmlns="http://yuma123.org/ns/test/netconfd/anyxml/test-anyxml"><two bar="blaer" xmlns="http://yuma123.org/ns/test/netconfd/anyxml/test-anyxml">2</two><!--<tree/>--></one>
        </a>
      </c>
    </config>
</edit-config>
"""
	commit_rpc = """
<commit/>
"""

	get_rpc="""
<get>
  <filter type="subtree">
    <c xmlns="http://yuma123.org/ns/test/netconfd/anyxml/test-anyxml">
      <a/>
    </c>
  </filter>
</get>
"""

        print("<edit-config> - /c/a ...")
        result = conn.rpc(edit_config_rpc)
	print(lxml.etree.tostring(result, pretty_print=True, inclusive_ns_prefixes=True))
        print("<commit> ...")
        result = conn.rpc(commit_rpc)
	print(lxml.etree.tostring(result, pretty_print=True, inclusive_ns_prefixes=True))
        print("<get> - /c/a ...")
        result = conn.rpc(get_rpc)

	print(lxml.etree.tostring(result, pretty_print=True, inclusive_ns_prefixes=True))
	one = result.xpath("./nc:data/test-anyxml:c/test-anyxml:a/test-anyxml:one", namespaces=namespaces)
	assert(len(one)==1)

sys.exit(main())
