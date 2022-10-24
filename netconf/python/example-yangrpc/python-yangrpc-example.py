import yuma
import yangrpc
import sys

conn = yangrpc.connect("127.0.0.1", 830, "root", "mysecretpass","/root/.ssh/id_rsa","/root/.ssh/id_rsa.pub")
if(conn==None):
	print("Error: yangrpc failed to connect!")
	sys.exit(1)

(res, rpc_val) = yangrpc.parse_cli(conn, "xget /interfaces-state")
if(res!=0):
	print("Error: yangrpc failed to parse cli command!")
	sys.exit(1)

yuma.val_dump_value(rpc_val,1)

(res, reply_val) = yangrpc.rpc(conn, rpc_val)
if(res!=0):
	print("Error: yangrpc failed to execute rpc!")
	sys.exit(1)

yuma.val_dump_value(reply_val,1)

print("Done.")
