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
    <title>Yangdump Validation Results</title>
  </head>
  <body>
    <div class="ncresults">
      <?python

	 import os,sys

	 banner = "Validation results for " + srcfile
	 err = ""
	 err2 = None
	 results = ""
	 opened = 0
	 opened2 = 0

	 fname = os.getcwd() + "/ncorg/workdir/" + resfile

	 try:
	      f = open(fname, 'r')
	 except IOError:
	     err = "Could not open file %s" % (fname)
	 except:
	     err = "Unexpected error: %s" % (sys.exc_info()[0])
	     raise
	 else:
	     opened = 1
	     results = f.read()
	     f.close()

	 if report:
	     opened2 = 0
	     fname2 = os.getcwd() + "/ncorg/workdir/reports"
	     try:
	         f2 = open(fname2, 'r')
	     except IOError:
	         err2 = "Could not open file %s" % (fname2)
	     except:
	         err2 = "Unexpected error: %s" % (sys.exc_info()[0])
	         raise
	     else:
	         opened2 = 1
	         results2 = f2.read()
	         f2.close()
      ?>
      <h2 py:content="banner"/>
      <pre py:if="opened==1" py:content="results"/>
      <div py:if="opened==0" py:content="err"/>
      <div py:if="report=='True'">
        <h2 py:content="'Reports for ' + srcfile" />
	<div py:if="err2" py:content="err2"/>
	<pre py:if="opened2==1" py:content="results2"/>	
      </div>
    </div>
  </body>
</html>
