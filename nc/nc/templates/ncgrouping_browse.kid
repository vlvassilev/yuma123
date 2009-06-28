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
    <title>Browse Netconf Central Groupings</title>
  </head>
  <body>
    <h1>Groupings</h1>
    <div class="ncmain">
      <div py:for="ncgrouping in ncgroupings" id="nclist">
	<a name="${ncgrouping.name}"/>
	<h2 py:if="ncgrouping.docurl" id="nclist_banner">
	  <a href="${tg.url(ncgrouping.docurl)}" py:content="ncgrouping.name"/>
	</h2>
	<h2 py:if="not ncgrouping.docurl" id="nclist_banner" py:content="ncgrouping.name"/>
	<div class="tabber">
          <div class="tabbertab">
            <h2>Summary</h2>
            <div>
	      <table>
		<tr>
		  <td>Name</td>
		  <td py:content="ncgrouping.name"/>
		</tr>
		<tr py:if="ncgrouping.description">
		  <td>&nbsp;</td><td>&nbsp;</td>
		</tr>
		<tr py:if="ncgrouping.description">
		  <td>&nbsp;</td>
		  <td>
		    <pre py:content="ncgrouping.description"/>
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
		  <td py:content="ncgrouping.modname"/>
		</tr>
		<tr py:if="ncgrouping.submodname != ncgrouping.modname">
		  <td>Submodule</td>
		  <td py:content="ncgrouping.submodname"/>
		</tr>
		<tr>
		  <td>Version</td>
		  <td py:content="ncgrouping.version"/>
		</tr>
		<tr py:if="ncgrouping.reference">
		  <td>Reference</td>
		  <td py:content="ncgrouping.reference"/>
		</tr>
		<tr py:if="ncgrouping.docurl">
                  <td>Source</td>
		  <td>
		    <a href="${tg.url(ncgrouping.docurl)}" 
		       py:content="ncgrouping.submodname + ' line ' + 
				   str(ncgrouping.linenum)"/>
		  </td>
		</tr>
		<tr py:if="ncgrouping.objectlist">
                  <td>Objects</td>
		  <td py:content="ncgrouping.objectlist"/>
		</tr>
	      </table>
            </div>
          </div>
	</div>
      </div>
    </div>
  </body>
</html>
