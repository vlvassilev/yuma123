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
#Description: Test instance-identifier built-in type.
#Procedure:
#1 - Create  /top/list[idx="4"] and /top/id-list[id="/top/list[idx=\"4\"]"].
#2 - Try to create /top/id-list[id="/top/list[idx=\"5\"]"].
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

	edit_config_rpc = """
<edit-config>
    <target>
      <candidate/>
    </target>
    <default-operation>merge</default-operation>
    <test-option>set</test-option>
    <config>
      <top xmlns="http://yuma123.org/ns/test-instance-identifier" xmlns:nc="urn:ietf:params:xml:ns:netconf:base:1.0" nc:operation="delete">
      </top>
    </config>
  </edit-config>
"""
	print("edit-config ...")
	result = conn.rpc(edit_config_rpc)

	commit_rpc = """
<commit/>
"""
	print("commit ...")
	result = conn.rpc(commit_rpc)
	ok = result.xpath('//ok')
	assert(len(ok)==1)

	print('''#1 - Create  /top/list[idx="4"] and /top/id-list[id="/top/list[idx=\"4\"]"].''')
	edit_config_rpc = """
<edit-config>
    <target>
      <candidate/>
    </target>
    <default-operation>merge</default-operation>
    <test-option>set</test-option>
    <config>
      <top xmlns="http://yuma123.org/ns/test-instance-identifier">
          <list xmlns:nc="urn:ietf:params:xml:ns:netconf:base:1.0" nc:operation="create">
              <idx>4</idx>
          </list>
          <id-list xmlns:nc="urn:ietf:params:xml:ns:netconf:base:1.0" nc:operation="create">
              <id xmlns:tii="http://yuma123.org/ns/test-instance-identifier">/tii:top/tii:list[tii:idx="4"]</id>
          </id-list>
      </top>
    </config>
  </edit-config>
"""
#          <id-list xmlns:nc="urn:ietf:params:xml:ns:netconf:base:1.0" nc:operation="create">
#              <id>/top/list[idx=4]</id>
#          </id-list>

	print("edit-config ...")
	result = conn.rpc(edit_config_rpc)
	print result
	ok = result.xpath('//ok')
	assert(len(ok)==1)

	commit_rpc = """
<commit/>
"""
	print("commit ...")
	result = conn.rpc(commit_rpc)
	ok = result.xpath('//ok')
	assert(len(ok)==1)

	print('''#2 - Try to create /top/id-list[id="/top/list[idx=\"5\"]"].''')

	edit_config_rpc = """
<edit-config>
    <target>
      <candidate/>
    </target>
    <default-operation>merge</default-operation>
    <test-option>set</test-option>
    <config>
      <top xmlns="http://yuma123.org/ns/test-instance-identifier">
          <id-list xmlns:nc="urn:ietf:params:xml:ns:netconf:base:1.0" nc:operation="create">
              <id xmlns:tii="http://yuma123.org/ns/test-instance-identifier">/tii:top/tii:list[tii:idx="5"]</id>
          </id-list>
      </top>
    </config>
  </edit-config>
"""

	print("edit-config ...")
	result = conn.rpc(edit_config_rpc)
	ok = result.xpath('//ok')
	assert(len(ok)==1)

	commit_rpc = """
<commit/>
"""
	print("commit ...")
	result = conn.rpc(commit_rpc)
	print result
	ok = result.xpath('//ok')
	assert(len(ok)==0)

	discard_changes_rpc = """
<discard-changes/>
"""
	print("discard-changes ...")
	result = conn.rpc(discard_changes_rpc)
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
      <top xmlns="http://yuma123.org/ns/test-instance-identifier" xmlns:nc="urn:ietf:params:xml:ns:netconf:base:1.0" nc:operation="delete">
      </top>
    </config>
  </edit-config>
"""
	print("edit-config ...")
	result = conn.rpc(edit_config_rpc)
	result = conn.rpc(commit_rpc)
	ok = result.xpath('//ok')

	commit_rpc = """
<commit/>
"""
	print("commit ...")
	result = conn.rpc(commit_rpc)
	ok = result.xpath('//ok')
	assert(len(ok)==1)
sys.exit(main())
