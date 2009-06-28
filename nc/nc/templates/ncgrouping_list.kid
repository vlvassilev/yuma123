<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN"
          "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml"
      xmlns:py="http://purl.org/kid/ns#"
      py:extends="'master.kid'">
  <head>
    <meta content="text/html; charset=utf-8"
	  http-equiv="Content-Type" py:replace="''"/>
    <title>List Netconf Central Groupings</title>
  </head>
  <body>
    <div class="ncmain">
      <h1>Groupings</h1>
      <h2 py:if="mod" py:content="'module: ' + mod"/>
      <?python row = "0" ?>
      <table id="tablist" border="1">
	<tr id="tabheader">
	  <td>Grouping</td>
	  <td>Objects</td>
	  <td>Abstract</td>
	</tr>
	<tr py:for="ncgrouping in ncgroupings" id="tablist">
	  <?python
	     if row =="0":
                 row = "1"
                 idfill = "tablist_odd"
	     else:
                 row = "0"
                 idfill = "tablist_even"
	  ?>
	  <td id="${idfill}" py:if="ncgrouping.docurl">
	    <a href="${tg.url(ncgrouping.docurl)}"
	       py:content="ncgrouping.name"/>
	  </td>
	  <td id="${idfill}" py:if="not ncgrouping.docurl"
	      py:content="ncgrouping.name"/>
          <td id="${idfill}"
	       py:content="ncgrouping.objectlist[0:79]"/>
          <td id="${idfill}"
	       py:content="ncgrouping.description[0:79]"/>
        </tr>
      </table>
    </div>
  </body>
</html>
