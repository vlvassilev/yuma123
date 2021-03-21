#!/usr/bin/env python

import time
import sys, os
import argparse
from yangcli import yangcli
from lxml import etree
import yangrpc

def main():

	parser = argparse.ArgumentParser()
	parser.add_argument("--server", help="server name e.g. 127.0.0.1 or server.com (127.0.0.1 if not specified)")
	parser.add_argument("--user", help="username e.g. admin ($USER if not specified)")
	parser.add_argument("--port", help="port e.g. 830 (830 if not specified)")
	parser.add_argument("--password", help="password e.g. mypass123 (passwordless if not specified)")

	parser.add_argument("--operation", help="One of {configure-nacm, create, replace, read}")
	parser.add_argument("--option", help="One of {foo, bar, baz}")

	args = parser.parse_args()

	print("""
#Description: Validate nacm --operation=%(operation)s --option=%(option)s
#Procedure:
#1 - Do %(operation)s.
""" % {'operation': args.operation, 'option':args.option})


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

	if(args.operation == 'configure-nacm'):
		ok = yangcli(conn, '''create /nacm/groups/group[name='my-operators-group']/user-name value=%s''' % (user)).xpath('./ok')
		assert(len(ok)==1)
		ok = yangcli(conn, '''create /nacm/rule-list[name='my-operators-rules'] -- group=my-operators-group''').xpath('./ok')
		assert(len(ok)==1)
		ok = yangcli(conn, '''create /nacm/rule-list[name='my-operators-rules']/rule[name='my-foo-all-permit'] -- module-name=m1 path=/top/foo access-operations=* action=permit comment='With this rule the user can create /top/foo' ''').xpath('./ok')
		assert(len(ok)==1)
		ok = yangcli(conn, '''create /nacm/rule-list[name='my-operators-rules']/rule[name='my-baz-read-deny'] -- module-name=m1 path=/top/baz access-operations=read action=deny comment='With this rule the user can not read /top/baz' ''').xpath('./ok')
		assert(len(ok)==1)

		ok = yangcli(conn, '''commit''').xpath('./ok')
		assert(len(ok)==1)
	elif(args.operation == 'get'):
		result = yangcli(conn, '''sget /top/%s''' % (args.option))
		value = result.xpath('./data/top/%s' % (args.option))
		print etree.tostring(result, pretty_print=True, inclusive_ns_prefixes=True)
		assert(len(value)==1)
		assert(value[0].text==args.option)
	else:
		ok = yangcli(conn, '''%s /top/%s value=%s''' % (args.operation, args.option, args.option)).xpath('./ok')
		assert(len(ok)==1)
		ok = yangcli(conn, '''commit''').xpath('./ok')
		assert(len(ok)==1)

	print("done")
	return(0)
sys.exit(main())
