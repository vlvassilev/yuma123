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
    <title>Netconf Central YANG Documentation</title>
  </head>
  <body>
    <div class="ncdocs">
      <h1>YANG Data Modeling Language</h1>

      <img src="${tg.url('/static/images/yang_as_source_code2_web.png')}" 
	   border="0" alt="yang as source code"/>
      <p>&nbsp;</p>

      <div>
	<h2>NETMOD WG</h2>
	<p>
          The <b>NETCONF Data Modeling Language Working Group</b> 
	  <a target="_blank" 
             href="http://www.ietf.org/html.charters/netmod-charter.html">
	    (NETMOD)
	  </a>
	  is developing a high-level data modeling language for the 
	  NETCONF protocol, called <b>YANG</b>. 
	</p>

        <p>
          The NETMOD 
          <a target="_blank" 
             href="https://trac.tools.ietf.org/wg/netmod/">Wiki
          </a> 
          contains up-to-date information
          about the Internet Drafts under development by this WG.
        </p>
        <div class="publication-list">
	  <br/>
	  <h2>Standards Work in Progress</h2>

	  <h4>draft-ietf-netmod-yang</h4>
	  <p>
            YANG is a data modeling language used to model configuration and
            state data manipulated by the Network Configuration Protocol
            (NETCONF) protocol, NETCONF remote procedure calls, and NETCONF
            notifications.
	    <br/><br/>
	    <b>Intended Status:</b> Proposed Standard RFC, mandatory-to-implement<br/><br/>
	    <a target="_blank" 
               href="http://www.ietf.org/internet-drafts/draft-ietf-netmod-yang-11.txt">
              YANG - A data modeling language for NETCONF
	    </a>
	  </p>

	  <h4>draft-ietf-netmod-yang-types</h4>
	  <p>
            This document introduces a collection of common data types to be used
            with the YANG data modeling language.
	    <br/><br/>
	    <b>Intended Status:</b> Proposed Standard RFC, mandatory-to-implement<br/><br/>
	    <a target="_blank" 
               href="http://www.ietf.org/internet-drafts/draft-ietf-netmod-yang-types-07.txt">
              Common YANG Data Types
	    </a>
	  </p>

	  <h4>draft-ietf-netmod-dsdl-map</h4>
	  <p>
            This draft specifies the mapping rules for translating YANG data
            models into Document Schema Definition Languages (DSDL), a
            coordinated set of XML schema languages standardized as ISO 19757.
            The following DSDL schema languages are used by the mapping: RELAX
            NG, Schematron and DSRL.  The mapping takes one or more YANG modules
            and produces a set of DSDL schemas for a selected target document
            type - datastore content, NETCONF PDU etc.  Procedures for schema-
            based validation of such documents are also discussed.
	    <br/><br/>
	    <b>Intended Status:</b> Proposed Standard RFC, optional-to-implement<br/><br/>
	    <a target="_blank" 
               href="http://www.ietf.org/internet-drafts/draft-ietf-netmod-dsdl-map-04.txt">
              Mapping YANG to Document Schema Definition Languages and Validating
              NETCONF Content
	    </a>
	  </p>

	  <h4>draft-ietf-netmod-arch</h4>
	  <p>
            NETCONF gives access to native capabilities of the devices within a
            network, defining methods for manipulating configuration databases,
            retrieving operational data, and invoking specific operations.  YANG
            provides the means to define the content carried via NETCONF, both
            data and operations.  Using both technologies, standard modules can
            be defined to give interoperability and commonality to devices, while
            still allowing devices to express their unique capabilities.
            <br/><br/>
            This document describes how NETCONF and YANG help build network
            management applications that meet the needs of network operators.
	    <br/><br/>
	    <b>Intended Status:</b> Informational RFC, nothing-to-implement
            <br/><br/>
	    <a target="_blank" 
               href="http://www.ietf.org/internet-drafts/draft-ietf-netmod-arch-03.txt">
              An NETCONF- and NETMOD-based Architecture for Network Management
	    </a>
	  </p>

	  <h4>draft-ietf-netmod-yang-usage</h4>
	  <p>
            This memo provides guidelines for authors and reviewers of standards
            track specifications containing YANG data model modules.  Applicable
            portions may be used as a basis for reviews of other YANG data model
            documents.  Recommendations and procedures are defined, which are
            intended to increase interoperability and usability of NETCONF
            implementations which utilize YANG data model modules.
	    <br/><br/>
	    <b>Intended Status:</b> Informational RFC, nothing-to-implement
            <br/><br/>
	    <a target="_blank" 
               href="http://www.ietf.org/internet-drafts/draft-ietf-netmod-yang-usage-03.txt">
              Guidelines for Authors and Reviewers of YANG Data Model Documents
	    </a>
	  </p>

        </div>

        <p>&nbsp;</p>

	<h2>YANG Central</h2>
	<p>
          The WEB home for YANG information is
	  <a target="_blank" href="http://www.yang-central.org/">
	    YANG Central</a>, which has news and information about
	  the YANG language.
	</p>
	<p>
	  There are some 
	  <a target="_blank" 
             href="http://www.yang-central.org/twiki/bin/view/Main/YangTutorials">
	    tutorials</a> and 
	  <a target="_blank" 
             href="http://www.yang-central.org/twiki/bin/view/Main/YangExamples">
	    examples</a>
	  that may be helpful as well.
	</p>
	<h2>Additional Resources</h2>
	<p>
          <ul>
	    <li>
	      <a target="_blank"
		 href="http://www.ibr.cs.tu-bs.de/projects/libsmi/">
		libsmi</a> contains <b>smidump</b>, which can be used to
	      automatically convert SMIv2 modules to YANG modules.
	    </li>
	    <li>Another YANG
	      <a target="_blank"
		 href="${tg.url('/static/slides/yangtutorial/yang_getting_started2.html')}">
		tutorial
	      </a>, still a work-in-progress
	    </li>
	  </ul>
	</p>
      </div>
    </div>
  </body>
</html>
