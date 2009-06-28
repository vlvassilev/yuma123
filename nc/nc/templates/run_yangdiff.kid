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
    <title>Run Yangdiff at Netconf Central</title>
  </head>
  <body>
    <div id="tableform">
      <h2>Compare YANG module revisions</h2>
      <br />
      <p>
	This version of yangdiff supports YANG as defined
	in 'draft-ietf-netmod-yang-04.txt'.<br/>
	File names must be in one of these forms:
	<ul>
	  <li>modulename.yang  (foo.yang)</li>
	  <li>modulename.revision-date.yang  (foo.2009-03-17.yang)</li>
	</ul>
	Modules that are listed on Netconf Central do not need
	to be uploaded for module validatation, but if a different version
	is uploaded, it will be used instead.
      </p>
      <br/>
      <div>${form()}</div>
    </div>
  </body>
</html>
