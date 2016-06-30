import yangrpc
import yuma
import traceback
from lxml import etree

def yangcli(yangrpc_cb,cmd_line):

	(res, rpc_val) = yangrpc.parse_cli(yangrpc_cb, cmd_line)
	if(res!=0):
		raise NameError("Error: yangrpc failed to parse cli command!")
		return None

	(res, reply_val) = yangrpc.rpc(yangrpc_cb, rpc_val)
	if(res!=0):
		raise NameError("Error: yangrpc failed to execute rpc!")
		return None

	(res, reply_xml_str) = yuma.val_make_serialized_string(reply_val, yuma.NCX_DISPLAY_MODE_XML_NONS)
	if(res!=0):
		raise NameError("Error: yuma.val_make_serialized_string failed!")
		return None

	myetree = etree.fromstring(reply_xml_str)

	return myetree
