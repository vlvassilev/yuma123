<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN"
          "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml"
      xmlns:py="http://purl.org/kid/ns#"
      py:extends="'master.kid'">
  <head>
    <meta content="text/html; charset=utf-8"
	  http-equiv="Content-Type" py:replace="''"/>
    <title>List Netconf Central Typedefs</title>
  </head>
  <body>
    <div class="ncmain">
      <h1>Typedefs</h1>
      <h2 py:if="mod" py:content="'module: ' + mod"/>
      <?python row = "0" ?>
      <table id="tablist" border="1">
	<tr id="tabheader">
	  <td>Typedef</td>
	  <td>Base type</td>
	  <td>Abstract</td>
	</tr>
	<tr py:for="nctypedef in nctypedefs" id="tablist">
	  <?python
	     if row =="0":
                 row = "1"
                 idfill = "tablist_odd"
	     else:
                 row = "0"
                 idfill = "tablist_even"
	     ?>
	  <td id="${idfill}" py:if="nctypedef.docurl">
	    <a href="${tg.url(nctypedef.docurl)}"
	       py:content="nctypedef.name"/>
	  </td>
	  <td id="${idfill}" py:if="not nctypedef.docurl"
	      py:content="nctypedef.name"/>
	  <td id="${idfill}"
	       py:content="nctypedef.basetypename"/>
          <td  id="${idfill}" py:if="len(nctypedef.description) &lt; 256"
	       py:content="nctypedef.description"/>
          <td  id="${idfill}" py:if="len(nctypedef.description) &gt; 255"
	       py:content="nctypedef.description[0:255] + '...'"/>
        </tr>
      </table>
    </div>
  </body>
</html>
