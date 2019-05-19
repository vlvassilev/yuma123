#!/usr/bin/python

"""
Build a simple network from scratch, using mininet primitives.
This is more complicated than using the higher-level classes,
but it exposes the configuration details and allows customization.

For most tasks, the higher-level API will be preferable.
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

def scratchNet( cname='ovs-testcontroller', cargs='-v ptcp:16635' ):
    "Create network from scratch using Open vSwitch."

    info( "*** Creating nodes\n" )
    #controller = Node( 'c0', inNamespace=False)
    switch = Node( 's0', inNamespace=False)
    h0 = Node( 'h0' )
    h1 = Node( 'h1' )

    info( "*** Creating links\n" )
    Link( h0, switch )
    Link( h1, switch )

    info( "*** Configuring hosts\n" )
    h0.setIP( '192.168.123.1/24' )
    h1.setIP( '192.168.123.2/24' )
    info( str( h0 ) + '\n' )
    info( str( h1 ) + '\n' )
    info( str( switch ) + '\n' )

    info( "*** Starting network using Open vSwitch\n" )
    switch.cmd( cname + ' ' + cargs + '&' )
    switch.cmd( 'ovs-vsctl del-br dp0' )
    switch.cmd( 'ovs-vsctl add-br dp0' )
    for intf in switch.intfs.values():
        print switch.cmd( 'ovs-vsctl add-port dp0 %s' % intf )

    # Note: controller and switch are in root namespace, and we
    # can connect via loopback interface
    switch.cmd( 'ovs-vsctl set-controller dp0 tcp:127.0.0.1:16635' )

    info( '*** Waiting for switch to connect to controller' )
    while 'is_connected' not in quietRun( 'ovs-vsctl show' ):
        sleep( 1 )
        info( '.' )
    info( '\n' )

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
merge /bridge/ports/port -- name=s0-eth0
merge /interfaces/interface -- name=s0-eth0 type=ethernetCsmacd port-name=s0-eth0
merge /bridge/ports/port -- name=s0-eth1
merge /interfaces/interface -- name=s0-eth1 type=ethernetCsmacd port-name=s0-eth1
create /flows/flow[id='h0-to-h1'] -- match/in-port=s0-eth0 actions/action[order='0']/output-action/out-port=s0-eth1
create /flows/flow[id='h1-to-h0'] -- match/in-port=s0-eth1 actions/action[order='0']/output-action/out-port=s0-eth0
"""
    result=tntapi.yangcli_ok_script(yconns["s0"],yangcli_script)
    tntapi.network_commit(conns)

    info( "*** Running test\n" )
    h0.cmdPrint( 'ping -c1 ' + h1.IP() )

    state_after = tntapi.network_get_state(network, conns)
    #delta = tntapi.get_network_counters_delta(state_before,state_after)

    tntapi.print_state_ietf_interfaces_statistics_delta(network, state_before, state_after)

    info( "*** Stopping network\n" )
    #controller.cmd( 'kill %' + cname )
    switch.cmd( 'ovs-vsctl del-br dp0' )
    switch.deleteIntfs()
    info( '\n' )

if __name__ == '__main__':
    setLogLevel( 'info' )
    info( '*** Scratch network demo (kernel datapath)\n' )
    Mininet.init()
    scratchNet( cname='./run-netconfd', cargs="--module=ietf-network-bridge-openflow --no-startup --port=8830 --ncxserver-sockname=/tmp/ncxserver.8830.sock --superuser=${USER}")
    #scratchNet( cname='ovs-testcontroller', cargs='-v ptcp:16635')


