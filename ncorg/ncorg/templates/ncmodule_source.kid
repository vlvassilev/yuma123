<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN"
          "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml"
      xmlns:py="http://purl.org/kid/ns#"
      py:extends="'master.kid'">
  <head>
    <meta content="text/html; charset=utf-8"
	  http-equiv="Content-Type" py:replace="''"/>
    <title py:content="mod" />
  </head>
  <body>
    <?python
       import os,sys

       err = ""
       opened = 0
       fname = os.getcwd() + "/ncorg/static/modules/" + mod + "@" + version + ".div"
       try:
           f = open(fname, 'r')
       except IOError:
           err = "Error: Could not find module '%s' or version '%s'" % (mod, version)
       except:
           err = "Unexpected error: %s" % (sys.exc_info()[0])
           raise
       else:
          opened = 1
    ?>
    <div py:if="opened==1">${XML(f.read())}</div>
    <div py:if="opened==0" class="nchello_home">
      <h2 py:content="err"/>
    </div>
  </body>
</html>
