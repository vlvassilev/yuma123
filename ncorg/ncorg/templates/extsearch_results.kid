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
    <title>Extension Search Results</title>
  </head>
  <body>
    <h1>Extension Search Results</h1>
    <div class="ncmain">
      <?python row = "0" ?>
      <table id="tablist" border="1">
	<tr id="tabheader">
	  <td>Extension</td>
	  <td>Module</td>
	  <td>Version</td>
	  <td>Abstract</td>
	</tr>
	<tr py:for="ncext in ncexts" id="tablist">
	  <?python
	     if row =="0":
                 row = "1"
                 idfill = "tablist_odd"
	     else:
                 row = "0"
                 idfill = "tablist_even"
	     ?>
	  <td id="${idfill}">
	    <a href="${tg.url(ncext.docurl)}"
	       py:content="ncext.name"/>
	  </td>
	  <td id="${idfill}" py:content="ncext.modname"/>
	  <td id="${idfill}" py:content="ncext.version"/>
          <td id="${idfill}" 
              py:if="len(ncext.description) &lt; 256"
              py:content="ncext.description"/>
          <td id="${idfill}" 
              py:if="len(ncext.description) &gt; 255"
              py:content="ncext.description[0:255] + '...'"/>
        </tr>
      </table>
    </div>
  </body>
</html>
