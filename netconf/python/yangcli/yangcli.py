import yangrpc
import yuma
import traceback
from lxml import etree

def yangcli(yangrpc_cb,cmd_line,strip_namespaces=True):

	(res, rpc_val) = yangrpc.parse_cli(yangrpc_cb, cmd_line)
	if(res!=0):
		raise NameError("Error: yangrpc failed to parse cli command!")
		return None

	(res, reply_val) = yangrpc.rpc(yangrpc_cb, rpc_val)
	if(res!=0):
		raise NameError("Error: yangrpc failed to execute rpc!")
		return None

	if(strip_namespaces):
		display_mode=yuma.NCX_DISPLAY_MODE_XML_NONS
	else:
		display_mode=yuma.NCX_DISPLAY_MODE_XML

	(res, reply_xml_str) = yuma.val_make_serialized_string(reply_val, display_mode)
	if(res!=0):
		raise NameError("Error: yuma.val_make_serialized_string failed!")
		return None

	yuma.val_free_value(reply_val);

	myetree = etree.fromstring(reply_xml_str)

	return myetree
