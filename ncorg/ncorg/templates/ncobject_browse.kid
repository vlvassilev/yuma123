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
    <title>Browse Netconf Central Objects</title>
  </head>
  <body>
    <h1>Objects</h1>
    <div class="ncmain">
      <div py:for="ncobject in ncobjects" id="nclist">
	<a name="${ncobject.name}"/>
	<h2 py:if="ncobject.docurl" id="nclist_banner">
	  <a href="${tg.url(ncobject.docurl)}" py:content="ncobject.name"/>
	</h2>
	<h2 py:if="not ncobject.docurl" id="nclist_banner" py:content="ncobject.name"/>
	<div class="tabber">
          <div class="tabbertab">
            <h2>Summary</h2>
            <div>
	      <table>
		<tr>
		  <td>Name</td>
		  <td py:content="ncobject.name"/>
		</tr>
		<tr>
		  <td>Type</td>
		  <td py:content="ncobject.objtyp"/>
		</tr>
		<tr>
		  <td>Module</td>
		  <td py:content="ncobject.modname"/>
		</tr>
		<tr py:if="ncobject.description">
		  <td>&nbsp;</td><td>&nbsp;</td>
		</tr>
		<tr py:if="ncobject.description">
		  <td>&nbsp;</td>
		  <td>
		    <pre py:content="ncobject.description"/>
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
		  <td>Node ID</td>
		  <td py:content="ncobject.objectid"/>
		</tr>
		<tr>
		  <td>Module</td>
		  <td py:content="ncobject.modname"/>
		</tr>
		<tr py:if="ncobject.modname != ncobject.submodname">
		  <td>Submodule</td>
		  <td py:content="nctypedef.submodname"/>
		</tr>
		<tr>
		  <td>Version</td>
		  <td py:content="ncobject.version"/>
		</tr>
		<tr py:if="ncobject.reference">
		  <td>Reference</td>
		  <td py:content="ncobject.reference"/>
		</tr>
		<tr py:if="ncobject.augwhen">
		  <td>When</td>
		  <td py:content="ncobject.augwhen"/>
		</tr>
		<tr py:if="ncobject.listkey">
		  <td>Key</td>
		  <td py:content="ncobject.listkey"/>
		</tr>
		<tr>
		  <td>Config</td>
		  <td py:content="ncobject.config"/>
		</tr>
		<tr>
		  <td>Mandatory</td>
		  <td py:content="ncobject.mandatory"/>
		</tr>
		<tr py:if="ncobject.defval">
                  <td>Default</td>
		  <td py:content="ncobject.defval"/>
		</tr>
		<tr py:if="ncobject.childlist">
                  <td>Children</td>
		  <td py:content="ncobject.childlist"/>
		</tr>
		<tr py:if="ncobject.docurl">
                  <td>Source</td>
		  <td>
		    <a href="${tg.url(ncobject.docurl)}" 
		       py:content="ncobject.submodname + ' line ' +
				   str(ncobject.linenum)"/>
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
