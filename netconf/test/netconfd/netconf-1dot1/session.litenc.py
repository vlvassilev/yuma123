#!/usr/bin/env python

import time
import sys, os
sys.path.append("../../litenc")
import socket
import paramiko
import litenc
import litenc_lxml
import lxml
import argparse

#Steps follow:

def step_1(server, port, user, password):
	print("#1")
	print("Connecting ...")
	conn_raw = litenc.litenc()
	ret = conn_raw.connect(server=server, port=port, user=user, password=password)
	if ret != 0:
		print("[FAILED] Connecting to server=%(server)s:" % {'server':server})
		assert(0)
	print("Connected ...")
	return (conn_raw)

def step_2(conn_raw):
	print("#2")
	time.sleep(5)
	try:
		rx_data = conn_raw.chan.recv(1000000)
	except socket.timeout:
		print("Timeout!")
		assert(0)
	#print the received <hello> from server
	print(rx_data)

	#intentionally send <hello> and <rpc> without delay
	tx_data = """
<?xml version="1.0" encoding="UTF-8"?>
<hello xmlns:nc="urn:ietf:params:xml:ns:netconf:base:1.0"
  xmlns="urn:ietf:params:xml:ns:netconf:base:1.0">
  <capabilities>
    <capability>urn:ietf:params:netconf:base:1.0</capability>
    <capability>urn:ietf:params:netconf:base:1.1</capability>
  </capabilities>
</hello>]]>]]>
#181
<?xml version="1.0" encoding="UTF-8"?>
<rpc message-id="1"
  xmlns="urn:ietf:params:xml:ns:netconf:base:1.0">
  <get xmlns="urn:ietf:params:xml:ns:netconf:base:1.0">
  </get>
</rpc>
##
"""
	print(tx_data)
	data=tx_data
	try:
		while data:
			n = conn_raw.chan.send(data)
			if n <= 0:
				return -1
			data = data[n:]
	except Exception as e:
		print("Exception while sending.")
		assert(0)


	time.sleep(5)
	print("Receiving ...")
	try:
		rx_data = conn_raw.chan.recv(1000000)
	except socket.timeout:
		print("Timeout!")
		assert(0)

	print(rx_data)

	print("Done.")

def main():
	print("""
#Description: Test implementation of RFC6241 framing
#Procedure:
#1 - Connect.
#2 - Validate NETCONF1.1 framed RPCs are accepted and processed correctly.
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


	conn_raw=step_1(server=server, port=port, user=user, password=password)
	step_2(conn_raw)
	return 0

sys.exit(main())
