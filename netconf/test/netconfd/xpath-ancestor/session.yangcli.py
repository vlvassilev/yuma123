#!/usr/bin/env python

import time
import sys, os
import argparse
from yangcli import yangcli
from lxml import etree
import yangrpc

def main():
	print("""
#Description: Validate xpath ancestor axis
#Procedure:
#1 - Create /a/value = foo.
#2 - Validate creating '/a/b/c/value = bar' fails.
#3 - Validate creating '/a/b/c/value = foo' works.

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

	ok = yangcli(conn, '''create /a/value value=foo''').xpath('./ok')
	assert(len(ok)==1)

	ok = yangcli(conn, '''commit''').xpath('./ok')
	assert(len(ok)==1)

	ok = yangcli(conn, '''create /a/b/c/value value=bar''').xpath('./ok')
	assert(len(ok)==1)
	ok = yangcli(conn, '''commit''').xpath('./ok')
	assert(len(ok)==0)
	ok = yangcli(conn, '''discard-changes''').xpath('./ok')
	assert(len(ok)==1)


	ok = yangcli(conn, '''create /a/b/c/value value=foo''').xpath('./ok')
	assert(len(ok)==1)
	ok = yangcli(conn, '''commit''').xpath('./ok')
	assert(len(ok)==1)

	print("done")
	return(0)
sys.exit(main())
