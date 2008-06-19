<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" 
	  "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<?python import sitetemplate ?>

<html xmlns="http://www.w3.org/1999/xhtml" xmlns:py="http://purl.org/kid/ns#"
      py:extends="sitetemplate">

  <head py:match="item.tag=='{http://www.w3.org/1999/xhtml}head'" 
	py:attrs="item.items()">
    <meta content="text/html; charset=UTF-8" http-equiv="content-type" 
	  py:replace="''"/>
    <title py:replace="''">Your title goes here</title>
    <meta py:replace="item[:]" name="description" content="master template"/>
    <link rel="stylesheet" type="text/css" media="screen" 
	  href="../static/css/style.css"
          py:attrs="href=tg.url('/static/css/style.css')"/>

<script type="text/javascript"><!--//--><![CDATA[//><!--

sfHover = function() {
	var sfEls = document.getElementById("nav").getElementsByTagName("LI");
	for (var i=0; i<sfEls.length; i++) {
		sfEls[i].onmouseover=function() {
			this.className+=" sfhover";
		}
		sfEls[i].onmouseout=function() {
			this.className=this.className.replace(new RegExp(" sfhover\\b"), "");
		}
	}
}
if (window.attachEvent) window.attachEvent("onload", sfHover);

//--><!]]></script>


  <meta name="keywords"
	content="netconf, xml, yang, yangtools, configuration, 
         router configuration, configuration tools, yangdump, SMIv2"/>
  </head>
  <body py:match="item.tag=='{http://www.w3.org/1999/xhtml}body'" 
	py:attrs="item.items()">
    <table class="ncheader">
      <tr>
	<td>
	  <a href="${tg.url('/')}">
            <img src="${tg.url('/static/images/ncorg-logo.png')}" 
		 align="center" border="0" alt="netconfcentral logo" 
		 height="60" width="300"/>
	  </a>
	</td>
	<td class="ncheader_right">
	  ${modmenu.display()}
	</td>
      </tr>
    </table>

    <div class="ncbanner">
      <ul id="nav">
	<li><a href="/">Home</a></li>
	<li><a href="">YANG&nbsp;Database</a>
	  <ul>
            <li class="daddy"><a href="">List&nbsp;All</a>
	      <ul>
		<li><a href="/modulelist">Modules</a></li>
		<li><a href="/typedeflist">Typedefs</a></li>
		<li><a href="/groupinglist">Groupings</a></li>
		<li><a href="/objectlist">Objects</a></li>
		<li><a href="/rpclist">RPC&nbsp;Methods</a></li>
		<li><a href="/notificationlist">Notifications</a></li>
		<li><a href="/extensionlist">Extensions</a></li>
	      </ul>
	    </li>
	    <li class="daddy"><a href="">Browse&nbsp;All</a>
	      <ul>
		<li><a href="/modulebrowse">Modules</a></li>
		<li><a href="/typedefbrowse">Typedefs</a></li>
		<li><a href="/groupingbrowse">Groupings</a></li>
		<li><a href="/objectbrowse">Objects</a></li>
		<li><a href="/rpcbrowse">RPC&nbsp;Methods</a></li>
		<li><a href="/notificationbrowse">Notifications</a></li>
		<li><a href="/extensionbrowse">Extensions</a></li>
	      </ul>
	    </li>
	    <li><a href="/search">Search</a></li>
	  </ul>
	</li>
	<li><a href="">Online&nbsp;Tools</a>
	  <ul>
	    <li><a href="/run_yangdump">Run&nbsp;yangdump</a></li>
          </ul>
	</li>
	<li><a href="">Documentation</a>
	  <ul>
	    <li><a href="/database_docs">YANG&nbsp;database</a></li>
	    <li><a href="/yangdump_manual">yangdump&nbsp;program</a></li>
	    <li><a href="/netconf_docs">NETCONF&nbsp;protocol</a></li>
	    <li><a href="/yang_docs">YANG&nbsp;language</a></li>
	  </ul>
	</li>
	<li><a href="">Free&nbsp;Downloads</a>
	  <ul>
	    <li><a href="/download">yangdump</a></li>
          </ul>
	</li>

      </ul>
    </div>

    <div id="main_content">
      <div id="status_block" class="flash"
           py:if="value_of('tg_flash', None)" py:content="tg_flash"></div>
      <div py:replace="[item.text]+item[:]">page content</div>
    </div>

    <div id="footer">
      <hr />
      <p>YangTools 0.9.&beta;3, Copyright &#169; 2008, Andy Bierman</p>
      <p>TurboGears is Copyright &#169; 2007, Kevin Dangoor</p>
      <p>Send comments and bug reports to &lt;support@netconfcentral.com&gt;</p>
    </div>
  </body>

</html>
