<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN"
          "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml"
      xmlns:py="http://purl.org/kid/ns#"
      py:extends="'master.kid'">
  <head>
    <meta content="text/html; charset=utf-8"
	  http-equiv="Content-Type" py:replace="''"/>
    <link media="screen" href="/tg_widgets/turbogears.widgets/tabber/tabber.css"
	  type="text/css" rel="stylesheet"/>
    <title>Netconf Central NETCONF Documentation</title>
  </head>
  <body>
    <div class="ncdocs">
      <h1>NETCONF Protocol</h1>

      <img src="${tg.url('/static/images/netconf_layers.png')}" 
	   border="2" alt="netconf layers"/>
      <p>&nbsp;</p>

      <div>
	<h2>Base Protocol</h2>
	<p>
          The <b>Network Configuration Protocol</b> is defined in 
          <a href="http://tools.ietf.org/html/rfc4741">RFC 4741</a>,
	  and was published in December 2006, by the
	  <a href="http://www.ops.ietf.org/netconf/">NETCONF Working Group</a>
	  within the
	  <a href="http://www.ietf.org/">
	    Internet Engineering Task Force (IETF).
	  </a>   The XSD in RFC 4741 has been converted to
	  a YANG module, called
	  <a href="${tg.url('/modulereport/netconf')}">
	    netconf.yang.
	  </a>
	</p>
	<p>
	  NETCONF is a session-based network management protocol,
	  which uses XML-encoded remote procedure calls (RPCs)
	  and configuration data to manage network devices.
	</p>
	<p>
	  The mandatory transport protocol for NETCONF is
          <a href="http://tools.ietf.org/html/rfc4253">
	    The Secure Shell (SSH)
	  </a>,	and the details for implementing this transport
	  mapping are defined in 
          <a href="http://tools.ietf.org/html/rfc4742">RFC 4742</a>.
	  The default TCP port assigned for this mapping is 830.
	  A NETCONF agent implementation must listen for connections
	  to the 'netconf' subsystem on this port.
	</p>
	<p>
	  An agent may optionally support additional transport
	  mappings.  
	  <a href="http://tools.ietf.org/html/rfc4743">RFC 4743</a>
	  defines mappings to the
	  <a href="http://www.w3.org/TR/soap12-part1/">
	    Simple Object Access Protocol (SOAP).
	  </a>
	  Two different transport protocols are supported for SOAP.
	  The <a href="http://tools.ietf.org/html/rfc3080">
	    Blocks Extensible Exchange Protocol (BEEP)
	  </a> mapping, called 'SOAP over BEEP',
	  is defined in 
	  <a href="http://tools.ietf.org/html/rfc4227">RFC 4227</a>.
	  The default TCP port for this mapping is 833.
	  The <a href="http://tools.ietf.org/html/rfc2616">
	    Hypertext Transfer Protocol (HTTP)
	  </a> mapping is defined by BEEP.  NETCONF agents
	  must provide secure HTTP (HTTPS), by running HTTP
	  over the 
	  <a href="http://tools.ietf.org/html/rfc4346">
	    Transport Layer Security Protocol (TLS).
	  </a>.
	  The default TCP port for this mapping is 832.
	</p>
	<p>
	  The NETCONF protocol can also be run directly over BEEP.
	  This mapping is defined in
	  <a href="http://tools.ietf.org/html/rfc4744">RFC 4744</a>.
	  The default TCP port for this mapping is 831.
	</p>
      </div>
      <div>
	<h2>Notifications</h2>
	<p>
	  NETCONF has a Notification delivery mechanism that
	  will be published as an RFC soon.  The current
	  <a href="http://www.ietf.org/internet-drafts/draft-ietf-netconf-notification-12.txt">Internet draft</a> defines two new modules.
	  These have been converted to YANG, named
	  <a href="${tg.url('/modulereport/nc-notifications')}">
	    nc-notifications.yang
	  </a> and
	  <a href="${tg.url('/modulereport/notifications')}">
	    notifications.yang.
	  </a>
	</p>
      </div>
      <div>
	<h2>Resources</h2>
	<ul>
	  <li>
	    <a href="${tg.url('/slides/netconf/netconf_tutorial.html')}">Tutorial Slides (circa 2004)</a>
	  </li>
	  <li>
	    <a href="http://www.ietf.org/html.charters/netconf-charter.html">
	      NETCONF WG Charter
	    </a>
	  </li>
	  <li>
	    <a href="http://www3.tools.ietf.org/wg/netconf/trac/wiki">
	      NETCONF WG Wiki Page
	    </a>
	  </li>
	  <li>
	    <a href="http://www.rfc-editor.org/errata_search.php?rfc=4742">
	      Errata for RFC 4742 (NETCONF Over SSH)
	    </a>
	  </li>
	  <li>
	    <a href="http://www.iana.org/assignments/xml-registry/ns/netconf.txt">
	      IANA assignment for NETCONF namespace
	    </a>
	  </li>
	  <li>
	    <a href="http://www.iana.org/assignments/xml-registry/ns/netconf/base/1.0.txt">
	      IANA assignment for NETCONF base namespace
	    </a>
	  </li>
	  <li>
	    <a href="http://www.iana.org/assignments/xml-registry/ns/netconf/soap.txt">
	      IANA assignment for NETCONF over SOAP URI
	    </a>
	  </li>
	  <li>
	    <a href="http://www.iana.org/assignments/xml-registry/schema/netconf.xsd">
	      IANA version of the NETCONF protocol XSD
	    </a>
	  </li>
	</ul>
      </div>
    </div>
  </body>
</html>
