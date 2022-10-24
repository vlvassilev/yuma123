#!/usr/bin/python

"""
Build a simple network from scratch according to topology.xml.

      eth0   eth0 eth1   eth0 eth1   eth0
  +----+      +----+      +----+      +----+
  | h0 |------| b0 |------| b1 |------| h1 |
  +----+      +----+      +----+      +----+

netconfd instance is started on each node with modules
according to the node type.

Node types:

 * h - host (--module=ietf-interfaces --module=ietf-traffic-generator --module=ietf-traffic-analyzer)
 * b - bridge (--module=ietf-network-bridge-openflow)
 * r - router (--module=ietf-interfaces --module=ietf-ip --module=ietf-routing)

After the network is started configuration is commited and connectivity tested.
"""

from mininet.net import Mininet
from mininet.node import Node
from mininet.link import Link
from mininet.log import setLogLevel, info
from mininet.util import quietRun

from time import sleep
import tntapi
import lxml
from lxml import etree

namespaces={"nc":"urn:ietf:params:xml:ns:netconf:base:1.0",
        "nd":"urn:ietf:params:xml:ns:yang:ietf-network",
        "nt":"urn:ietf:params:xml:ns:yang:ietf-network-topology"}

def netconfNet():
    "Create network from scratch using netconfd instance on each node."

    info( "*** Creating nodes\n" )
    h0 = Node( 'h0', inNamespace=True)
    h1 = Node( 'h1', inNamespace=True)
    b0 = Node( 'b0', inNamespace=False)
    b1 = Node( 'b1', inNamespace=False)

    ncproxy = Node( 'ncproxy', inNamespace=False)

    info( "*** Creating links\n" )
    b0.cmd("ip link del b0-eth0")
    b0.cmd("ip link del b0-eth1")
    b1.cmd("ip link del b1-eth0")
    b1.cmd("ip link del b1-eth1")
    Link( h0, b0, intfName2="b0-eth0")
    Link( b0, b1, intfName1="b0-eth1", intfName2="b1-eth0")
    Link( h1, b1, intfName2="b1-eth1")

    info( "*** Configuring hosts\n" )
    h0.setIP( '192.168.123.1/24' )
    h1.setIP( '192.168.123.2/24' )
    info( str( h0 ) + '\n' )
    info( str( h1 ) + '\n' )
    info( str( b0 ) + '\n' )
    info( str( b1 ) + '\n' )

    info( "*** Starting network\n" )
    h0.cmd( '''INTERFACE_NAME_PREFIX=h0- ./run-netconfd --module=ietf-interfaces --no-startup --port=8832 --ncxserver-sockname=/tmp/ncxserver.8832.sock --superuser=${USER} &''' )
    ncproxy.cmd( '''./run-sshd-for-netconfd --port=8832 --ncxserver-sockname=/tmp/ncxserver.8832.sock &''' )
    h1.cmd( '''INTERFACE_NAME_PREFIX=h1- ./run-netconfd --module=ietf-interfaces --no-startup --port=8833 --ncxserver-sockname=/tmp/ncxserver.8833.sock --superuser=${USER} &''' )
    ncproxy.cmd( '''./run-sshd-for-netconfd --port=8833 --ncxserver-sockname=/tmp/ncxserver.8833.sock &''' )

    #bridge b0
    b0.cmd( '''VCONN_ARG=ptcp:16636 ./run-netconfd --module=ietf-network-bridge-openflow --no-startup --superuser=${USER} --port=8830 --ncxserver-sockname=/tmp/ncxserver.8830.sock &''' )
    b0.cmd( 'ovs-vsctl del-br dp0' )
    b0.cmd( 'ovs-vsctl add-br dp0' )
    for intf in list(b0.intfs.values()):
        print("Setting: "+ str(intf))
        print(b0.cmd( 'ovs-vsctl add-port dp0 %s' % intf ))
        print(b0.cmd( 'ifconfig -a' ))

    # Note: controller and switch are in root namespace, and we
    # can connect via loopback interface
    b0.cmd( 'ovs-vsctl set-controller dp0 tcp:127.0.0.1:16636' )

    info( '*** Waiting for switch to connect to controller' )
    while 'is_connected' not in quietRun( 'ovs-vsctl show' ):
        sleep( 1 )
        info( '.' )
    info( '\n' )

    #bridge b1
    b1.cmd( '''VCONN_ARG=ptcp:16635 ./run-netconfd --module=ietf-network-bridge-openflow --no-startup --superuser=${USER} --port=8831 --ncxserver-sockname=/tmp/ncxserver.8831.sock &''' )
    b1.cmd( 'ovs-vsctl del-br dp1' )
    b1.cmd( 'ovs-vsctl add-br dp1' )
    for intf in list(b1.intfs.values()):
        print("Setting: "+ str(intf))
        print(b1.cmd( 'ovs-vsctl add-port dp1 %s' % intf ))
        print(b1.cmd( 'ifconfig -a' ))

    # Note: controller and switch are in root namespace, and we
    # can connect via loopback interface
    b1.cmd( 'ovs-vsctl set-controller dp1 tcp:127.0.0.1:16635' )

    sleep(10)
    input("Press Enter to continue...")


    tree=etree.parse("topology.xml")
    network = tree.xpath('/nc:config/nd:networks/nd:network', namespaces=namespaces)[0]

    conns = tntapi.network_connect(network)
    yconns = tntapi.network_connect_yangrpc(network)
    mylinks = tntapi.parse_network_links(network)
    print("Done")

    state_before = tntapi.network_get_state(network, conns)
    info( "*** Running test\n" )

    #create flows configuration
    yangcli_script="""
merge /bridge/ports/port -- name=b0-eth0
merge /interfaces/interface -- name=b0-eth0 type=ethernetCsmacd port-name=b0-eth0
merge /bridge/ports/port -- name=b0-eth1
merge /interfaces/interface -- name=b0-eth1 type=ethernetCsmacd port-name=b0-eth1
create /flows/flow[id='h0-to-h1'] -- match/in-port=b0-eth0 actions/action[order='0']/output-action/out-port=b0-eth1
create /flows/flow[id='h1-to-h0'] -- match/in-port=b0-eth1 actions/action[order='0']/output-action/out-port=b0-eth0
"""
    result=tntapi.yangcli_ok_script(yconns["b0"],yangcli_script)

    yangcli_script="""
merge /bridge/ports/port -- name=b1-eth0
merge /interfaces/interface -- name=b1-eth0 type=ethernetCsmacd port-name=b1-eth0
merge /bridge/ports/port -- name=b1-eth1
merge /interfaces/interface -- name=b1-eth1 type=ethernetCsmacd port-name=b1-eth1
create /flows/flow[id='h0-to-h1'] -- match/in-port=b1-eth0 actions/action[order='0']/output-action/out-port=b1-eth1
create /flows/flow[id='h1-to-h0'] -- match/in-port=b1-eth1 actions/action[order='0']/output-action/out-port=b1-eth0
"""
    result=tntapi.yangcli_ok_script(yconns["b1"],yangcli_script)
    tntapi.network_commit(conns)

    # The commit should cause the netconfd server to execute the OpenFlow equivalent of:
    #switch.cmd( 'ovs-ofctl add-flow dp0 in_port=2,actions=output:1' )
    #switch.cmd( 'ovs-ofctl add-flow dp0 in_port=1,actions=output:2' )

    b0.cmdPrint( 'ovs-ofctl dump-flows dp0' )


    info( "*** Running test\n" )
    h0.cmdPrint( 'ping -I h0-eth0 -c10 ' + h1.IP() )

    b0.cmdPrint( 'ovs-ofctl dump-flows dp0' )

    state_after = tntapi.network_get_state(network, conns)
    #delta = tntapi.get_network_counters_delta(state_before,state_after)

    tntapi.print_state_ietf_interfaces_statistics_delta(network, state_before, state_after)

    input("Press Enter to continue...")

    info( "*** Stopping network\n" )
    b0.cmd( 'killall -KILL netconfd' )
    b0.cmd( 'ovs-vsctl del-br dp0' )
    b0.deleteIntfs()
    b1.cmd( 'killall -KILL netconfd' )
    b1.cmd( 'ovs-vsctl del-br dp1' )
    b1.deleteIntfs()
    info( '\n' )

if __name__ == '__main__':
    setLogLevel( 'info' )
    info( '*** NETCONF network demo\n' )
    Mininet.init()
    netconfNet()
