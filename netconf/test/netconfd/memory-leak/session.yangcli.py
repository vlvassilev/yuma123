#!/usr/bin/env python

import time
import sys, os
import argparse
from yangcli import yangcli
from lxml import etree
import yangrpc

def main():
	print("""Edit config, get and shutdown to test there are no memory leaks 
#Description: .
#Procedure:
#1 - Create interface "foo" of type=ethernetCsmacd. Verify commit succeeds.
#2 - Replace type for interface "foo" with type=other. Read back the type after commit.
#3 - <get> the entire root.
#4 - <shutdown> the netconf server.
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

	conn = yangrpc.connect(server, port, user, password, os.getenv('HOME')+"/.ssh/id_rsa.pub", os.getenv('HOME')+"/.ssh/id_rsa")
	if(conn==None):
		print("Error: yangrpc failed to connect!")
		return(-1)

	time.sleep(1)


	ok = yangcli(conn, '''delete /interfaces''').xpath('./ok')

	ok = yangcli(conn, '''commit''').xpath('./ok')
	assert(len(ok)==1)

	ok = yangcli(conn, '''create /interfaces/interface -- name='foo' type='ethernetCsmacd' ''').xpath('./ok')
	assert(len(ok)==1)

	ok = yangcli(conn, '''commit''').xpath('./ok')
	assert(len(ok)==1)

	result = yangcli(conn, "xget /interfaces")
	names = result.xpath('./data/interfaces/interface/name')
	for name in names:
		print(name.text)
	types = result.xpath('./data/interfaces/interface/type')
	for type in types:
		print(type.text)

	assert(len(types)==1)
	assert(types[0].text=='ianaift:ethernetCsmacd')

	ok = yangcli(conn, '''replace /interfaces/interface[name='foo']/type value='other' ''').xpath('./ok')
	assert(len(ok)==1)

	ok = yangcli(conn, '''commit''').xpath('./ok')
	assert(len(ok)==1)

	result = yangcli(conn, "xget /interfaces")
	names = result.xpath('./data/interfaces/interface/name')
	for name in names:
		print(name.text)
	types = result.xpath('./data/interfaces/interface/type')
	for type in types:
		print(type.text)

	assert(len(types)==1)
	assert(types[0].text=='ianaift:other')

	result = yangcli(conn, "xget /")

	ok = yangcli(conn, '''shutdown''').xpath('./ok')
	assert(len(ok)==1)

	return(0)
sys.exit(main())
