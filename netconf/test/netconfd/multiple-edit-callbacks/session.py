from yangcli.yangcli import yangcli
from lxml import etree
import yangrpc
import sys
import os

conn = yangrpc.connect("127.0.0.1", 830, os.getenv('USER'), None, os.getenv('HOME')+"/.ssh/id_rsa.pub",os.getenv('HOME')+"/.ssh/id_rsa")
if(conn==None):
	print("Error: yangrpc failed to connect!")
	sys.exit(1)


yangcli(conn, "create /interfaces/interface[name='blah']/type value='ethernetCsmacd'")
result=yangcli(conn, "commit")
assert(result.xpath('./ok'))

result=yangcli(conn, "xget /interfaces")
a=result.xpath('./data/interfaces/interface[name=\'blah\']/a')
b=result.xpath('./data/interfaces/interface[name=\'blah\']/b')

print((a[0].text))
assert(a[0].text=="Hello from a!")
print((b[0].text))
assert(b[0].text=="Hello from b!")

print("Done.")
