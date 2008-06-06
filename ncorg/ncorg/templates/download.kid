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
    <title>Netconf Central Downloads</title>
  </head>
  <body>
    <div class="download">
      <h1>Downloads</h1>
      <h2>
	<a href="${tg.url('/yangdump_manual')}">yangdump</a>
      </h2>
      <div class="tabber">
	<div class="tabbertab">
          <h2>Summary</h2>
          <p>
            The yangdump program is used to convert
            YANG source files
            to XML Schema 1.0 (XSD),
	    Extensible HyperText Markup Language 1.0 (XHTML), 
            and other formats.
	  </p>
	  <p>
            It is available in binary form for Linux and MacOSX platforms.
	  </p>
	  <p><b>
	    Download the archive file for your computer platform.</b>
	    <br/>
	    Refer to the README file in the distribution for more details.
	  </p>
	</div>
	<div class="tabbertab">
          <h2>Linux</h2>
          <p>
	    Contains the yangdump binary release for Linux x86 platforms.
	  </p>
	  <table>
	    <tr>
	      <td><b>Release date</b></td>
	      <td>&nbsp;</td>
	      <td><b>Filename</b></td>
	    </tr>
	    <tr>
	      <td>2008-06-05</td>
	      <td>&nbsp;</td>
	      <td>
		<a href="${tg.url('/downloads/yangdump/yangdump' +
			 '-linux_0.9.2.tar.gz')}">
		  yangdump-linux_0.9.2.tar.gz
		</a>
	      </td>
	    </tr>
	    <tr>
	      <td>2008-05-15</td>
	      <td>&nbsp;</td>
	      <td>
		<a href="${tg.url('/downloads/yangdump/yangdump' +
			 '-linux_0.9.1.tar.gz')}">
		  yangdump-linux_0.9.1.tar.gz
		</a>
	      </td>
	    </tr>
	  </table>
	</div>
	<div class="tabbertab">
          <h2>MacOSX</h2>
          <p>
	    Contains the yangdump binary release for 
	    MacOS X (Intel) platforms.
	  </p>
	  <table>
	    <tr>
	      <td><b>Release date</b></td>
	      <td>&nbsp;</td>
	      <td><b>Filename</b></td>
	    </tr>
	    <tr>
	      <td>2008-06-05</td>
	      <td>&nbsp;</td>
	      <td>
		<a href="${tg.url('/downloads/yangdump/yangdump' +
			 '-macosx_0.9.2.tar.gz')}">
		  yangdump-macosx_0.9.2.tar.gz
		</a>
	      </td>
	    </tr>
	    <tr>
	      <td>2008-05-15</td>
	      <td>&nbsp;</td>
	      <td>
		<a href="${tg.url('/downloads/yangdump/yangdump' +
			 '-macosx_0.9.1.tar.gz')}">
		  yangdump-macosx_0.9.1.tar.gz
		</a>
	      </td>
	    </tr>
	  </table>
	</div>
      </div>
      <div>&nbsp;</div>
      <h2>Modules</h2>
      <div class="tabber">
	<div class="tabbertab">
	  <h2>Summary</h2>
          <p>
	    Archive containing just the YANG modules included in the release.
	    <br/>This file is also included in the binary distributions.
	  </p>
	</div>
	<div class="tabbertab">
	  <h2>All platforms</h2>
	  <table>
	    <tr>
	      <td><b>Release date</b></td>
	      <td>&nbsp;</td>
	      <td><b>Filename</b></td>
	    </tr>
	    <tr>
	      <td>2008-06-05</td>
	      <td>&nbsp;</td>
	      <td>
		<a href="${tg.url('/downloads/modules/modules_0.9.2.tar.gz')}">
		  modules_0.9.2.tar.gz
		</a>
	      </td>
	    </tr>
	    <tr>
	      <td>2008-05-15</td>
	      <td>&nbsp;</td>
	      <td>
		<a href="${tg.url('/downloads/modules/modules_0.9.1.tar.gz')}">
		  modules_0.9.1.tar.gz
		</a>
	      </td>
	    </tr>
	  </table>
	</div>
      </div>
      <br/>
      <div>
	<h2>Subversion Server Access</h2>
	<p>
	  The most current files can be obtained from the
	  <a href="http://subversion.tigris.org/">Subversion</a>
	  directory, with the following command:
	</p>
	<pre class="code">

     svn checkout http://svn.netconfcentral.com/svn/yangtools
	</pre>
	<p>
	  To retrieve only the latest set of modules from the subversion
	  server, use the following command:
	</p>
	<pre class="code">

     svn checkout http://svn.netconfcentral.com/svn/yangtools/modules
	</pre>
      </div>
    </div>
  </body>
</html>
