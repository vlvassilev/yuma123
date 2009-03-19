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

      <img src="${tg.url('/static/images/yang_layers.png')}" 
	   border="2" alt="yang layers"/>
      <p>&nbsp;</p>

      <div>
	<h2>NETMOD WG</h2>
	<p>
          The <b>NETCONF Data Modeling Language Working Group</b> 
	  <a href="http://www.ietf.org/html.charters/netmod-charter.html">
	    (NETMOD)
	  </a>
	  is developing a high-level data modeling language for the 
	  NETCONF protocol, called <b>YANG</b>.  The current version
	  of the language specification is dated
	  <a href="http://www.ietf.org/internet-drafts/draft-ietf-netmod-yang-04.txt">
	    March 2009.
	  </a>
	</p>
	<h2>YANG Central</h2>
	<p>
          The WEB home for YANG information is
	  <a href="http://www.yang-central.org/">
	    YANG Central</a>, which has news and information about
	  the YANG language.
	</p>
	<p>
	  There are some 
	  <a href="http://www.yang-central.org/twiki/bin/view/Main/YangTutorials">
	    tutorials</a> and 
	  <a href="http://www.yang-central.org/twiki/bin/view/Main/YangExamples">
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
