#!/usr/bin/env python

import time
import sys, os
sys.path.append("../../litenc")
import litenc
import litenc_lxml
import lxml
import argparse

def connect(server, port, user, password):
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
	return conn_raw

def lock(conn):
	lock_rpc = """
       <lock>
         <target>
           <candidate/>
         </target>
       </lock>
"""
	result = conn.rpc(lock_rpc)
	ok = result.xpath('ok')
	print lxml.etree.tostring(result)

	assert(len(ok)==0 or len(ok)==1)
	if(len(ok)==1):
		return True
	else:
		return False

def unlock(conn):
	unlock_rpc = """
       <unlock>
         <target>
           <candidate/>
         </target>
       </unlock>
"""
	result = conn.rpc(unlock_rpc)
	ok = result.xpath('ok')
	print lxml.etree.tostring(result)

	assert(len(ok)==0 or len(ok)==1)
	if(len(ok)==1):
		return True
	else:
		return False

def commit():
	commit_rpc = """
<commit/>
"""
	print("commit ...")
	result = conn.rpc(commit_rpc)
	ok = result.xpath('ok')
	print lxml.etree.tostring(result)
	assert(len(ok)==0 or len(ok)==1)
	if(len(ok)==1):
		return True
	else:
		return False

#Steps follow:

def step_1(server, port, user, password):
	print("#1 - Create 2 sessions.")
	print("Connecting #1 ...")
	conn_raw_1 = connect(server=server, port=port, user=user, password=password)
	conn_1=litenc_lxml.litenc_lxml(conn_raw_1)
	print("Connecting #2 ...")
	conn_raw_2 = connect(server=server, port=port, user=user, password=password)
	conn_2=litenc_lxml.litenc_lxml(conn_raw_2)

	print("Connected ...")
	return (conn_raw_1, conn_1, conn_raw_2, conn_2)

def step_2(conn_1,conn_2):
	print("#2 - Lock candidate from session #1.")
	assert(lock(conn_1))

def step_3(conn_1,conn_2):
	print("#3 - Try to lock candidate from session #2. Validate this fails.")
	assert(not(lock(conn_2)))


def step_4(conn_1,conn_2):
	print("#4 - Try to modify candidate from session #2. Validate this fails.")
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
        </interface>
      </interfaces>
    </config>
  </edit-config>
"""
	print("edit-config - create single 'foo' ...")
	result = conn_2.rpc(edit_config_rpc)
	ok = result.xpath('ok')
	print result
	print lxml.etree.tostring(result)
	assert(len(ok)==0)

def step_5(conn_1,conn_2):
	print("#5 - Modify candidate from session #1 confirm this is OK.")
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
        </interface>
      </interfaces>
    </config>
  </edit-config>
"""
	print("edit-config - create single 'foo' ...")
	result = conn_1.rpc(edit_config_rpc)
	ok = result.xpath('ok')
	print result
	print lxml.etree.tostring(result)
	assert(len(ok)==1)
	return

def step_6(conn_1,conn_2):
	print("#6 - Unlock from #1")
	assert(unlock(conn_1))

def step_7(conn_1,conn_2):
	print("#7 - Try to lock candidate from session #2. Validate this fails.")
	assert(not (lock(conn_2)))

def step_8(conn_1,conn_2):
	print("#8 - Commit")
	assert(commit(conn_1))

def step_9(conn_1,conn_2):
	print("#9 - Lock candidate from session #2.")
	assert(lock(conn_2))

def step_10(conn_1,conn_2):
	print("#10 - Try to unlock candidate from session #1. Validate this fails.")
	assert(not (lock(conn_1)))

def step_11(conn_1,conn_2):
	print("#11 - Unlock candidate from session #2.")
	assert(unlock(conn_2))

def main():
	print("""
#Description: Test implementation of <lock> RPC rfc6241#section-7.5
#Procedure:
#1 - Create 2 sessions.
#2 - Lock candidate from session #1.
#3 - Try to lock candidate from session #2. Validate this fails.
#4 - Try to modify candidate from session #2. Validate this fails.
#5 - Modify candidate from session #1 confirm this is OK.
#6 - Unlock from #1
#7 - Try to lock candidate from session #2. Validate this fails.
#8 - Commit
#9 - Lock candidate from session #2.
#10 - Try to unlock candidate from session #1. Validate this fails.
#11 - Unlock candidate from session #2.
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


	(conn_raw_1, conn_1, conn_raw_2, conn_2)=step_1(server=server, port=port, user=user, password=password)
	step_2(conn_1,conn_2)
	step_3(conn_1,conn_2)
	step_4(conn_1,conn_2)
	step_5(conn_1,conn_2)
	step_6(conn_1,conn_2)
	step_7(conn_1,conn_2)
	step_8(conn_1,conn_2)
	step_9(conn_1,conn_2)
	step_10(conn_1,conn_2)
	return 0

sys.exit(main())
