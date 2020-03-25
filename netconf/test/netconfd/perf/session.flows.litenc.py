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
#Description: Create a number of list entries in edit-config and delete them.
#Procedure:
#1 - Create specified interface count (or none if --if-count=0) and commit. Verify commit succeeds.
#2 - Delete the interfaces and commit. Verify commit succeeds.
""")

	parser = argparse.ArgumentParser()
	parser.add_argument("--server", help="server name e.g. 127.0.0.1 or server.com (127.0.0.1 if not specified)")
	parser.add_argument("--user", help="username e.g. admin ($USER if not specified)")
	parser.add_argument("--port", help="port e.g. 830 (830 if not specified)")
	parser.add_argument("--password", help="password e.g. mypass123 (passwordless if not specified)")

	parser.add_argument("--connections-count", help="count of connections to be opened and closed in sequence for each completing <hello> negotiation and creating --interfaces-count interfaces in configuration before closing the session")
	parser.add_argument("--interfaces-count", help="interface count to create, commit and delete (0= just connect and terminate connection) starting from if0 e.g. if0, if1 ... if1000 ...")
	parser.add_argument("--skip-hello", help="true - only ssh session is established but no <hello> messages are sent or received")
	parser.add_argument("--bridge-flows-enable", help="true - in addition to interfaces create flows for simple mac learning bridge unicast forwarding table interfaces_count*(interfaces_count-1) flows")

	args = parser.parse_args()


	for x in range(int(args.connections_count)):
		ret=session(args)
		assert(ret==0)
	return(0)

def session(args):
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

	if(args.skip_hello=="true"):
		conn_raw.close()
		return(0)

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

	#print "[OK] Receiving <hello> =%(reply_xml)s:" % {'reply_xml':reply_xml}


	print("Connected ...")

	if(int(args.interfaces_count)>0):
		edit_config(conn,args)

	conn_raw.close()
	return(0)

def edit_config(conn,args):

	edit_config_rpc = """
<edit-config>
    <target>
      <candidate/>
    </target>
    <default-operation>merge</default-operation>
    <test-option>test-then-set</test-option>
    <config>
%s
    </config>

  </edit-config>
"""%(edit_config_create(conn,args))

	print("edit-config ...")
	print(edit_config_rpc)
	result = conn.rpc(edit_config_rpc)
	#print lxml.etree.tostring(result)
	ok = result.xpath('ok')
	#print result
	#print ok
	print lxml.etree.tostring(result)
	assert(len(ok)==1)

	commit_rpc = """
<commit/>
"""
	print("commit ...")
	result = conn.rpc(commit_rpc)
	ok = result.xpath('ok')
	assert(len(ok)==1)


	edit_config_rpc = """
<edit-config>
    <target>
      <candidate/>
    </target>
    <default-operation>merge</default-operation>
    <test-option>set</test-option>
    <config>
%s
    </config>
  </edit-config>
"""%(edit_config_delete(conn,args))

	print("edit-config - delete /interfaces ...")
	result = conn.rpc(edit_config_rpc)
	#print lxml.etree.tostring(result)
	ok = result.xpath('ok')
	assert(len(ok)==1)

	print("commit ...")
	result = conn.rpc(commit_rpc)
	#print lxml.etree.tostring(result)
	ok = result.xpath('//ok')
	assert(len(ok)==1)

def edit_config_create(conn,args):

	config="""
<interfaces xmlns="urn:ietf:params:xml:ns:yang:ietf-interfaces">
"""
	if(args.bridge_flows_enable!="true"):
		for index in range(0,int(args.interfaces_count)):
			config=config+"""
        <interface xmlns:nc="urn:ietf:params:xml:ns:netconf:base:1.0" nc:operation="create">
          <name>%s</name>
          <type
            xmlns:ianaift="urn:ietf:params:xml:ns:yang:iana-if-type">ianaift:ethernetCsmacd</type>
        </interface>
"""%("if"+str(index))

		config=config+"""
</interfaces>
"""
		return(config)

	for index in range(0,int(args.interfaces_count)):
		config=config+"""
        <interface xmlns:nc="urn:ietf:params:xml:ns:netconf:base:1.0" nc:operation="create">
          <name>%s</name>
          <type
            xmlns:ianaift="urn:ietf:params:xml:ns:yang:iana-if-type">ianaift:ethernetCsmacd</type>
          <port-name xmlns="urn:ietf:params:xml:ns:yang:ietf-network-bridge">%s</port-name>
        </interface>
"""%("if"+str(index), "if"+str(index))

	config=config+"""
</interfaces>
"""


	config=config+"""
<bridge xmlns="urn:ietf:params:xml:ns:yang:ietf-network-bridge">
  <ports>
"""
	for index in range(0,int(args.interfaces_count)):
		config=config+"""
    <port><name>%s</name></port>
"""%("if"+str(index))
	config=config+"""
  </ports>
</bridge>
"""

	config=config+"""
<flows xmlns="urn:ietf:params:xml:ns:yang:ietf-network-bridge-flows">
"""

	for index_src in range(0,int(args.interfaces_count)):
		for index_dst in range(0,int(args.interfaces_count)):
			if(index_src==index_dst):
				continue
			config=config+"""
    <flow>
      <id>%s</id>
      <match>
        <in-port>%s</in-port>
        <ethernet-match>
          <ethernet-source>
            <address>%s</address>
          </ethernet-source>
          <ethernet-destination>
            <address>%s</address>
          </ethernet-destination>
        </ethernet-match>
      </match>
      <actions>
        <action>
          <order>0</order>
          <output-action>
            <out-port>%s</out-port>
          </output-action>
        </action>
      </actions>
    </flow>
"""%("flow-"+str(index_src)+"-to-"+str(index_dst),"if"+str(index_src),index2mac(index_src),index2mac(index_dst),"if"+str(index_dst))

	config=config+"""
</flows>
"""
	return(config)
def edit_config_delete(conn,args):

	config="""
<interfaces xmlns="urn:ietf:params:xml:ns:yang:ietf-interfaces" xmlns:nc="urn:ietf:params:xml:ns:netconf:base:1.0" nc:operation="delete"/>
"""
	if(args.bridge_flows_enable=="true"):
		config=config+"""
<bridge xmlns="urn:ietf:params:xml:ns:yang:ietf-network-bridge" xmlns:nc="urn:ietf:params:xml:ns:netconf:base:1.0" nc:operation="delete"/>
<flows xmlns="urn:ietf:params:xml:ns:yang:ietf-network-bridge-flows" xmlns:nc="urn:ietf:params:xml:ns:netconf:base:1.0" nc:operation="delete"/>
"""
	return config

def index2mac(index):
	#dummy for now
	return("00:00:00:00:00:00")

sys.exit(main())
