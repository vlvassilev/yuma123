#!/usr/bin/env python

import time
import sys, os
import argparse
from yangcli.yangcli import yangcli
import lxml
from lxml import etree
import yangrpc

def yangcli_ok_script(conn, yangcli_script):
	for line in yangcli_script.splitlines():
		line=line.strip()
		if not line:
			continue
		print("Executing: "+line)
		reply = yangcli(conn, line)
		print(lxml.etree.tostring(reply))
		ok=reply.xpath('./ok')
		assert(len(ok)==1)

def main():
	print("""
#Description: Usecase for ietf-traffic-generator and ietf-traffic-analyzer modules.
#Procedure:
#1 - Start traffic-analyzer on "veth-a1"
#2 - Start traffic-generator on "veth-b1" 1000 64 octet frames with minimal interframe gap
#3 - After 1 second validate 1000 test frames are registered by the traffic-analyzer
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

	conn = yangrpc.connect(server, port, user, password, os.getenv('HOME')+"/.ssh/id_rsa.pub", os.getenv('HOME')+"/.ssh/id_rsa", "--dump-session=tmp/nc-session-")
	if(conn==None):
		print("Error: yangrpc failed to connect!")
		return(-1)

	time.sleep(1)

	result = yangcli(conn, "delete /interfaces")
	result = yangcli(conn, "commit")
	ok = result.xpath('./ok')
	assert(len(ok)==1)


	yangcli_script='''
create /interfaces/interface -- name=veth2 type=ethernetCsmacd description="Analyzer interface in a link."
create /interfaces/interface[name='veth2']/traffic-analyzer
commit
'''
	yangcli_ok_script(conn, yangcli_script)

	yangcli_script='''
create /interfaces/interface -- name=veth1 type=ethernetCsmacd description="Generator interface in a link."
create /interfaces/interface[name='veth1']/traffic-generator -- frame-size=64 gap=20 total-frames=10000 frame-data="6c:a9:6f:00:00:02:6c:a9:6f:00:00:01:08:00:45:00:00:2e:d4:a5:00:00:0a:11:58:16:c0:00:02:01:c0:00:02:02:c0:20:00:07:00:1a:00:00:01:02:03:04:05:06:07:08:09:0a:0b:0c:0d:0e:0f:10:11:12"
commit
'''

	yangcli_ok_script(conn, yangcli_script)

	time.sleep(3)

	result_state = yangcli(conn, "xget /interfaces-state/interface[name='veth2']/statistics" )
	print(lxml.etree.tostring(result_state))
	result = yangcli(conn, "xget /interfaces/interface[name='veth2']/traffic-analyzer" )
	print(lxml.etree.tostring(result))

	testframe_pkts = result.xpath("//rpc-reply/data/interfaces/interface[name='veth2']/traffic-analyzer/state/testframe-statistics/pkts")
	print(testframe_pkts)
	assert((len(testframe_pkts)==1))
	assert(testframe_pkts[0].text == "10000")


	return(0)
sys.exit(main())
