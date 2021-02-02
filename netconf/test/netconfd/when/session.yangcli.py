#!/usr/bin/env python

import time
import sys, os
import argparse
from yangcli import yangcli
from lxml import etree
import yangrpc

def main():
	print("""
#Description: Validate leafs with failing when statements are removed from candidate
#Procedure:
#1 - Create /top/type = bar.
#2 - Create '/top/foo'.
#3 - Validate '/top/foo' is not present
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
	print("hello")

	ok = yangcli(conn, '''create /top/child -- type=bar foo=1''').xpath('./ok')
	assert(len(ok)==1)

	result = yangcli(conn, "xget-config source=candidate /top")
	foo = result.xpath('./data/top/child/foo')

	assert(len(foo)==0)

	ok = yangcli(conn, '''create /top/child/bar value=1''').xpath('./ok')
	assert(len(ok)==1)

	print("done")
	return(0)
sys.exit(main())
