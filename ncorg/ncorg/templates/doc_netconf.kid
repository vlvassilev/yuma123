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
    <div class="sidebar">
      <table>
	<tr>
	  <td>
	    <b>Contents</b>
	  </td>
	</tr>
	<tr>
	  <td>
	    <a href="#summary">Summary</a>
	  </td>
	</tr>
	<tr>
	  <td>
	    <a href="#base">Base&nbsp;Protocol</a>
	  </td>
	</tr>
	<tr>
	  <td>
	    <a href="#sessions">Sessions</a>
	  </td>
	</tr>
	<tr>
	  <td>
	    <a href="#databases">Databases</a>
	  </td>
	</tr>
	<tr>
	  <td>
	    <a href="#operations">Operations</a>
	  </td>
	</tr>
	<tr>
	  <td>
	    <a href="#capabilities">Capabilities</a>
	  </td>
	</tr>
	<tr>
	  <td>
	    <a href="#notifications">Notifications</a>
	  </td>
	</tr>
	<tr>
	  <td>
	    <a href="#standards">RFCs</a>
	  </td>
	</tr>
	<tr>
	  <td>
	    <a href="#work">Internet&nbsp;Drafts</a>
	  </td>
	</tr>
	<tr>
	  <td>
	    <a href="#resources">Additional&nbsp;Resources</a>
	  </td>
	</tr>
      </table>
    </div>

    <div class="ncdocs">
      <h1>Network Configuration Protocol</h1>

      <img src="${tg.url('/static/images/netconf_functional_layers_web.png')}" 
	   border="0" alt="netconf layers"/>
      <p>&nbsp;</p>

      <a name="summary"/>
      <div>
	<br/>
	<h2>Summary</h2>
	<p>
	  [Note: This is not official documentation for the NETCONF
	  protocol.  Refer to the actual RFC specifications for authoritative
	  documentation.]
	</p>
	<p>
	  The Network Configuration Protocol (NETCONF) provides 
	  operators and application developers
	  with a standard framework and a
	  set of standard Remote Procedure Call (RPC) methods 
	  to manipulate the configuration of a network device.
	</p>
	<p>
	  It is designed to be a replacement for Command
	  Line Interface (CLI) based
	  programmatic interfaces, such as
	  <a target="_blank" href="http://www.perl.com/">Perl</a> + 
	  <a target="_blank" href="http://expect.nist.gov/">Expect</a> over 
	  <a target="_blank" href="http://tools.ietf.org/html/rfc4251">Secure Shell</a> (SSH).
	  The CLI is also used by humans, which increases the complexity
	  and reduces the predictability of the API for real 
	  application usage.
	</p>
	<p>
	  NETCONF is usually transported over the SSH protocol, using
	  the 'netconf' sub-system (similar to the 'sftp' sub-system).
	  and in many ways it
	  mimics the native proprietary CLI over SSH interface available in
	  the device.  However, it uses structured schema-driven data,
	  and provides detailed structured error return information,
	  which the CLI cannot provide.
	</p>
	<p>
	  The device configuration data, and the protocol itself, are
	  encoded with the 
	  <a target="_blank" href="http://www.w3.org/XML/">Extensible Markup Language</a>
	  (XML).  Standard
	  XML tools such as 
	  <a target="_blank" href="http://www.w3.org/TR/xpath">XML Path Language</a>
	  (XPath) are used to
	  provide retrieval of a particular subset configuration data.
	  All NETCONF messages are encoded in XML within
	  <a target="_blank" href="http://www.w3.org/TR/REC-xml-names/">XML Namespaces</a>.
	  The protocol messages usually contain user data in a different 
	  namespace
	  than the NETCONF protocol PDUs.
	</p>
	<p>
	  All NETCONF devices must allow the configuration data
	  to be locked, edited, saved, and unlocked.  In addition,
	  all modifications to the configuration data must be
	  saved across a reboot in non-volatile storage.
	</p>
	<p>
	  The protocol (and sometimes even the configuration data)
	  is conceptually partitioned, based on a 'capability'.
	  These capabilities are given unique identifiers and
	  advertised by the server when the client starts a
	  NETCONF session.
	</p>
	<p>
	  A capability can be thought of as an 'API contract'
	  between the server and the client.  It represents
	  a set of functionality that cannot be diminished
	  by other capabilities.  They can be nested
	  (i.e., one capability required in order to
	  support another) but they are always additive.
	</p>
	<p>
	  There are a core set of operations
	  that must always be supported by the server.
	  To use any additional optional operations, the client
	  should make sure that the server supports the capability
	  associated with that operation.
	</p>
      </div>

      <a name="base"/>
      <div>
	<br/>
	<h2>Base Protocol</h2>
	<p>
          The <b>Network Configuration Protocol</b> is defined in 
          <a target="_blank" href="http://tools.ietf.org/html/rfc4741">RFC 4741</a>,
	  and was published as a Proposed Standard in December 2006, by the
	  <a target="_blank" href="http://www.ietf.org/html.charters/netconf-charter.html">
	    NETCONF Working Group
	  </a>
	  within the
	  <a target="_blank" href="http://www.ietf.org/">
	    Internet Engineering Task Force</a> (IETF).
	</p>
	<p>
	  NETCONF is a session-based network management protocol,
	  which uses XML-encoded remote procedure calls (RPCs)
	  and configuration data to manage network devices.
	</p>
	<p>
	  The mandatory transport protocol for NETCONF is
          <a target="_blank" href="http://tools.ietf.org/html/rfc4253">
	    The Secure Shell Transport Layer Protocol</a> (SSH), 
	  and the details for implementing this transport
	  mapping are defined in 
          <a target="_blank" href="http://tools.ietf.org/html/rfc4742">RFC 4742</a>.
	  The default TCP port assigned for this mapping is 830.
	  A NETCONF server implementation must listen for connections
	  to the 'netconf' subsystem on this port.
	</p>
	<p>
	  A server may optionally support additional transport
	  mappings.  
	  <a target="_blank" href="http://tools.ietf.org/html/rfc4743">RFC 4743</a>
	  defines mappings to the
	  <a target="_blank" href="http://www.w3.org/TR/soap12-part1/">
	    Simple Object Access Protocol (SOAP).
	  </a>
	  Two different transport protocols are supported for SOAP.
	  The <a target="_blank" href="http://tools.ietf.org/html/rfc3080">
	    Blocks Extensible Exchange Protocol (BEEP)
	  </a> mapping, called 'SOAP over BEEP',
	  is defined in 
	  <a target="_blank" href="http://tools.ietf.org/html/rfc4227">RFC 4227</a>.
	  The default TCP port for this mapping is 833.
	  The <a target="_blank" href="http://tools.ietf.org/html/rfc2616">
	    Hypertext Transfer Protocol (HTTP)
	  </a> mapping is defined by BEEP.  NETCONF servers
	  must provide secure HTTP (HTTPS), by running HTTP
	  over the 
	  <a target="_blank" href="http://tools.ietf.org/html/rfc4346">
	    Transport Layer Security Protocol (TLS).
	  </a>.
	  The default TCP port for this mapping is 832.
	</p>
	<p>
	  The NETCONF protocol can also be run directly over BEEP.
	  This mapping is defined in
	  <a target="_blank" href="http://tools.ietf.org/html/rfc4744">RFC 4744</a>.
	  The default TCP port for this mapping is 831.
	</p>
	<p>
	  The 
	  <a target="_blank" href="http://www.iana.org/assignments/port-numbers">IANA port
	    assignments</a> for NETCONF can be summarized in 
	  <b>/etc/services</b> format as follows:
	</p>
	<pre>

        netconf-ssh     830/tcp    NETCONF-over-SSH     # [RFC4742]
        netconf-ssh     830/udp    NETCONF-over-SSH   
        netconf-beep    831/tcp    NETCONF-over-BEEP 
        netconf-beep    831/udp    NETCONF-over-BEEP    # [RFC4744]
        netconfsoaphttp 832/tcp    NETCONF-for-SOAP-over-HTTPS
        netconfsoaphttp 832/udp    NETCONF-for-SOAP-over-HTTPS # [RFC4743]
        netconfsoapbeep 833/tcp    NETCONF-for-SOAP-over-BEEP
        netconfsoapbeep 833/udp    NETCONF-for-SOAP-over-BEEP

	</pre>
	<p>
	  The XSD in RFC 4741 defining the NETCONF protocol
	  has been converted to a YANG module, called
	  <a target="_blank" href="${tg.url('/modulereport/netconf')}">
	    netconf.yang.
	  </a>
	</p>
      </div>

      <a name="sessions"/>
      <div>
	<br/>
	<h2>NETCONF Sessions</h2>
	<p>
	  All NETCONF operations are carried out within a session,
	  which is tied to the transport layer connection.  There is
	  no standard security model for NETCONF yet, but it is assumed that
	  a session represents a particular user with some set
	  of access rights (assigned by an administrator).
	  The NETCONF server is required to authenticate the entity
	  requesting a session before processing any requests
	  from the client.
	</p>
	<p>
	  NETCONF messages are encoded in XML, using the UTF-8 
	  character set.  For SSH, a special message termination
	  sequence of 6 characters is used to provide message framing:
	</p>
	<pre>

          ]]&gt;]]&gt;

	</pre>
	<h3>Session Initiation For Clients</h3>
	<p>
	  The client must initiate the connection
	  and session establishment in NETCONF.  The
	  mandatory (and most implemented) transport is SSH,
	  so a client must open an SSH2 connection to the
	  <b>netconf sub-system</b> to reach the NETCONF server,
	  as shown in the following command line example:
	</p>
	<pre>

	  nms1&gt; ssh -s -p830 server.example.com netconf

	</pre>
	<p>
	  The server should sent its hello message right away,
	  and the client should do the same.  The following
	  example shows the entire &lt;hello&gt; message that
	  a client is required to send:
	</p>
	<pre>

	  &lt;?xml version="1.0" encoding="UTF-8"?&gt;
          &lt;hello xmlns="urn:ietf:params:xml:ns:netconf:base:1.0"&gt;
            &lt;capabilities&gt;
	      &lt;capability&gt;urn:ietf:params:netconf:base:1.0&lt;/capability&gt;
            &lt;/capabilities&gt;
          &lt;/hello&gt;]]&gt;]]&gt;

	</pre>
	<p>
	  At this point the server should be waiting for &lt;rpc&gt;
	  requests to process.  The server should sent an &lt;rpc-reply&gt;
	  for each &lt;rpc&gt; request.  The client can add as many
	  XML attributes to the &lt;rpc&gt; element as desired, and the
	  server will return all those attributes in the &lt;rpc-reply&gt;
	  element.  The <b>message-id</b> attribute is required by the
	  protocol, although this is not really needed.
	</p>
      </div>

      <a name="databases"/>
      <div>
	<br/>
	<h2>Configuration Databases</h2>
        <p>
          <img src="${tg.url('/static/images/database_options_web.png')}" 
	       border="0" alt="netconf layers"/>
        </p>
        <p>&nbsp;</p>
	<p>
	  The NETCONF protocol contains several standard operations
	  which operate on one or more conceptual configuration
	  databases.  For example, the &lt;target&gt; parameter
	  for the 
	  <a target="_blank" 
             href="${tg.url('/modules/netconf/2009-04-10#edit-config.579')}">
	    &lt;edit-config&gt;
	  </a> operation specifies which database to edit.
	</p>
	<p>
	  Configuration databases are represented (in XML)
	  within NETCONF operations as either an empty element
	  identifying a standard database, or a &lt;url&gt;
	  element identifying a non-standard (possibly offline)
	  database.
	</p>
	<p>
	  There are 3 standard conceptual configuration databases
	  <ul>
	    <li><b>&lt;running/&gt;</b>
	      <p>
		This database represents the entire active
		configuration currently within the device.
		It is the only mandatory standard database.
	      </p>
	      <p>
		Unless the server supports the <b>:candidate</b>
		capability, the server must allow this database
		to be edited directly.  Otherwise, the server
		is not required to support changing this database
		directly.  If it does support it, then the
		<b>:writable-running</b> capability will
		be advertised by the server to indicate this support.
	      </p>
	      <p>
		The &lt;running/&gt; database is also used to
		contain all the conceptual state information
		currently available on the device.  This can be
		confusing, but operations such as &lt;get&gt;,
		which operate on the &lt;running/&gt; database
		can retrieve status information and statistics,
		in addition to configuration parameters.
		The &lt;get-config&gt; operation can be used
		instead of &lt;get&gt; to retrieve only the
		configuration data.
	      </p>
	    </li>
	    <li><b>&lt;candidate/&gt;</b>
	      <p>
		This database is available if the <b>:candidate</b>
		capability is supported by the server.  It is a
		global scratchpad database that is used to collect
		edits via 1 or more &lt;edit-config&gt; operations.
		A client can build up a set of changes which
		may or may not be validated by the server,
		until explicitly committed to the running configuration,
		all at once.
	      </p>
	      <p>
		<b>Unlike the &lt;running/&gt; database, any changes
		made to the &lt;candidate/&gt; database do not
		take effect right away within the network device.</b>
	      </p>
	      <p>
		When ready, the client can use the &lt;commit&gt;
		operation to activate the changes embodied
		in the &lt;candidate/&gt; database, and make
		them part of the &lt;running/&gt; configuration.
	      </p>
	      <p>
		After a successful  &lt;commit&gt; operation, 
		the &lt;candidate/&gt; database and the 
		&lt;running/&gt; database have the same 
		configuration contents.  This special condition
		is important because a &lt;lock&gt; operation
		on the &lt;candidate/&gt; database cannot
		be granted otherwise.  It does not matter
		which session makes any changes to the 
		&lt;candidate/&gt; database.  If it is different
		than the &lt;running/&gt; database,
		then it cannot be locked.
	      </p>
	      <p>
		Since this is a global database,
		the client should use the &lt;discard-changes&gt;
		operation to remove any unwanted changes, if
		the &lt;commit&gt; operation is not used.
		This will clean out the &lt;candidate/&gt; database without
		activating any changes that it may contain, and
		prevent the next client using this global database
		from making unintended changes.
		Care must be taken (e.g., use locks) to make sure
		multiple sessions do not make any database edits 
		at the same time.
	      </p>
              <p>
		Server platforms which support 
		the <b>:candidate</b> capability
		usually do not also support the <b>:writable-running</b>
		capability, since mixing direct edits to &lt;running/&gt;
		would defeat the purpose of using this scratchpad
		database to collect and validate changes before
		applying them. 
	      </p>
	    </li>
	    <li><b>&lt;startup/&gt;</b>
	      <p>
		This database is available if the <b>:startup</b>
		capability is supported by the server.  It represents
		the configuration to use upon the next reboot of the
		device.
	      </p>
	      <p>
		If present,
		then the server will not automatically save changes
		to the &lt;running/&gt; database in non-volatile
		storage.  Instead, a &lt;copy-config&gt; operation
		is needed to overwrite the contents of the &lt;startup/&gt;
		database with the current configuration.
	      </p>
	      <p>
		If not present, then the server will automatically
		update its non-volatile storage any time the
		running configuration is modified.  In either case,
		the server is required to maintain non-volatile
		storage of the running configuration, and be able
		to restore a running configuration after a reboot.
	      </p>
	    </li>
	  </ul>
	</p>
      </div>

      <a name="operations"/>
      <div>
	<br/>
	<h2>Protocol Operations</h2>
	<p>
	  Once a NETCONF session is established, the client knows
	  which capabilities the server supports.  The client then
	  can send RPC method requests and receive RPC replies 
	  from the server.  The server's request queue is serialized,
	  so requests will be processed in the order received.
	</p>
	<p>
	  Most operations are designed to select one or two
	  specific configuration databases, but 
	  there are also two general operations for ending
	  NETCONF sessions.
	</p>
	<p>
	  The following table summarizes the set of protocol
	  operations, and shows which capabilities must be
	  supported by the server (see next section) in order
	  for a client to use the operation.
	</p>
	<table align="center" border="1" width="90%">
	  <tr>
	    <td><b>Operation</b></td>
	    <td><b>Usage</b></td>
	    <td><b>Description</b></td>
	  </tr>
	  <tr>
	    <td>close-session</td>
	    <td>:base</td>
	    <td>Terminate this session</td>
	  </tr>
	  <tr>
	    <td>commit</td>
	    <td>:base AND :candidate</td>
	    <td>
	      Commit the contents of the &lt;candidate/&gt; configuration
	      database to the &lt;running/&gt; configuration database
	    </td>
	  </tr>
	  <tr>
	    <td>copy-config</td>
	    <td>:base</td>
	    <td>Copy a configuration database</td>
	  </tr>
	  <tr>
	    <td>create-subscription</td>
	    <td>:notification</td>
	    <td>Create a NETCONF notification subscription</td>
	  </tr>
	  <tr>
	    <td>delete-config</td>
	    <td>:base</td>
	    <td>Delete a configuration database</td>
	  </tr>
	  <tr>
	    <td>discard-changes</td>
	    <td>:base AND :candidate</td>
	    <td>
	      Clear all changes from the &lt;candidate/&gt; configuration
	      database and make it match the &lt;running/&gt; configuration
	      database
	    </td>
	  </tr>
	  <tr>
	    <td>edit-config</td>
	    <td>:base</td>
	    <td>Modify a configuration database</td>
	  </tr>
	  <tr>
	    <td>get</td>
	    <td>:base</td>
	    <td>
	      Retrieve data from the running configuration database
	      and/or device statistics
	    </td>
	  </tr>
	  <tr>
	    <td>get-config</td>
	    <td>:base</td>
	    <td>
	      Retrieve data from the running configuration database
	    </td>
	  </tr>
	  <tr>
	    <td>kill-session</td>
	    <td>:base</td>
	    <td>Terminate another session</td>
	  </tr>
	  <tr>
	    <td>lock</td>
	    <td>:base</td>
	    <td>Lock a configuration database so only my session can write</td>
	  </tr>
	  <tr>
	    <td>unlock</td>
	    <td>:base</td>
	    <td>Unlock a configuration database so any session can write</td>
	  </tr>
	  <tr>
	    <td>validate</td>
	    <td>:base AND :validate</td>
	    <td>
	      Validate the entire contents of a configuration database
	    </td>
	  </tr>
	</table>
	<h3>Editing the Configuration</h3>
	<p>
	  Before using the NETCONF edit operations, the client
	  must determine which database to use as the target
	  by examining the capabilities sent by the server
	  during session establishment.
	</p>
	<pre>

	  if ':candidate' capability supported:
	     target = &lt;candidate/&gt;
	  else if ':writable-running' capability supported:
	     target = &lt;running/&gt;
	  else if ':url' capability supported:
	     target = &lt;url&gt;file://path/to/file&lt;/url&gt;
	  else:
	     target = None     #  Server is non-complaint

	</pre>
	<p>
	  Once the target of the edit operation is determined,
	  the client needs to determine the 'activate' operation
	  that will be needed for the configuration changes to 
	  take effect.
	</p>
	<pre>

	  if ':candidate' capability supported:
             if ':confirmed-commit' capability supported and desired:
                if default timeout of 600 seconds desired:
                   activate_fn =  &lt;commit&gt;
                                   &lt;confirmed/&gt;
                                  &lt;/commit&gt;
                else:
                   activate_fn =  &lt;commit&gt;
                                   &lt;confirmed/&gt;
                                   &lt;confirm-timeout&gt;300&lt;/confirm-timeout&gt;
                                  &lt;/commit&gt;
             else:
                activate_fn =  &lt;commit/&gt;
	  else 
	     activate_fn = None     #  &lt;running/&gt; or &lt;url&gt; target

	</pre>
	<p>
	  After the 'target' and 'activate function' are determined,
	  the client needs to determine how the activated
	  configuration changes are saved in non-volatile storage.
	</p>
	<pre>

	  if ':startup' capability supported:
             save_fn =  &lt;copy-config&gt;
                           &lt;target&gt;&lt;startup/&gt;&lt;/target&gt;
                           &lt;source&gt;&lt;running/&gt;&lt;/source&gt;
                        &lt;/copy-config&gt;
	  else 
	     save_fn = None     #  automatic NV-update

	</pre>
	<h4>Candidate Configuration Example</h4>
	<p>
	  A basic NETCONF edit transaction if the <b>candidate</b> database
	  is the target can be described with
	  the following set of RPC transactions:
	</p>
	<ol>
	  <li>lock &lt;running/&gt; database</li>
	  <li>lock &lt;candidate/&gt; database</li>
	  <li>edit &lt;candidate/&gt; database</li>
	  <li>commit &lt;candidate/&gt; database</li>
	  <li>unlock &lt;candidate/&gt; database</li>
	  <li>unlock &lt;running/&gt; database</li>
	</ol>
	<p>
	  Here is an XML example of this PDU sequence.
	  Note that step 3 can be done multiple times, in arbitrary fashion,
          since none of the changes take effect until step 4.
	</p>

	<pre>

          &lt;rpc message-id="101" 
              xmlns="urn:ietf:params:xml:ns:netconf:base:1.0"&gt;
            &lt;lock&gt;
              &lt;target&gt;&lt;running/&gt;&lt;/target&gt;
            &lt;/lock&gt;
          &lt;/rpc&gt;

          # server returns &lt;ok/&gt; status

          &lt;rpc message-id="102" 
              xmlns="urn:ietf:params:xml:ns:netconf:base:1.0"&gt;
            &lt;lock&gt;
              &lt;target&gt;&lt;candidate/&gt;&lt;/target&gt;
            &lt;/lock&gt;
          &lt;/rpc&gt;

          # server returns &lt;ok/&gt; status

          &lt;rpc message-id="103" 
              xmlns="urn:ietf:params:xml:ns:netconf:base:1.0"&gt;
            &lt;edit-config&gt;
              &lt;target&gt;&lt;candidate/&gt;&lt;/target&gt;
              &lt;default-operation&gt;none&lt;/default-operation&gt;
              &lt;test-option&gt;test-then-set&lt;/test-option&gt;
              &lt;error-option&gt;stop-on-error&lt;/error-option&gt;
              &lt;nc:config 
                  xmlns:nc="urn:ietf:params:xml:ns:netconf:base:1.0"
                  xmlns="uri-for-my-data-model-namespace"&gt;
	        &lt;some-existing-node&gt;
	           &lt;my-new-node nc:operation="create"&gt;
  	             &lt;my-new-leaf&gt;7&lt;/my-new-leaf&gt;
	           &lt;/my-new-node&gt;
	        &lt;/some-existing-node&gt;
              &lt;/nc:config&gt;
            &lt;/edit-config&gt;
          &lt;/rpc&gt;

          # server returns &lt;ok/&gt; status

          &lt;rpc message-id="104" 
              xmlns="urn:ietf:params:xml:ns:netconf:base:1.0"&gt;
            &lt;commit/&gt;
          &lt;/rpc&gt;

          # server returns &lt;ok/&gt; status

          &lt;rpc message-id="105" 
              xmlns="urn:ietf:params:xml:ns:netconf:base:1.0"&gt;
            &lt;unlock&gt;
              &lt;target&gt;&lt;candidate/&gt;&lt;/target&gt;
            &lt;/unlock&gt;
          &lt;/rpc&gt;

          # server returns &lt;ok/&gt; status

          &lt;rpc message-id="106" 
              xmlns="urn:ietf:params:xml:ns:netconf:base:1.0"&gt;
            &lt;unlock&gt;
              &lt;target&gt;&lt;running/&gt;&lt;/target&gt;
            &lt;/unlock&gt;
          &lt;/rpc&gt;

          # server returns &lt;ok/&gt; status

	</pre>

	<h4>Running + Startup Configuration Example</h4>
	<p>
	  A basic NETCONF edit transaction where the <b>&lt;running/&gt;</b>
	  database is the target, and the changes need to be explicitly
	  saved to the <b>&lt;startup/&gt;</b> database, can be described with
	  the following set of RPC transactions:
	</p>
	<ol>
	  <li>lock &lt;running/&gt; database</li>
	  <li>lock &lt;startup/&gt; database</li>
	  <li>edit &lt;running/&gt; database</li>
	  <li>copy &lt;running/&gt; database to &lt;startup/&gt; database</li>
	  <li>unlock &lt;startup/&gt; database</li>
	  <li>unlock &lt;running/&gt; database</li>
	</ol>
	<p>
	  Here is an XML example of this PDU sequence.
	  Note that step 3 must be done carefully, since
          the changes will take effect right away.
	</p>

	<pre>

          &lt;rpc message-id="107" 
              xmlns="urn:ietf:params:xml:ns:netconf:base:1.0"&gt;
            &lt;lock&gt;
              &lt;target&gt;&lt;running/&gt;&lt;/target&gt;
            &lt;/lock&gt;
          &lt;/rpc&gt;

          # server returns &lt;ok/&gt; status

          &lt;rpc message-id="108" 
              xmlns="urn:ietf:params:xml:ns:netconf:base:1.0"&gt;
            &lt;lock&gt;
              &lt;target&gt;&lt;startup/&gt;&lt;/target&gt;
            &lt;/lock&gt;
          &lt;/rpc&gt;

          # server returns &lt;ok/&gt; status

          &lt;rpc message-id="109" 
              xmlns="urn:ietf:params:xml:ns:netconf:base:1.0"&gt;
            &lt;edit-config&gt;
              &lt;target&gt;&lt;running/&gt;&lt;/target&gt;
              &lt;default-operation&gt;none&lt;/default-operation&gt;
              &lt;test-option&gt;test-then-set&lt;/test-option&gt;
              &lt;error-option&gt;rollback-on-error&lt;/error-option&gt;
              &lt;nc:config 
                  xmlns:nc="urn:ietf:params:xml:ns:netconf:base:1.0"
                  xmlns="uri-for-my-data-model-namespace"&gt;
	        &lt;some-existing-node&gt;
	           &lt;my-new-node nc:operation="create"&gt;
  	             &lt;my-new-leaf&gt;7&lt;/my-new-leaf&gt;
	           &lt;/my-new-node&gt;
	        &lt;/some-existing-node&gt;
              &lt;/nc:config&gt;
            &lt;/edit-config&gt;
          &lt;/rpc&gt;

          # server returns &lt;ok/&gt; status

          &lt;rpc message-id="110" 
              xmlns="urn:ietf:params:xml:ns:netconf:base:1.0"&gt;
            &lt;copy-config&gt;
              &lt;target&gt;&lt;startup/&gt;&lt;/target&gt;
              &lt;source&gt;&lt;running/&gt;&lt;/source&gt;
            &lt;/copy-config&gt;
          &lt;/rpc&gt;

          # server returns &lt;ok/&gt; status

          &lt;rpc message-id="111" 
              xmlns="urn:ietf:params:xml:ns:netconf:base:1.0"&gt;
            &lt;unlock&gt;
              &lt;target&gt;&lt;startup/&gt;&lt;/target&gt;
            &lt;/unlock&gt;
          &lt;/rpc&gt;

          # server returns &lt;ok/&gt; status

          &lt;rpc message-id="112" 
              xmlns="urn:ietf:params:xml:ns:netconf:base:1.0"&gt;
            &lt;unlock&gt;
              &lt;target&gt;&lt;running/&gt;&lt;/target&gt;
            &lt;/unlock&gt;
          &lt;/rpc&gt;

          # server returns &lt;ok/&gt; status

	</pre>
      </div>

      <a name="capabilities"/>
      <div>
	<br/>
	<h2>Protocol Capabilities</h2>
	<p>
	  Many features and mechanisms within the NETCONF protocol
	  do not apply to every use case or every device.  Optional
	  mechanisms are given URI handles, which are sent by
	  the server during the NETCONF &lt;hello&gt; message exchange,
	  during session initialization.
	</p>
	<p>
	  For example, if a server allows the running configuration
	  to be edited directly, then it will include the following
	  &lt;capability&gt; element in its &lt;hello&gt; message:
	</p>
	<pre>

          &lt;capability&gt;
            urn:ietf:params:netconf:capability:writable-running:1.0
	  &lt;/capability&gt;
  
	</pre>
	<p>
	  Each capability is also given a human-readable name,
	  which is used throughout the documentation, instead
	  of the URI representation.  By convention, a colon character
	  is pre-pended to the name to indicate it is a capability
	  identifier.
	</p>
	<pre>

          :writable-running
	  
	</pre>
	<p>
	  The NETCONF protocol itself has a capability URI assignment,
	  which is used in the &lt;hello&gt; message exchange to
	  ensure both peers are using the same version of the protocol.
	  The client and server each send the following capability
	  URI during this exchange.  For the client, this is the
	  only capability sent.  The server will also include
	  &lt;capability&gt; elements for each optional capability
	  supported, if any.
	</p>
	<pre>

        &lt;capability&gt;urn:ietf:params:netconf:base:1.0&lt;/capability&gt;
  
	</pre>
	<p>
	  Although the base protocol is not optional, and not really
	  a capability, it is given the following name in
	  this document, for consistency and future-proofing.
	</p>
	<pre>

          :base
	  
	</pre>

	<h3>Standard Capabilities</h3>
	<p>
	  The following table summarizes the standard capabilities
	  which a server may choose to support.
	</p>
	<ul>
	  <li><b>:candidate</b>
	    <p>
	      The server supports the &lt;candidate/&gt; database.
	      It will allow this special database to be locked, edited,
	      saved, and unlocked.  The server will also support the
	      &lt;discard-changes&gt; and basic &lt;commit&gt;
	      operations.
	    </p>
	  </li>
	  <li><b>:confirmed-commit</b>
	    <p>
	      For servers that support the <b>:candidate</b> capability,
	      this additional capability will also be advertised
	      if the server supports the 'confirmed commit' feature.
	      <b>This special mode requires a server to send two
	      &lt;commit&gt; RPC method requests instead of one,
	      to save any changes to the &lt;running/&gt; database.</b>
	      If the second request does not arrive within a specified 
	      time interval, the server will automatically revert
	      the running configuration to the previous version.
	    </p>
	  </li>
	  <li><b>:interleave</b>
	    <p>
	      The server will accept &lt;rpc&gt; requests
	      (besides &lt;close-session&gt; while
	      notification delivery is active.  The
	      :notification capability must also be present
	      if this capability is advertised.
	    </p>
	  </li>
	  <li><b>:notification</b>
	    <p>
	      The server supports the basic notification delivery
	      mechanisms defined in RFC 5277, e.g., the
	      &lt;create-subscription&gt; operation will
	      be accepted by the server.  Unless the
	      :interleave capability is also supported, only
	      the &lt;close-session&gt; operation must be
	      supported by the server while notification
	      delivery is active.
	    </p>
	  </li>
	  <li><b>:rollback-on-error</b>
	    <p>
	      The server supports the 'rollback-on-error'
	      value for the &lt;error-option&gt;
	      parameter to the &lt;edit-config&gt; operation.
	      If any error occurs during the requested edit
	      operation, then the target database (usually the
	      running configuration) will be left affected.
	      <b>This provides an 'all-or-nothing' edit mode
	      for a single &lt;edit-config&gt; request.</b>
	    </p>
	  </li>
	  <li><b>:startup</b>
	    <p>
	      The server supports the &lt;startup/&gt; database.
	      It will allow the running configuration to be
	      copied to this special database.  It can also be locked,
	      and unlocked, but a server is not required to allow
	      it to be edited.
	    </p>
	  </li>
	  <li><b>:url</b>
	    <p>
	      The server supports the &lt;url&gt; parameter value form to
	      specify protocol operation source and target
	      parameters.  The capability URI for this feature
	      will indicate which schemes (e.g., file, https, sftp)
	      that the server supports within a particular URL value.
	      The 'file' scheme allows for editable local 
	      configuration databases.   The other schemes allow
	      for remote storage of configuration databases.
	    </p>
	  </li>
	  <li><b>:validate</b>
	    <p>
	      The server supports the &lt;validate&gt; operation.
	      When this operation is requested on a target database,
	      the server will perform some amount of parameter validation
	      and referential integrity checking.  Since the standard
	      does not define exactly what must be validated by this
	      operation, a client cannot really rely on it for anything
	      useful.
	    </p>
	    <p>
	      This operation is used to validate a complete database.
	      There is no standard way to validate a single edit
	      request against a target database, however a
	      non-standard set-option for the &lt;edit-config&gt;
	      operation called <b>test-only</b> has been defined
	      for this purpose.
	    </p>
	  </li>
	  <li><b>:writable-running</b>
	    <p>
	      The server allows the client to change the
	      running configuration directly.  Either this capability
	      or the <b>:candidate</b> capability will be supported
	      by the server, but usually not both.
	    </p>
	  </li>
	  <li><b>:xpath</b>
	    <p>
	      The server fully supports the XPath 1.0 specification
	      for filtered retrieval
	      of configuration and other database contents.
	      The 'type' attribute within the &lt;filter&gt;
	      parameter for &lt;get&gt; and &lt;get-config&gt;
	      operations may be set to 'xpath'.  The 'select'
	      attribute (which contains the XPath expression)
	      will also be supported by the server.
	    </p>
	    <p>
	      A server may support partial XPath retrieval
	      filtering, but it cannot advertise the <b>:xpath</b>
	      capability unless the entire XPath 1.0 specification
	      is supported.
	    </p>
	  </li>
	</ul>
      </div>

      <a name="notifications"/>
      <div>
	<br/>
	<h2>Notifications</h2>
	<p>
	  NETCONF has a notification delivery mechanism,
	  defined in
	  <a target="_blank" href="http://www.rfc-editor.org/rfc/rfc5277.txt">
	    RFC 5277.
	  </a>
	</p>
	<p>
	  The  &lt;create-subscription&gt; operation is used
	  to setup session-specific filtering and optional
	  buffered notification delivery parameters.
	  These parameters cannot be modified after
	  notification delivery has started on a particular session.
	  The only way to stop live notification delivery
	  is to terminate the session, usually with the
	  &lt;close-session&gt; operation.
	</p>
	<p>
	  The NETCONF notification definitions are divided
	  into 2 namespaces, one for the top-level
	  &lt;notification&gt; element, and the other
	  for some initial notification content, 
	  such as the &lt;replayComplete&gt; event.
	</p>
	<p>
	  These definitions have been converted to YANG, named
	  <a target="_blank" href="${tg.url('/modulereport/nc-notifications')}">
	    nc-notifications.yang
	  </a> and
	  <a target="_blank" href="${tg.url('/modulereport/notifications')}">
	    notifications.yang.
	  </a>
	</p>
      </div>

      <a name="standards"/>
      <div class="publication-list">
	<br/>
	<h2>Completed RFC Specifications</h2>

	<h4>RFC 4741</h4>
	<p>
	  Defines the NETCONF protocol RPC layer and operations layer.<br/><br/>
	  <b>Status:</b> Proposed Standard RFC, mandatory-to-implement<br/><br/>
	  <a target="_blank" href="http://www.ietf.org/rfc/rfc4741.txt">
	    NETCONF Configuration Protocol
	  </a>
	</p>

	<h4>RFC 4742</h4>
	<p>
	  Defines the NETCONF-over-SSH transport mapping.<br/><br/>
	  <b>Status:</b> Proposed Standard RFC, mandatory-to-implement<br/><br/>
	  <a target="_blank" href="http://www.ietf.org/rfc/rfc4742.txt">
	    Using the NETCONF Configuration Protocol over Secure Shell (SSH)
	  </a>
	</p>

	<h4>RFC 4743</h4>
	<p>
	  Defines the NETCONF-over-SOAP transport mapping.<br/><br/>
	  <b>Status:</b> Proposed Standard RFC, optional-to-implement<br/><br/>
	  <a target="_blank" href="http://www.ietf.org/rfc/rfc4743.txt">
	    Using NETCONF over the Simple Object Access Protocol (SOAP)
	  </a>
	</p>

	<h4>RFC 4744</h4>
	<p>
	  Defines the NETCONF-over-BEEP transport mapping.<br/><br/>
	  <b>Status:</b> Proposed Standard RFC, optional-to-implement<br/><br/>
	  <a target="_blank" href="http://www.ietf.org/rfc/rfc4744.txt">
	    Using the NETCONF Protocol over
            the Blocks Extensible Exchange Protocol (BEEP)
	  </a>
	</p>

	<h4>RFC 5277</h4>
	<p>
	  Defines the NETCONF notification delivery mechanisms.<br/><br/>
	  <b>Status:</b> Proposed Standard RFC, optional-to-implement<br/><br/>
	    <a target="_blank" href="http://www.ietf.org/rfc/rfc5277.txt">
	      NETCONF Event Notifications
	    </a>
	</p>

	<h4>RFC 5381</h4>
	<p>
	  Documents some NETCONF-over-SOAP implementation experience.<br/><br/>
	  <b>Status:</b> Informational RFC, nothing-to-implement<br/><br/>
	    <a target="_blank" href="http://www.ietf.org/rfc/rfc5381.txt">
	      Experience of Implementing NETCONF over SOAP
	    </a>
	</p>

	<h4>RFC 5539</h4>
	<p>
	  Defines the NETCONF-over-TLS transport mapping.<br/><br/>
	  <b>Status:</b> Proposed Standard RFC, optional-to-implement<br/><br/>
	  <a target="_blank" href="http://www.ietf.org/rfc/rfc5539.txt">
	    NETCONF Over Transport Layer Security (TLS)
	  </a>
	</p>

	<h4>RFC 5717</h4>
	<p>
	  Defines a partial database locking mechanism (based on
	  instance-identifier or XPath expressions) for the 
	  NETCONF protocol.<br/><br/>
	  <b>Status:</b> Proposed Standard RFC, optional-to-implement<br/><br/>
	  <a target="_blank" 
             href="http://www.ietf.org/rfc/rfc5717.txt">
	    Partial Lock RPC for NETCONF
	  </a>
	</p>

      </div>

      <a name="work"/>
      <div class="publication-list">
	<br/>
	<h2>Standards Work in Progress</h2>

	<h4>draft-ietf-netconf-monitoring</h4>
	<p>
	  Defines a server monitoring data model and schema retrieval
	  mechanism for the NETCONF protocol.<br/><br/>
	  <b>Intended Status:</b> Proposed Standard RFC, optional-to-implement<br/><br/>
	  <a target="_blank" 
             href="http://www.ietf.org/internet-drafts/draft-ietf-netconf-monitoring-12.txt">
	    NETCONF Monitoring Schema
	  </a>
	</p>

	<h4>draft-ietf-netconf-with-defaults</h4>
	<p>
	  Defines a mechanism to control the filtering of leaf objects
          containing the 'default' value, during NETCONF retrieval operations.<br/><br/>
	  <b>Intended Status:</b> Proposed Standard RFC, optional-to-implement<br/><br/>
	  <a target="_blank" 
             href="http://www.ietf.org/internet-drafts/draft-ietf-netconf-with-defaults-05.txt">
	    With-defaults capability for NETCONF
	  </a>
	</p>

	<h4>draft-ietf-netconf-4741bis</h4>
	<p>
	  Defines a minor update to the NETCONF protocol.<br/><br/>
	  <b>Intended Status:</b> Proposed Standard RFC, mandatory-to-implement<br/><br/>
	  <a target="_blank" 
             href="http://www.ietf.org/internet-drafts/draft-ietf-netconf-4741bis-02.txt">
	    Network Configuration Protocol
	  </a>
	</p>

	<h4>draft-ietf-netconf-4742bis</h4>
	<p>
	  Defines a minor update to the NETCONF protocol over Secure Shell document.<br/><br/>
	  <b>Intended Status:</b> Proposed Standard RFC, mandatory-to-implement<br/><br/>
	  <a target="_blank" 
             href="http://www.ietf.org/internet-drafts/draft-ietf-netconf-rfc4742bis-00.txt">
	    Using the Network Configuration Protocol over Secure Shell
	  </a>
	</p>

      </div>

      <a name="resources"/>
      <div>
	<br/>
	<h2>Additional NETCONF Resources</h2>
	<ul>
	  <li>
	    <a target="_blank" 
               href="http://www.ietf.org/dyn/wg/charter/netconf-charter.html">
	      NETCONF WG Charter
	    </a>
	  </li>
	  <li>
	    <a target="_blank" 
               href="http://trac.tools.ietf.org/wg/netconf/trac/wiki">
	      NETCONF WG Wiki Page
	    </a>
	  </li>
	  <li>
	    <a target="_blank" 
               href="http://www.rfc-editor.org/errata_search.php?rfc=4742">
	      Errata for RFC 4742 (NETCONF Over SSH)
	    </a>
	  </li>
	  <li>
	    <a target="_blank" 
               href="http://www.iana.org/assignments/xml-registry/ns/netconf.txt">
	      IANA assignment for NETCONF namespace
	    </a>
	  </li>
	  <li>
	    <a target="_blank" 
               href="http://www.iana.org/assignments/xml-registry/ns/netconf/base/1.0.txt">
	      IANA assignment for NETCONF base namespace
	    </a>
	  </li>
	  <li>
	    <a target="_blank" 
               href="http://www.iana.org/assignments/xml-registry/ns/netconf/soap.txt">
	      IANA assignment for NETCONF over SOAP URI
	    </a>
	  </li>
	  <li>
	    <a target="_blank" 
               href="http://www.iana.org/assignments/xml-registry/schema/netconf.xsd">
	      IANA version of the NETCONF protocol XSD
	    </a>
	  </li>
	  <li>
	    <a target="_blank" 
               href="${tg.url('/slides/netconf/netconf_tutorial.html')}">Tutorial Slides (circa 2004)</a>
	  </li>
	  <li>
	    <a target="_blank" 
               href="${tg.url('/static/papers/df_netconf_white_paper.pdf')}">
	      NETCONF White Paper by David French (embeddedmind.com)</a>
	  </li>
	</ul>
      </div>

    </div>
  </body>
</html>
