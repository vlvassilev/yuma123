#!/usr/bin/env python

import time
import sys, os
import argparse
from yangcli import yangcli
from lxml import etree
import yangrpc

def yangcli_ok_script(conn, yangcli_script):
	for line in yangcli_script.splitlines():
		line=line.strip()
		if not line:
			continue
		print("Executing: "+line)
		ok = yangcli(conn, line).xpath('./ok')
		assert(len(ok)==1)

def main():
	print("""
#Description: Usecase fo ietf-routing module.
#Procedure:
#1 - Create interface "eth0" of type=ethernetCsmacd with static ip=10.0.0.2/24 and static default route for destination-prefix=0.0.0.0/0 and next-hop=10.0.0.1.
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

        yangcli_script='''
create /interfaces/interface -- name=eth0 type=ethernetCsmacd description="Uplink to ISP."
create /interfaces/interface[name='eth0']/ipv4/address -- ip=192.0.2.1 prefix-length=24
create /interfaces/interface[name='eth0']/ipv4/forwarding value=enabled

create /routes/control-plane-protocols/control-plane-protocol -- name=st0 type=static
create /routes/control-plane-protocols/control-plane-protocol[name='st0']/static-routes/ipv4/route -- destination-prefix=0.0.0.0/0 next-hop/next-hop-address=192.0.2.2
commit
'''
	yangcli_ok_script(conn, yangcli_script)

	return(0)
sys.exit(main())
