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
#Description: Connect to opendaylight and <get> /.
#Procedure:
#1 - Print the <hello>
#2 - Print the operational state.
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

	# receive <hello>
	result=conn.receive()

	print("#1 - Print the <hello>")
	print lxml.etree.tostring(result)

        print("#2 - Print the operational state.")

	get_rpc = """
<get xmlns="urn:ietf:params:xml:ns:netconf:base:1.0">
</get>
"""
	print("get / ...")
	result = conn.rpc(get_rpc)

	print lxml.etree.tostring(result)

	delete_config_rpc = """
  <edit-config xmlns="urn:ietf:params:xml:ns:netconf:base:1.0">
    <target>
      <candidate/>
    </target>
    <default-operation>merge</default-operation>
    <config>
      <modules xmlns="urn:opendaylight:params:xml:ns:yang:controller:config">
        <module
          xmlns:nc="urn:ietf:params:xml:ns:netconf:base:1.0"
          nc:operation="delete">
          <type
            xmlns:sal-netconf="urn:opendaylight:params:xml:ns:yang:controller:md:sal:connector:netconf">sal-netconf:sal-netconf-connector</type>
          <name>left</name>
        </module>
      </modules>
    </config>
  </edit-config>
"""
	print("delete-config ...")
	result = conn.rpc(delete_config_rpc)

	commit_rpc = """
<commit/>
"""
	print("commit ...")
	result = conn.rpc(commit_rpc)
	print lxml.etree.tostring(result)
	ok = result.xpath('ok')
	assert(len(ok)==1)

	edit_config_rpc = """
  <edit-config xmlns="urn:ietf:params:xml:ns:netconf:base:1.0">
    <target>
      <candidate/>
    </target>
    <default-operation>merge</default-operation>
    <config>
      <modules xmlns="urn:opendaylight:params:xml:ns:yang:controller:config">
        <module
          xmlns:nc="urn:ietf:params:xml:ns:netconf:base:1.0"
          nc:operation="merge">
          <type
            xmlns:sal-netconf="urn:opendaylight:params:xml:ns:yang:controller:md:sal:connector:netconf">sal-netconf:sal-netconf-connector</type>
          <name>left</name>
          <address xmlns="urn:opendaylight:params:xml:ns:yang:controller:md:sal:connector:netconf">127.0.0.1</address>
          <port xmlns="urn:opendaylight:params:xml:ns:yang:controller:md:sal:connector:netconf">3830</port>
          <tcp-only xmlns="urn:opendaylight:params:xml:ns:yang:controller:md:sal:connector:netconf">false</tcp-only>
          <username xmlns="urn:opendaylight:params:xml:ns:yang:controller:md:sal:connector:netconf">demo</username>
          <password xmlns="urn:opendaylight:params:xml:ns:yang:controller:md:sal:connector:netconf">demo</password>
  <event-executor xmlns="urn:opendaylight:params:xml:ns:yang:controller:md:sal:connector:netconf">
    <type xmlns:prefix="urn:opendaylight:params:xml:ns:yang:controller:netty">prefix:netty-event-executor</type>
    <name>global-event-executor</name>
  </event-executor>
  <binding-registry xmlns="urn:opendaylight:params:xml:ns:yang:controller:md:sal:connector:netconf">
    <type xmlns:prefix="urn:opendaylight:params:xml:ns:yang:controller:md:sal:binding">prefix:binding-broker-osgi-registry</type>
    <name>binding-osgi-broker</name>
  </binding-registry>
  <dom-registry xmlns="urn:opendaylight:params:xml:ns:yang:controller:md:sal:connector:netconf">
    <type xmlns:prefix="urn:opendaylight:params:xml:ns:yang:controller:md:sal:dom">prefix:dom-broker-osgi-registry</type>
    <name>dom-broker</name>
  </dom-registry>
  <client-dispatcher xmlns="urn:opendaylight:params:xml:ns:yang:controller:md:sal:connector:netconf">
    <type xmlns:prefix="urn:opendaylight:params:xml:ns:yang:controller:config:netconf">prefix:netconf-client-dispatcher</type>
    <name>global-netconf-dispatcher</name>
  </client-dispatcher>
  <processing-executor xmlns="urn:opendaylight:params:xml:ns:yang:controller:md:sal:connector:netconf">
    <type xmlns:prefix="urn:opendaylight:params:xml:ns:yang:controller:threadpool">prefix:threadpool</type>
    <name>global-netconf-processing-executor</name>
  </processing-executor>
  <keepalive-executor xmlns="urn:opendaylight:params:xml:ns:yang:controller:md:sal:connector:netconf">
    <type xmlns:prefix="urn:opendaylight:params:xml:ns:yang:controller:threadpool">prefix:scheduled-threadpool</type>
    <name>global-netconf-ssh-scheduled-executor</name>
  </keepalive-executor>
        </module>
      </modules>
    </config>
  </edit-config>
"""


	print("edit-config ...")
	result = conn.rpc(edit_config_rpc)
	print lxml.etree.tostring(result)
	ok = result.xpath('ok')
	assert(len(ok)==1)

	commit_rpc = """
<commit/>
"""
	print("commit ...")
	result = conn.rpc(commit_rpc)
	print lxml.etree.tostring(result)
	ok = result.xpath('ok')
	assert(len(ok)==1)

        return(0)

sys.exit(main())
