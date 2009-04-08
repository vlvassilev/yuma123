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
    <title>Netconf Central</title>
  </head>
  <body>
    <div class="nchello_home">
      <h1>Welcome to Netconf Central</h1>
      <h3>XML-based Network Configuration Tools<br/>
	Online YANG Module Database</h3>
      <br/>
      <img border="0" 
	   src="${tg.url('/static/images/nails.png')}" 
	   alt="Nails"/>
      <br/><br/>
      <h3><a href="/netconf_docs">Learn About NETCONF</a></h3>
      <h3><a href="/yang_docs">Learn About YANG</a></h3>
    </div>
  </body>
</html>
