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
    <title>Search Netconf Central Database</title>
  </head>
  <body>
    <div class="ncsearch">
      <h2>Search the YANG database</h2>
      <br />
      <div class="tabber">
        <div class="tabbertab">
          <h2>Find modules</h2>
	  <div>${modform()}</div>
	</div>
        <div class="tabbertab">
          <h2>Find types</h2>
	  <div>${typform()}</div>
	</div>
        <div class="tabbertab">
          <h2>Find objects</h2>
	  <div>${objform()}</div>
	</div>
        <div class="tabbertab">
          <h2>Find type usage</h2>
	  <div>${tuform()}</div>
	</div>
        <div class="tabbertab">
          <h2>Find extensions</h2>
	  <div>${extform()}</div>
	</div>
      </div>
    </div>
  </body>
</html>
