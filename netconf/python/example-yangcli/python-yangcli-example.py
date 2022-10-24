from yangcli.yangcli import yangcli
from lxml import etree
import yangrpc
import sys

conn = yangrpc.connect("127.0.0.1", 830, "root", "mysecretpass","/root/.ssh/id_rsa","/root/.ssh/id_rsa.pub")
if(conn==None):
    print("Error: yangrpc failed to connect!")
    sys.exit(1)


names = yangcli(conn, "xget /interfaces-state").xpath('./data/interfaces-state/interface/name')

for name in names:
	print(name.text)

print("Done.")
