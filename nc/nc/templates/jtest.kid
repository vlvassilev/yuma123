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
    <title>jquery test</title>
    <style type="text/css">
       a.test { font-weight: bold; }
    </style>
    <script type="text/javascript" src="/static/javascript/jquery.js"/>
    <script type="text/javascript">
      $(document).ready(function(){
         $("a").click(function(event){
            alert("Thanks for visiting!");
            event.preventDefault();
            $(this).hide("slow");
         });
      });
      $.get('myhtmlpage.html', function(){
        myCallBack(param1, param2);
      });
    </script>
  </head>
  <body>
    <div>
      <a href="http://jquery.com/">jQuery</a>
    </div>
  </body>
</html>
