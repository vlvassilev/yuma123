<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN"
          "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml"
      xmlns:py="http://purl.org/kid/ns#"
      py:extends="'master.kid'">
  <head>
    <meta content="text/html; charset=utf-8"
	  http-equiv="Content-Type" py:replace="''"/>
    <title>List Netconf Central Notifications</title>
  </head>
  <body>
    <div class="ncmain">
      <h1>Notifications</h1>
      <h2 py:if="mod" py:content="'module: ' + mod"/>
      <?python row = "0" ?>
      <table id="tablist" border="1">
	<tr id="tabheader">
	  <td>Notification</td>
	  <td>Abstract</td>
	</tr>
	<tr py:for="ncobject in ncobjects" id="tablist">
	  <?python
	     if row =="0":
                 row = "1"
                 idfill = "tablist_odd"
	     else:
                 row = "0"
                 idfill = "tablist_even"
	  ?>
	  <td id="${idfill}" py:if="ncobject.docurl">
	    <a href="${tg.url(ncobject.docurl)}" 
	       py:content="ncobject.name"/>
	  </td>
	  <td id="${idfill}" py:if="not ncobject.docurl"
	      py:content="ncobject.name"/>
          <td id="${idfill}"
              py:if="ncobject.description and len(ncobject.description) &lt; 256"
	       py:content="ncobject.description"/>
          <td id="${idfill}"
              py:if="ncobject.description and len(ncobject.description) &gt; 255"
	       py:content="ncobject.description[0:255] + '...'"/>
          <td id="${idfill}"
	      py:if="(not ncobject.description) and ncobject.childlist"
	       py:content="ncobject.childlist"/>
          <td id="${idfill}"
	      py:if="(not ncobject.description) and not ncobject.childlist"
	       py:content="'OID: ' + ncobject.objectid"/>
        </tr>
      </table>
    </div>
  </body>
</html>
