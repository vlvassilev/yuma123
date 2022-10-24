#!/usr/bin/env python

import time
import sys, os
import argparse
from yangcli.yangcli import yangcli
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

def connect(server, port, user, password):
	conn = yangrpc.connect(server, port, user, password, os.getenv('HOME')+"/.ssh/id_rsa.pub", os.getenv('HOME')+"/.ssh/id_rsa", "--dump-session=tmp/nc-session- --keep-session-model-copies-after-compilation=true")
	if(conn==None):
		print("Error: yangrpc failed to connect!")

	return conn

def commit(conn):
	result = yangcli(conn, "commit")
	print(lxml.etree.tostring(result))
	ok = result.xpath('./ok')

	assert(len(ok)==0 or len(ok)==1)
	if(len(ok)==1):
		return True
	else:
		return False

def discard(conn):
	result = yangcli(conn, "discard")
	print(lxml.etree.tostring(result))
	ok = result.xpath('./ok')

	assert(len(ok)==0 or len(ok)==1)
	if(len(ok)==1):
		return True
	else:
		return False

#Steps follow:

def step_1(server, port0, port1, user, password):
	print("#1 - Create 2 sessions.")

	print("Connecting #1 ...")
	conn_1 = connect(server=server, port=port0, user=user, password=password)
	assert(conn_1!=None)
	
	print("Connecting #2 ...")
	conn_2 = connect(server=server, port=port1, user=user, password=password)
	assert(conn_2!=None)

	print("Connected ...")
	return (conn_1, conn_2)

def step_2(conn_1,conn_2):
	print("#2 - Validate /interfaces/interface/foo can be created on session #1.")
	result = yangcli(conn_1, "create /interfaces/interface[name='foo'] -- type=ethernetCsmacd foo='hello'")
	print(lxml.etree.tostring(result))
	ok = result.xpath('./ok')
	assert(len(ok)==1)
		commit(conn_1)
	result = yangcli(conn_1, "delete /interfaces/interface[name='foo']")
	ok = result.xpath('./ok')
	assert(len(ok)==1)
		commit(conn_1)


def step_3(conn_1,conn_2):
	print("#3 - Validate /interfaces/interface/bar can be created on session #2.")
	result = yangcli(conn_2, "create /interfaces/interface[name='bar'] -- type=ethernetCsmacd bar='hello'")
	print(lxml.etree.tostring(result))
	ok = result.xpath('./ok')
	assert(len(ok)==1)
		commit(conn_2)
	result = yangcli(conn_2, "delete /interfaces/interface[name='bar']")
	ok = result.xpath('./ok')
	assert(len(ok)==1)
		commit(conn_2)

def step_4(conn_1,conn_2):
	print("#4 - Validate /interface/interface/bar can NOT be created on session #1.")
	(res, rpc_val) = yangrpc.parse_cli(conn_1, "create /interfaces/interface[name='bar'] -- type=ethernetCsmacd bar='hello'")
		assert(res!=0)

def step_5(conn_1,conn_2):
	print("#5 - Validate /interface/interface/foo can NOT be created on session #2.")
	(res, rpc_val) = yangrpc.parse_cli(conn_2, "create /interfaces/interface[name='foo'] -- type=ethernetCsmacd foo='hello'")
		assert(res!=0)

def step_6(conn_1,conn_2):
	print("#6 - Close session #1 and confirm 'bar' can still be created on session #2.")
	yangrpc.close(conn_1)
	result = yangcli(conn_2, "create /interfaces/interface[name='bar'] -- type=ethernetCsmacd bar='hello'")
	print(lxml.etree.tostring(result))
	ok = result.xpath('./ok')
	assert(len(ok)==1)
		commit(conn_2)
	result = yangcli(conn_2, "delete /interfaces/interface[name='bar']")
	ok = result.xpath('./ok')
	assert(len(ok)==1)
		commit(conn_2)


def main():
	print("""
#Description: Test multi-instance server and client implementation
#Procedure:
#1 - Create 2 sessions - one to each server instance.
#2 - Validate /interface/interface/foo can be created on session #1.
#3 - Validate /interface/interface/bar can be created on session #2.
#4 - Validate /interface/interface/bar can NOT be created on session #1.
#5 - Validate /interface/interface/foo can NOT be created on session #2.
#6 - Close session #1 and confirm 'bar' can still be created on session #2.
""")

	parser = argparse.ArgumentParser()
	parser.add_argument("--server", help="server name e.g. 127.0.0.1 or server.com (127.0.0.1 if not specified)")
	parser.add_argument("--user", help="username e.g. admin ($USER if not specified)")
	parser.add_argument("--port0", help="port0 e.g. 830 (830 if not specified)")
	parser.add_argument("--port1", help="port1 e.g. 1830 (1830 if not specified)")
	parser.add_argument("--password", help="password e.g. mypass123 (passwordless if not specified)")

	args = parser.parse_args()

	if(args.server==None or args.server==""):
		server="127.0.0.1"
	else:
		server=args.server

	if(args.port0==None or args.port0==""):
		port0=830
	else:
		port0=int(args.port0)

	if(args.port1==None or args.port1==""):
		port1=1830
	else:
		port1=int(args.port1)

	if(args.user==None or args.user==""):
		user=os.getenv('USER')
	else:
		user=args.user

	if(args.password==None or args.password==""):
		password=None
	else:
		password=args.password


	(conn_1, conn_2)=step_1(server=server, port0=port0, port1=port1, user=user, password=password)
	step_2(conn_1,conn_2)
	step_3(conn_1,conn_2)
	step_4(conn_1,conn_2)
	step_5(conn_1,conn_2)
	step_6(conn_1,conn_2)
	return 0
sys.exit(main())
