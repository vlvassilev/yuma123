<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xmlns:py="http://purl.org/kid/ns#"
    py:extends="'master.kid'">
<head>
<meta content="text/html; charset=utf-8" http-equiv="Content-Type" py:replace="''"/>
<title py:if="page=='edit'">edit</title>
<title py:if="page=='new'">new</title>
</head>
<body>
<span py:if="page=='new'">
<h1>New ${modelname}</h1>
${form(action='save', submit_text = "Create")}
</span>
<span py:if="page=='edit'">
<h1>Editing ${modelname}</h1>
${form(value=record, action=tg.url('../save/%s'%str(record.id)), submit_text = "Edit")}
</span>
<br/>
<div id="footbar" class="footbar" py:if="page=='edit'">
<a href="${tg.url('../show/%s'%record.id)}">Show</a> | <a href="${tg.url('../list')}">Back</a>
</div>
</body>
</html>
