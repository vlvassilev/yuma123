from yangcli import yangcli
from lxml import etree
import yangrpc
import sys
import os

conn = yangrpc.connect("127.0.0.1", 830, os.getenv('USER'), None, os.getenv('HOME')+"/.ssh/id_rsa.pub", os.getenv('HOME')+"/.ssh/id_rsa",)
if(conn==None):
    print("Error: yangrpc failed to connect!")
    sys.exit(1)


yangcli(conn, "replace /system/hostname value='ok3'")
yangcli(conn, "replace /system/location value='ok4'")
result=yangcli(conn, "commit")
assert(result.xpath('./ok'))

print("Done.")
