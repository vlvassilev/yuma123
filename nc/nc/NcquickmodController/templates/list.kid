<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xmlns:py="http://purl.org/kid/ns#"
    py:extends="'master.kid'">
<head>
<meta content="text/html; charset=utf-8" http-equiv="Content-Type" py:replace="''"/>
<title>list</title>
</head>
<body>
<h1>Listing ${modelname}</h1>
<table>
    <tr>
        <th> ID </th>
        <th> module name </th>
    </tr>
    <tr py:for="i, record in enumerate(records)" class="${i%2 and 'odd' or 'even'}">
        <td>${record.id}</td>
        <td>${record.modname}</td>
        <td><a href="${tg.url('show/%s'%record.id)}">Show</a></td>
        <td><a href="${tg.url('edit/%s'%record.id)}">Edit</a></td>   
        <td><a href="${tg.url('destroy/%s'%record.id)}" onclick="if (confirm('Are you sure?')) { var f = document.createElement('form'); this.parentNode.appendChild(f); f.method = 'POST'; f.action = this.href; f.submit(); };return false;">Destroy</a></td>
    </tr>
</table>
<br/>
<a href="new">New ${modelname}</a>
<span py:for="page in tg.paginate.pages">
    <a py:if="page != tg.paginate.current_page"
        href="${tg.paginate.get_href(page)}">${page}</a>
    <b py:if="page == tg.paginate.current_page">${page}</b>
</span>
</body>
</html>
