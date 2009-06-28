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
    <title>Netconf Central Extension List</title>
  </head>
  <body>
    <h1>Extensions</h1>
    <div class="ncmain">
      <div py:for="ncextension in ncextensions" id="nclist">
	<a name="${ncextension.name}"/>
	<h2 py:if="ncextension.docurl" id="nclist_banner">
          <a href="${tg.url(ncextension.docurl)}" py:content="ncextension.name"/>
	</h2>
	<h2 py:if="not ncextension.docurl" id="nclist_banner" py:content="ncextension.name"/>
	<div class="tabber">
          <div class="tabbertab">
            <h2>Summary</h2>
            <div>
	      <table>
		<tr>
		  <td>Name</td>
		  <td py:content="ncextension.name"/>
		</tr>
		<tr>
		  <td>Module</td>
		  <td py:content="ncextension.modname"/>
		</tr>
		<tr py:if="ncextension.submodname != ncextension.modname">
		  <td>Submodule</td>
		  <td py:content="ncextension.submodname"/>
		</tr>
		<tr>
		  <td>Version</td>
		  <td py:content="ncextension.version"/>
		</tr>
		<tr py:if="ncextension.description">
		  <td>&nbsp;</td><td>&nbsp;</td>
		</tr>
		<tr py:if="ncextension.description">
		  <td>&nbsp;</td>
		  <td>
		    <pre py:content="ncextension.description"/>
		  </td>
		</tr>
	      </table>
            </div>
          </div>
          <div class="tabbertab">
            <h2>Details</h2>
            <div>
	      <table>
		<tr py:if="ncextension.argument">
		  <td>Argument</td>
		  <td py:content="ncextension.argument"/>
		</tr>
		<tr py:if="ncextension.argument">
		  <td>YIN Element</td>
		  <td py:content="ncextension.yinelement"/>
		</tr>
		<tr>
		  <td>Version</td>
		  <td py:content="ncextension.version"/>
		</tr>
		<tr>
		  <td>File</td>
		  <td py:content="ncextension.submodname"/>
		</tr>
		<tr>
		  <td>Line</td>
		  <td py:content="ncextension.linenum"/>
		</tr>
		<tr py:if="ncextension.reference">
		  <td>Reference</td>
		  <td py:content="ncextension.reference"/>
		</tr>
		<tr py:if="ncextension.docurl">
                  <td>Source</td>
		  <td>
		    <a href="${tg.url(ncextension.docurl)}" 
		       py:content="ncextension.submodname + '.yang line ' +
				   str(ncextension.linenum)"/>
		  </td>
		</tr>
	      </table>
            </div>
          </div>
	</div>
      </div>
    </div>
  </body>
</html>
