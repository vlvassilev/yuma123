#!/usr/bin/env python

import time
import sys, os
import argparse
from yangcli import yangcli
#from lxml import etree
import lxml
import yangrpc

def yangcli_ok_script(conn, yangcli_script):
	for line in yangcli_script.splitlines():
		line=line.strip()
		if not line:
			continue
		print(("Executing: "+line))
		ok = yangcli(conn, line).xpath('./ok')
		assert(len(ok)==1)

def main():
	print("""
#Description: Testcase for OpenDaylight interoperability.
#Procedure:
#1 - Create topology.
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

	print("Connecting #1 ...")
	conn = yangrpc.connect(server, port, user, password, os.getenv('HOME')+"/.ssh/id_rsa.pub", os.getenv('HOME')+"/.ssh/id_rsa", "--dump-session=tmp/nc-session-")
	if(conn==None):
		print("Error: yangrpc failed to connect!")
		return(-1)


	result = yangcli(conn, "delete /modules/module[name='right'][type='sal-netconf-connector']")

	result = yangcli(conn, "commit")
	ok = result.xpath('./ok')
	print((lxml.etree.tostring(result)))
	assert(len(ok)==1)

	result = yangcli(conn, "merge /modules/module[name='right'][type='sal-netconf-connector'] -- address=127.0.0.1 sal-netconf:port=4830 tcp-only=false username=demo password=demo sal-netconf:event-executor/type=netty-event-executor sal-netconf:event-executor/name=global-event-executor sal-netconf:binding-registry/type=binding-broker-osgi-registry sal-netconf:binding-registry/name=binding-osgi-broker sal-netconf:dom-registry/type=dom-broker-osgi-registry sal-netconf:dom-registry/name=dom-broker sal-netconf:client-dispatcher/type=netconf-client-dispatcher sal-netconf:client-dispatcher/name=global-netconf-dispatcher sal-netconf:processing-executor/type=threadpool sal-netconf:processing-executor/name=global-netconf-processing-executor sal-netconf:keepalive-executor/type=scheduled-threadpool sal-netconf:keepalive-executor/name=global-netconf-ssh-scheduled-executor")
	ok = result.xpath('./ok')
	print((lxml.etree.tostring(result)))
	assert(len(ok)==1)
	result = yangcli(conn, "commit")
	ok = result.xpath('./ok')
	print((lxml.etree.tostring(result)))
	assert(len(ok)==1)


	return(0)
sys.exit(main())
