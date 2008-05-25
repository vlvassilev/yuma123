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
    <title>List Netconf Central Extensions</title>
  </head>
  <body>
    <div class="ncmain">
      <h1>Extensions</h1>
      <h2 py:if="mod" py:content="'module: ' + mod"/>
      <?python row = "0" ?>
      <table id="tablist" border="1">
	<tr id="tabheader">
	  <td>Extension</td>
	  <td>Argument</td>
	  <td>Abstract</td>
	</tr>
	<tr py:for="ncextension in ncextensions" id="tablist">
	  <?python
	     if row =="0":
                 row = "1"
                 idfill = "tablist_odd"
	     else:
                 row = "0"
                 idfill = "tablist_even"
	     ?>
	  <td id="${idfill}" py:if="ncextension.docurl">
	    <a href="${tg.url(ncextension.docurl)}"
	       py:content="ncextension.name"/>
	  </td>
	  <td id="${idfill}" py:if="not ncextension.docurl"
	      py:content="ncextension.name"/>
          <td  id="${idfill}" py:if="ncextension.argument"
	       py:content="ncextension.argument[0:69]"/>
          <td  id="${idfill}"
	       py:content="ncextension.description[0:69]"/>
        </tr>
      </table>
    </div>
  </body>
</html>
