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
    <title>Browse Netconf Central Modules</title>
  </head>
  <body>
  <h1>Modules</h1>
  <div class="ncmain">
    <div py:for="ncmodule in ncmodules" id="nclist">
      <a name="${ncmodule.modname}"/>
      <h2 py:if="ncmodule.docurl" id="nclist_banner">
	<a href="${tg.url(ncmodule.docurl)}" py:content="ncmodule.modname"/>
      </h2>
      <h2 py:if="not ncmodule.docurl" id="nclist_banner" py:content="ncmodule.modname"/>
      <div class="tabber">
        <div class="tabbertab">
          <h2>Summary</h2>
          <div>
	    <table>
              <tr>
		<td>Module</td>
		<td>
		  <a href="${tg.url('/modulereport/' + ncmodule.modname)}"
		     py:content="ncmodule.modname"/>
		</td>
	      </tr>
              <tr>
		<td>Version</td>
		<td py:content="ncmodule.version"/>
	      </tr>
              <tr py:if="ncmodule.abstract">
		<td>Abstract</td>
		<td py:content="ncmodule.abstract"/>
	      </tr>
	    </table>
          </div>
        </div>
        <div class="tabbertab"
	     py:if="ncmodule.description and ncmodule.abstract and
		    not (ncmodule.description == ncmodule.abstract)">
          <h2>Description</h2>
          <pre py:content="ncmodule.description"/>
        </div>
        <div class="tabbertab">
          <h2>Details</h2>
          <div>
	    <table>
              <tr><td>&nbsp;</td><td>&nbsp;</td></tr>
              <tr py:if="ncmodule.organization">
		<td>Organization</td>
		<td py:content="ncmodule.organization"/>
	      </tr>
              <tr py:if="ncmodule.organization"><td>&nbsp;</td><td>&nbsp;</td></tr>

              <tr>
		<td>Module</td>
		<td py:content="ncmodule.modname"/>
	      </tr>
              <tr py:if="ncmodule.ismod==0">
		<td>Submodule</td>
		<td py:content="ncmodule.submodname"/>
	      </tr>
              <tr>
		<td>Version</td>
		<td py:content="ncmodule.version"/>
	      </tr>
              <tr py:if="ncmodule.sourcename">
                <td>File</td>
		<td py:content="ncmodule.sourcename"/>
              </tr>

              <tr><td>&nbsp;</td><td>&nbsp;</td></tr>

              <tr py:if="ncmodule.ismod==1">
		<td>Prefix</td>
		<td py:content="ncmodule.modprefix"/>
	      </tr>
              <tr py:if="ncmodule.ismod==1">
		<td>Namespace</td>
		<td py:content="ncmodule.namespace"/>
	      </tr>
              <tr py:if="ncmodule.reference">
		<td>Reference</td>
		<td py:content="ncmodule.reference"/>
	      </tr>

              <tr><td>&nbsp;</td><td>&nbsp;</td></tr>

              <tr py:if="ncmodule.docurl">
                <td>HTML</td>
		<td><a href="${tg.url(ncmodule.docurl)}" 
		       py:content="ncmodule.docurl"/>
		</td>
              </tr>
              <tr py:if="ncmodule.srcurl">
                <td py:if="ncmodule.isyang==1">YANG</td>
                <td py:if="not ncmodule.isyang==1">NCX</td>
                <td>
		  <a href="${tg.url(ncmodule.srcurl)}"
		     py:content="ncmodule.srcurl"/>
		</td>
              </tr>
              <tr py:if="ncmodule.xsdurl">
                <td>XSD</td>
		<td>
		  <a href="${tg.url(ncmodule.xsdurl)}"  
		     py:content="ncmodule.xsdurl"/>
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
