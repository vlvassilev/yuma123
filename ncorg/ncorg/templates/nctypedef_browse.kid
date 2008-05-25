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
    <title>Browse Netconf Central Typedefs</title>
  </head>
  <body>
    <h1>Typedefs</h1>
    <div class="ncmain">
      <div py:for="nctypedef in nctypedefs" id="nclist">
	<a name="${nctypedef.name}"/>
	<h2 py:if="nctypedef.docurl" id="nclist_banner">
	  <a href="${tg.url(nctypedef.docurl)}" py:content="nctypedef.name"/>
	</h2>
	<h2 py:if="not nctypedef.docurl" id="nclist_banner" py:content="nctypedef.name"/>
	<div class="tabber">
          <div class="tabbertab">
            <h2>Summary</h2>
            <div>
	      <table>
		<tr>
		  <td>Name</td>
		  <td py:content="nctypedef.name"/>
		</tr>
		<tr>
		  <td>Type</td>
		  <td py:content="nctypedef.basetypename"/>
		</tr>
		<tr py:if="nctypedef.description">
		  <td>&nbsp;</td><td>&nbsp;</td>
		</tr>
		<tr py:if="nctypedef.description">
		  <td>&nbsp;</td>
		  <td>
		    <pre py:content="nctypedef.description"/>
		  </td>
		</tr>
	      </table>
            </div>
          </div>
          <div class="tabbertab">
            <h2>Details</h2>
            <div>
	      <table>
		<tr>
		  <td>Module</td>
		  <td py:content="nctypedef.modname"/>
		</tr>
		<tr py:if="nctypedef.modname != nctypedef.submodname">
		  <td>Submodule</td>
		  <td py:content="nctypedef.submodname"/>
		</tr>
		<tr>
		  <td>Version</td>
		  <td py:content="nctypedef.version"/>
		</tr>
		<tr py:if="nctypedef.reference">
		  <td>Reference</td>
		  <td py:content="nctypedef.reference"/>
		</tr>
		<tr py:if="nctypedef.docurl">
                  <td>Source</td>
		  <td>
		    <a href="${tg.url(nctypedef.docurl)}" 
		       py:content="nctypedef.submodname + ' line ' +
				   str(nctypedef.linenum)"/>
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
