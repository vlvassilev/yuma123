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
    <title>Show Netconf Central Module</title>
  </head>
  <body>
    <div id="main_content">
      <h1 py:content="mod"/>
      <div class="tabber" id="nclist">
        <div class="tabbertab">
          <h2>Summary</h2>
          <div py:for="ncmodule in ncmodules">
	    <table>

	      <tr><td>&nbsp;</td><td>&nbsp;</td></tr>

	      <tr id="nclist_hdr" py:if="ncmodules.count() != 1">
		<td py:content="ncmodule.submodname"/>
		<td>&nbsp;</td>
	      </tr>

              <tr py:if="ncmodule.organization"><td>&nbsp;</td><td>&nbsp;</td></tr>

              <tr py:if="ncmodule.organization">
		<td>Organization</td>
		<td py:content="ncmodule.organization"/>
	      </tr>

              <tr py:if="ncmodule.organization"><td>&nbsp;</td><td>&nbsp;</td></tr>

              <tr py:if="ncmodule.ismod==1">
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
		<td><pre py:content="ncmodule.reference"/></td>
	      </tr>

              <tr><td>&nbsp;</td><td>&nbsp;</td></tr>

              <tr py:if="ncmodule.docurl">
                <td>HTML</td>
		<td><a href="${tg.url(ncmodule.docurl)}" 
		       py:content="ncmodule.docurl"/></td>
              </tr>
              <tr py:if="ncmodule.srcurl">
                <td py:if="ncmodule.isyang == 1">YANG</td>
                <td py:if="ncmodule.isyang != 1">NCX</td>
                <td><a href="${tg.url(ncmodule.srcurl)}" 
		       py:content="ncmodule.srcurl"/></td>
              </tr>
              <tr py:if="ncmodule.xsdurl">
                <td>XSD</td>
		<td><a href="${tg.url(ncmodule.xsdurl)}"  
		       py:content="ncmodule.xsdurl"/></td>
              </tr>

              <tr><td>&nbsp;</td><td>&nbsp;</td></tr>

              <tr py:if="ncmodule.abstract">
		<td>Abstract</td>
		<td py:content="ncmodule.abstract"/>
	      </tr>

              <tr><td>&nbsp;</td><td>&nbsp;</td></tr>

              <tr py:if="ncmodule.contact">
		<td>Contact</td>
		<td><pre py:content="ncmodule.contact"/></td>
	      </tr>
	    </table>
          </div>
        </div>
        <div class="tabbertab">
          <h2>Description</h2>
          <div py:for="ncmodule in ncmodules">
	    <table>
	      <tr>
		<td>&nbsp;</td>
	      </tr>
	      <tr id="nclist_hdr" py:if="ncmodules.count() != 1">
		<td py:content="ncmodule.submodname"/>
	      </tr>
	      <tr>
		<td py:if="ncmodule.description">
		  <pre py:content="ncmodule.description" /></td>
		<td py:if="not ncmodule.description">
		  <pre py:content="ncmodule.abstract" /></td>
	      </tr>
	    </table>
	  </div>
        </div>
        <div class="tabbertab" py:if="nctypedefs.count() != 0">
          <h2>Typedefs</h2>
          <div>
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
		<td  id="${idfill}"
		     py:content="nctypedef.description[0:69]"/>
              </tr>
	    </table>
          </div>
        </div>
        <div class="tabbertab" py:if="ncgroupings.count() != 0">
          <h2>Groupings</h2>
          <div>
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
        </div>
        <div class="tabbertab" py:if="ncobjects.count() != 0">
          <h2>Objects</h2>
	  <div>
	    <table id="tablist_m">
	      <tr><td>Type Key</td></tr>
	      <tr><td id="tablist_m1">Mandatory config</td></tr>
	      <tr><td id="tablist_m2">Optional config</td></tr>
	      <tr><td id="tablist_m3">Not config</td></tr>
	    </table>
	  </div>
          <div>
	    <?python row = "0" ?>
	    <table id="tablist" border="1">
	      <tr id="tabheader">
		<td>Object</td>
		<td>Type</td>
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

		   if ncobject.config and ncobject.mandatory:
		       l2fill = "tablist_m1"

		   if ncobject.config and not ncobject.mandatory:
		       l2fill = "tablist_m2"

		   if not ncobject.config:
		       l2fill = "tablist_m3"
		?>
		<td id="${idfill}">
		  <span py:for="idx in range(1, ncobject.level)">
		    &nbsp;&nbsp;
		  </span>
		  <a py:if="ncobject.docurl"
		     href="${tg.url(ncobject.docurl)}" 
		     py:content="ncobject.name"/>
		  <span py:if="not ncobject.docurl"
			py:content="ncobject.name"/>			
		</td>
		<td id="${l2fill}"
		    py:content="ncobject.objtyp"/>
		<td id="${idfill}" py:if="ncobject.description"
		    py:content="ncobject.description[0:69]"/>
		<td id="${idfill}"
		    py:if="(not ncobject.description) and ncobject.childlist"
		    py:content="ncobject.childlist[0:69]"/>
		<td id="${idfill}"
		    py:if="(not ncobject.description) and not ncobject.childlist"
		    py:content="'OID: ' + ncobject.objectid[0:69]"/>
              </tr>
	    </table>
          </div>
        </div>

        <div class="tabbertab" py:if="ncrpcs.count() != 0">
          <h2>RPC Methods</h2>
          <div>
	    <?python row = "0" ?>
	    <table id="tablist" border="1">
	      <tr id="tabheader">
		<td>RPC</td>
		<td>Abstract</td>
	      </tr>
	      <tr py:for="ncrpc in ncrpcs" id="tablist">
		<?python
		   if row =="0":
                       row = "1"
                       idfill = "tablist_odd"
		   else:
                       row = "0"
                       idfill = "tablist_even"
		?>
		<td id="${idfill}" py:if="ncrpc.docurl">
		  <a href="${tg.url(ncrpc.docurl)}" 
		     py:content="ncrpc.name"/>
		</td>
		<td id="${idfill}" py:if="not ncrpc.docurl"
		    py:content="ncrpc.name"/>
		<td id="${idfill}" py:if="ncrpc.description"
		    py:content="ncrpc.description[0:79]"/>
		<td id="${idfill}"
		    py:if="(not ncrpc.description) and ncrpc.childlist"
		    py:content="ncrpc.childlist[0:79]"/>
		<td id="${idfill}"
		    py:if="(not ncrpc.description) and not ncrpc.childlist"
		    py:content="'OID: ' + ncrpc.objectid[0:79]"/>
              </tr>
	    </table>
          </div>
        </div>

        <div class="tabbertab" py:if="ncnotifs.count() != 0">
          <h2>Notifications</h2>
          <div>
	    <?python row = "0" ?>
	    <table id="tablist" border="1">
	      <tr id="tabheader">
		<td>Notification</td>
		<td>Abstract</td>
	      </tr>
	      <tr py:for="ncnotif in ncnotifs" id="tablist">
		<?python
		   if row =="0":
                       row = "1"
                       idfill = "tablist_odd"
		   else:
                       row = "0"
                       idfill = "tablist_even"
		?>
		<td id="${idfill}" py:if="ncnotif.docurl">
		  <a href="${tg.url(ncnotif.docurl)}"
		     py:content="ncnotif.name"/>
		</td>
		<td id="${idfill}" py:if="not ncnotif.docurl"
		    py:content="ncnotif.name"/>
		<td id="${idfill}" py:if="ncnotif.description"
		    py:content="ncnotif.description[0:79]"/>
		<td id="${idfill}"
		    py:if="(not ncnotif.description) and ncnotif.childlist"
		    py:content="ncnotif.childlist[0:79]"/>
		<td id="${idfill}"
		    py:if="(not ncnotif.description) and not ncnotif.childlist"
		    py:content="'OID: ' + ncnotif.objectid[0:79]"/>
              </tr>
	    </table>
          </div>
        </div>
        <div class="tabbertab" py:if="ncextensions.count() != 0">
          <h2>Extensions</h2>
          <div>
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
		<td  id="${idfill}" py:if="not ncextension.argument">&nbsp;</td>
		<td  id="${idfill}"
		     py:content="ncextension.description[0:69]"/>
              </tr>
	    </table>
	  </div>
	</div>

<!--
        <?python
           showraw = 0
	   if ncmodules.count()==1:
	      for ncmodule in ncmodules:
	         if ncmodule.isyang:
	            showraw = 1
        ?>

        <div class="tabbertab" py:if="showraw == 1">
          <h2>Raw HTML</h2>
          <div>
	    <?python
       import os,sys

       err = ""
       opened = 0
       if version=='latest':
          version = ncmodule.version

       fname = os.getcwd() + "/ncorg/static/modules/" + \
          mod + "." + version + ".div"
       try:
           f = open(fname, 'r')
       except IOError:
           err = "Could not open file %s" % (fname)
       except:
           err = "Unexpected error: %s" % (sys.exc_info()[0])
           raise
       else:
          opened = 1
	       ?>
	     <div py:if="opened==1">${XML(f.read())}</div>
	     <div py:if="opened==0" py:content="err"/>
	  </div>
	</div>
-->
        <?python
           showcooked = 0
	   if ncmodules.count()==1:
	      for ncmodule in ncmodules:
	         if ncmodule.isyang:
	            showcooked = 1
        ?>
        <div class="tabbertab" py:if="showcooked == 1">
          <h2>Cooked HTML</h2>
          <div>
	    <?python
       import os,sys

       err = ""
       opened = 0
       if version=='latest':
          version = ncmodule.version

       fname = os.getcwd() + "/ncorg/static/cookedmodules/" + \
          mod + "." + version + ".div"
       try:
           f = open(fname, 'r')
       except IOError:
           err = "Could not open file %s" % (fname)
       except:
           err = "Unexpected error: %s" % (sys.exc_info()[0])
           raise
       else:
          opened = 1
	       ?>
	     <div py:if="opened==1">${XML(f.read())}</div>
	     <div py:if="opened==0" py:content="err"/>
	  </div>
	</div>
      </div>
    </div>
  </body>
</html>
