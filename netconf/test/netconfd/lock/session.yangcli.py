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
	conn = yangrpc.connect(server, port, user, password, os.getenv('HOME')+"/.ssh/id_rsa.pub", os.getenv('HOME')+"/.ssh/id_rsa", "--dump-session=tmp/nc-session-")
	if(conn==None):
		print("Error: yangrpc failed to connect!")
		return(-1)

	return conn

def lock(conn):
	result = yangcli(conn, "lock target=candidate")
	print(lxml.etree.tostring(result))
	ok = result.xpath('./ok')

	assert(len(ok)==0 or len(ok)==1)
	if(len(ok)==1):
		return True
	else:
		return False

def unlock(conn):
	result = yangcli(conn, "unlock target=candidate")
	print(lxml.etree.tostring(result))
	ok = result.xpath('./ok')

	assert(len(ok)==0 or len(ok)==1)
	if(len(ok)==1):
		return True
	else:
		return False

def commit(conn):
	result = yangcli(conn, "commit")
	print(lxml.etree.tostring(result))
	ok = result.xpath('./ok')

	assert(len(ok)==0 or len(ok)==1)
	if(len(ok)==1):
		return True
	else:
		return False

#Steps follow:

def step_1(server, port, user, password):
	print("#1 - Create 2 sessions.")
	print("Connecting #1 ...")
	conn_1 = connect(server=server, port=port, user=user, password=password)
	print("Connecting #2 ...")
	conn_2 = connect(server=server, port=port, user=user, password=password)

	print("Connected ...")
	return (conn_1, conn_2)

def step_2(conn_1,conn_2):
	print("#2 - Lock candidate from session #1.")
	assert(lock(conn_1))

def step_3(conn_1,conn_2):
	print("#3 - Try to lock candidate from session #2. Validate this fails.")
	assert(not(lock(conn_2)))


def step_4(conn_1,conn_2):
	print("#4 - Try to modify candidate from session #2. Validate this fails.")
	line="create /interfaces/interface -- name=foo type=ethernetCsmacd"
	result = yangcli(conn_2, line)
	print(lxml.etree.tostring(result))
	ok = result.xpath('./ok')

	assert(len(ok)==0)

def step_5(conn_1,conn_2):
	print("#5 - Modify candidate from session #1 confirm this is OK.")
	line="create /interfaces/interface -- name=foo type=ethernetCsmacd"
	result = yangcli(conn_1, line)
	print(lxml.etree.tostring(result))
	ok = result.xpath('./ok')

	assert(len(ok)==1)

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


	(conn_1, conn_2)=step_1(server=server, port=port, user=user, password=password)
	step_2(conn_1,conn_2)
	step_3(conn_1,conn_2)
	step_4(conn_1,conn_2)
	step_5(conn_1,conn_2)
	step_6(conn_1,conn_2)
	#step_7(conn_1,conn_2)
	step_8(conn_1,conn_2)
	step_9(conn_1,conn_2)
	step_10(conn_1,conn_2)
	return 0

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
