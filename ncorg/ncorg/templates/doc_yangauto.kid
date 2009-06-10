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
    <title>Netconf Central YANG Based Automation</title>
  </head>
  <body>
    <div class="ncdocs">
      <h1>YANG Based Automation</h1>

      <img src="${tg.url('/static/images/cookedmodules.png')}" 
	   border="0" alt="cooked modules"/>

      <div>
	<h2>Cooked YANG Modules</h2>	
	<p>
	  One of the most significant costs
	  in network configuration management is the ad-hoc,
	  manual, and trial-and-error approach to platform
	  specific customization (and bugs).
	</p>
	<p>
	  The YANG language includes several features to enable tools
	  to automate many of the software development tasks
	  associated with platform or implementation specific
	  details. 
	</p>
	<p>
	  A <b>cooked module</b> is the conceptual
	  composite of a common YANG source file and a set
	  of parameters for a particular agent implementation.
	  A manager application can use a cooked module to
	  perform offline database validation, tailor
	  management operations, control user interface
	  features, etc.
	</p>
	<p>
	  There are six mechanisms available in YANG to
	  support this sort of automation:
	  <ul>
	    <li>YANG language extensions</li>
	    <li>Enabled module features</li>
	    <li>Platform specific object deviations</li>
	    <li>Platform enabled NETCONF capabilities</li>
	    <li>Supported module revision</li>
	    <li>Platform-specific XPath variables for must/when</li>
	  </ul>
	</p>

	<h4>Extensions</h4>
	<p>
	  Language extensions allow developers to annotate the
	  YANG source file with tool directives of arbitrary
	  complexity.  However, conditional directives,
	  such as '#ifdef' is C are not supported.
	  An example of YANG language extensions can be found
	  in the module 
	  <a href="${tg.url('/modulereport/ncx')}">
	    ncx.yang.
	  </a>
	</p>
	    
	<h4>Features</h4>
	<p>
	  The YANG 'feature' mechanism allows a module to
	  be partitioned into mandatory and optional sections.
	  Any number of 'if-feature' statements can be combined
	  (i.e., simple AND expression) to allow complex
	  conditional objects to be specified.
	</p>
	<p>
	  At runtime, the agent specifies which features are supported
	  for each module (in the module &lt;capability&gt; URI).
	  Tools can automatically include/exclude module-related code
	  based on the 'feature list' for the platform, and
	  automatically generate the &lt;hello&gt; message
	  for each session.
	</p>

	<h4>Deviations</h4>
	<p>
	  YANG provides an extensive 'deviations' mechanism to
	  allow an implementation to identify exactly how
	  a particular data model object is implemented,
	  even if it deviates dramatically from the standard definition.
	</p>
	<p>
	  At YANG module set processing time, a tool can automatically
	  patch the deviations into the internal YANG module 
	  representation.  The composite module can be constructed
	  by the manager application, using the deviations information
	  in the module &lt;capabilities&gt; advertised in each 
	  &lt;hello&gt; message.
	</p>

	<h4>Capabilities</h4>
	<p>
	  Vendor-specific NETCONF capabilities can always be
	  used if desired, or if no YANG mechanism is available
	  to provide the correct tool behavior.  Language
	  extensions could be used in conjunction with vendor
	  capabilities to associate portion(s) of the data model
	  with the NETCONF capability.
	</p>
	<p>
	  Similar to features, a tool can utilize vendor capabilities
	  to tailor automatically generated code for supporting
	  the YANG module on different platforms.
	</p>

	<h4>Revision</h4>
	<p>
	  The exact revision date of each module is advertised
	  in the module &lt;capabilities&gt;.  YANG allows
	  multiple versions of the same 'library' modules
	  to be used concurrently within the agent,
	  via the 'import-by-revision' feature.
	</p>
	<p>
	  Tools can maintain a library of all known module
	  versions, and construct the exact module set
	  used by a particular agent, using the module
	  information in the &lt;hello&gt; message.
	</p>

	<h4>XPath Variables</h4>
	<p>
	  YANG provides two XPath-based mechanisms to specify
	  multi-object constraint conditions.  The 'must' statement
	  is used to specify the conditions that must be true
          for an instance of the object containing the must statement(s) to be
	  considered 'valid'.  The 'when' statement is used
	  to specify the conditions that must be true for
	  the object itself to be considered supported by the agent.
	</p>
	<p>
	  XPath supports user variables
	  which can be used in 'must' or 'when' expressions.
	  An agent can utilize a platform-specific set of XPath
	  variables, during normal automatic processing of
	  all 'must' and 'when' statement validation.
	</p>
      </div>

      <img src="${tg.url('/static/images/coststack.png')}" 
	   border="0" alt="cost stack"/>

      <div>
	<h2>NETCONF Development Costs</h2>	
	<p>
	  Another source of significant costs
	  in network configuration management is the labor-intensive
	  nature of data model agent implementation on multiple product
	  platforms.  
	</p>
	<p>
	  There are three basic types of code within the agent implementation
	  for a particular data model module:
	  <ul>
	    <li>
	      <b>Networking feature</b>:<br/>
	      This code performs the actual operations/behavior indicated
	      by the configuration database contents (e.g., turn the
	      fan motor on or off).  This code would be the same
	      cost regardless of the management protocol that is used.
	    </li>
	    <li>
	      <b>NETCONF database access</b>:<br/>
	      This code handles all the protocol operation access to
	      the data model content in all of the NETCONF databases.
	      Most of the development costs are found here, due to
	      complex validation, creation, deletion, editing, filtering,
	      commit and rollback requirements imposed by the protocol.
	      The platform-dependent development costs are the highest,
	      and they are also the source of divergence between
	      implementation behavior for the same data model.
	    </li>
	    <li>
	      <b>NETCONF operations</b>:<br/>
	      This code handles the NETCONF protocol operation processing
	      which is unrelated to data models, such as session support,
	      XML message handling, error reporting, and transport mappings.
	      Due to the significant complexity, but universal applicability,
	      this code is often available in 'toolkits', so the cost
	      is relatively low, compared to the rest of the agent 
	      implementation.
	    </li>
	  </ul>
	</p>
      </div>
      <hr/>
      <img border="0" 
	   src="${tg.url('/static/images/nails.png')}" 
	   alt="Nails"/>
    </div>
  </body>
</html>
