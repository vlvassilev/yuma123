#!/usr/bin/env python

import time
import sys, os
import argparse
from yangcli.yangcli import yangcli
from lxml import etree
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
#Description: Using sub-interfaces configure vlan bridge.
#Procedure:
#1 - Create interface "xe0", "ge0", "ge1" of type=ethernetCsmacd.
#2 - Create sub-interface "xe0-green" - s-vlan-id=1000, "ge1-green" - c-vlan-id=10 of type=ethSubInterface.
#3 - Create VLAN bridge with "ge0", "ge1-green" and "xe0-green".
#4 - Commit.
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


	ok = yangcli(conn, '''delete /vlans''').xpath('./ok')
	ok = yangcli(conn, '''delete /interfaces''').xpath('./ok')
	ok = yangcli(conn, '''commit''').xpath('./ok')
	assert(len(ok)==1)

        yangcli_script='''
create /interfaces/interface -- name='xe0' type='ethernetCsmacd'
create /interfaces/interface -- name='ge0' type='ethernetCsmacd'
create /interfaces/interface -- name='ge1' type='ethernetCsmacd'
create /interfaces/interface -- name='ge1-green' type=ethSubInterface parent-interface=ge0
create /interfaces/interface[name='ge1-green']/forwarding-mode value=layer-2-forwarding
create /interfaces/interface[name='ge1-green']/encapsulation/flexible/match/dot1q-vlan-tagged/outer-tag/dot1q-tag -- tag-type=c-vlan vlan-id=10
create /interfaces/interface -- name='xe0-green' type=ethSubInterface parent-interface=xe0
create /interfaces/interface[name='xe0-green']/forwarding-mode value=layer-2-forwarding
create /interfaces/interface[name='xe0-green']/encapsulation/flexible/match/dot1q-vlan-tagged/outer-tag/dot1q-tag -- tag-type=s-vlan vlan-id=1000
create /interfaces/interface[name='xe0-green']/encapsulation/flexible/match/dot1q-vlan-tagged/second-tag/dot1q-tag -- tag-type=c-vlan vlan-id=10
create /vlans/vlan -- name=green
create /vlans/vlan[name='green']/interface value=ge0
create /vlans/vlan[name='green']/interface value=ge1-green
create /vlans/vlan[name='green']/interface value=xe0-green
commit
'''
	yangcli_ok_script(conn, yangcli_script)

	result = yangcli(conn, "xget /interfaces")
	names = result.xpath('./data/interfaces/interface/name')
	for name in names:
		print(name.text)
	types = result.xpath('./data/interfaces/interface/type')
	for type in types:
		print(type.text)

	#cleanup
#	ok = yangcli(conn, '''delete /vlans''').xpath('./ok')
#	assert(len(ok)==1)
#	ok = yangcli(conn, '''delete /interfaces''').xpath('./ok')
#	assert(len(ok)==1)
#	ok = yangcli(conn, '''commit''').xpath('./ok')
#	assert(len(ok)==1)


	return(0)
sys.exit(main())
