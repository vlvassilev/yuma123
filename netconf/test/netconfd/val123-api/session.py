from yangcli import yangcli
import lxml
import yangrpc
import sys
import os

conn = yangrpc.connect("127.0.0.1", 830, os.getenv('USER'), None, os.getenv('HOME')+"/.ssh/id_rsa.pub", os.getenv('HOME')+"/.ssh/id_rsa",)
if(conn==None):
    print("Error: yangrpc failed to connect!")
    sys.exit(1)

result=yangcli(conn, "xget /system-state/platform")
print lxml.etree.tostring(result)
assert("foo"==result.xpath('./data/system-state/platform/os-name')[0].text)
assert("bar"==result.xpath('./data/system-state/platform/os-release')[0].text)

print("Done.")
