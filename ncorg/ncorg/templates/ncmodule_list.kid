<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN"
          "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml"
      xmlns:py="http://purl.org/kid/ns#"
      py:extends="'master.kid'">
  <head>
    <meta content="text/html; charset=utf-8"
	  http-equiv="Content-Type" py:replace="''"/>
    <title>List Netconf Central Modules</title>
  </head>
  <body>
    <div class="ncmain">
      <h1>Modules</h1>

      <?python row = "0" ?>
      <table id="tablist" border="1">
	<tr id="tabheader">
	  <td>Module</td>
	  <td>Abstract</td>
	</tr>
	<tr py:for="ncmodule in ncmodules" id="tablist">
	  <?python
	     if row =="0":
                 row = "1"
                 idfill = "tablist_odd"
	     else:
                 row = "0"
                 idfill = "tablist_even"
	     ?>
	  <td id="${idfill}">
	    <a href="${tg.url('/modulereport/' + ncmodule.modname)}"
	       py:content="ncmodule.modname"/>
	  </td>
          <td  id="${idfill}"
	       py:content="ncmodule.abstract"/>
        </tr>
      </table>
    </div>
  </body>
</html>
