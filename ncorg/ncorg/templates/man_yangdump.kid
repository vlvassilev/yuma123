<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN"
          "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml"
      xmlns:py="http://purl.org/kid/ns#"
      py:extends="'master.kid'">
<head>
  <meta content="text/html; charset=utf-8"
    http-equiv="Content-Type" py:replace="''"/>
  <title>Netconf Central yangdump manual</title>
</head>
<body>
<h1 align="center">yangdump</h1>
<h2 align="center">Version 0.9.1</h2>
<a href="#NAME">NAME</a><br/>
<a href="#SYNOPSIS">SYNOPSIS</a><br/>
<a href="#DESCRIPTION">DESCRIPTION</a><br/>
<a href="#USAGE">USAGE</a><br/>
<a href="#OPTIONS">OPTIONS</a><br/>
<a href="#INPUT FILES">INPUT FILES</a><br/>
<a href="#SEARCH PATH">SEARCH PATH</a><br/>
<a href="#OUTPUT MODES">OUTPUT MODES</a><br/>
<a href="#ERROR LOGGING">ERROR LOGGING</a><br/>
<a href="#ENVIRONMENT">ENVIRONMENT</a><br/>
<a href="#CONFIGURATION FILES">CONFIGURATION FILES</a><br/>
<a href="#FILES">FILES</a><br/>
<a href="#BUGS">BUGS</a><br/>
<a href="#DIAGNOSTICS">DIAGNOSTICS</a><br/>
<a href="#AUTHOR">AUTHOR</a><br/>
<a href="#SEE ALSO">SEE ALSO</a><br/>

<hr/>
<a name="NAME"></a>
<h2>NAME</h2>
<!-- INDENTATION -->
<table width="100%" border="0" rules="none" frame="void"
       cols="2" cellspacing="0" cellpadding="0">
  <tr valign="top" align="left">
    <td width="10%"></td>
    <td width="89%">
      <p>yangdump &minus; validate YANG modules and convert them
	to different formats</p>
    </td>
  </tr>
</table>
<a name="SYNOPSIS"></a>
<h2>SYNOPSIS</h2>
<!-- INDENTATION -->
<table width="100%" border="0" rules="none" frame="void"
       cols="2" cellspacing="0" cellpadding="0">
<tr valign="top" align="left">
<td width="10%"></td>
<td width="89%">
<p><b>yangdump [parameter=</b> <i>value</i> <b>...]</b></p>
</td></tr>
</table>
<a name="DESCRIPTION"></a>
<h2>DESCRIPTION</h2>
<!-- INDENTATION -->
<table width="100%" border="0" rules="none" frame="void"
       cols="2" cellspacing="0" cellpadding="0">
<tr valign="top" align="left">
<td width="10%"></td>
<td width="89%">
<p><b>yangdump</b> provides validation and translation of
YANG data models. Information about a module or submodule
can be generated as well. This version of yangdump supports
the YANG data modeling language defined in
<b>draft-bjorklund-netconf-yang-02.txt</b>.</p>
<!-- INDENTATION -->
<p>The <b>format</b> parameter is used to select a
translation output mode. If it is missing, then no
translation will be done.</p>
<!-- INDENTATION -->
<p>This parameter can be used with the module reports
parameters, but the translation output should be directed to
a file instead of STDOUT to keep them separated.</p>
<!-- INDENTATION -->
<p>For XSD 1.0 translation, use the <b>format=xsd</b>
parameter.</p>
<!-- INDENTATION -->
<p>For XHTML 1.0 translation, use the <b>format=html</b>
parameter.</p>
<!-- INDENTATION -->
<p>For a 1 line output of the module name and version, use
the <b>modversion</b> parameter.</p>
<!-- INDENTATION -->
<p>For a listing of all the symbols that the file exports to
other files, use the <b>exports</b> parameter.</p>
<!-- INDENTATION -->
<p>For a listing of all the files that the file depends on,
to compile, use the <b>dependencies</b> parameter.</p>
<!-- INDENTATION -->
<p>For a listing of all the accessible object identifiers
that the file contains, use the <b>identifiers</b>
parameter.</p>
</td></tr>
</table>
<a name="USAGE"></a>
<h2>USAGE</h2>
<!-- INDENTATION -->
<table width="100%" border="0" rules="none" frame="void"
       cols="2" cellspacing="0" cellpadding="0">
<tr valign="top" align="left">
<td width="10%"></td>
<td width="89%">
<p>Parameters can be entered in any order, and have the
form:</p>
<!-- INDENTATION -->
<p><b>[start] name [separator [value]]</b></p>
<!-- INDENTATION -->
<p>where:</p>
<!-- INDENTATION -->
<p><b>start</b> == 0, 1, or 2 dashes (foo, -foo, --foo)</p>
<!-- INDENTATION -->
<p><b>name</b> == parameter name</p>
<!-- INDENTATION -->
<pre>         Parameter name completion will be attempted
         if a partial name is entered.

</pre>
<!-- INDENTATION -->
<p><b>separator</b> == whitespace or equals sign (foo=bar,
foo bar)</p>
<!-- INDENTATION -->
<p><b>value</b> == string value for the parameter.</p>
<!-- INDENTATION -->
<pre>         Strings with whitespace need to be double quoted
         (--foo=&quot;some string&quot;)

</pre>
<!-- INDENTATION -->
<p>Some examples of valid command line parameters:</p>
<!-- INDENTATION -->
<pre>   foo=3
   -foo=3
   --foo=3
   foo 3
   foo=fred
   --foo &quot;fred flintstone&quot;
</pre>
<!-- INDENTATION -->
<p>Partial parameter names can be entered if they are
unique.</p>
</td></tr>
</table>
<a name="OPTIONS"></a>
<h2>OPTIONS</h2>
<!-- INDENTATION -->
<table width="100%" border="0" rules="none" frame="void"
       cols="2" cellspacing="0" cellpadding="0">
<tr valign="top" align="left">
<td width="10%"></td>
<td width="89%">
<p>--<b>config</b>=filespec</p></td></tr>
</table>
<!-- INDENTATION -->
<table width="100%" border="0" rules="none" frame="void"
       cols="2" cellspacing="0" cellpadding="0">
<tr valign="top" align="left">
<td width="21%"></td>
<td width="77%">
<p>The name of the configuration file to use. Any parameter
except this one can be set in the config file. The default
config file <i>/etc/yangdump.conf</i> will be not be checked
if this parameter is present.</p>
</td></tr>
</table>
<!-- INDENTATION -->
<table width="100%" border="0" rules="none" frame="void"
       cols="2" cellspacing="0" cellpadding="0">
<tr valign="top" align="left">
<td width="10%"></td>
<td width="89%">
<p>--<b>module</b>=string</p></td></tr>
</table>
<!-- INDENTATION -->
<table width="100%" border="0" rules="none" frame="void"
       cols="2" cellspacing="0" cellpadding="0">
<tr valign="top" align="left">
<td width="21%"></td>
<td width="77%">
<p>YANG or NCX source module name to validate and convert.
The <b>subtree</b> option cannot be used if this option is
present.</p>
</td></tr>
</table>
<!-- INDENTATION -->
<table width="100%" border="0" rules="none" frame="void"
       cols="2" cellspacing="0" cellpadding="0">
<tr valign="top" align="left">
<td width="10%"></td>
<td width="89%">
<p>--<b>subtree</b>=string</p></td></tr>
</table>
<!-- INDENTATION -->
<table width="100%" border="0" rules="none" frame="void"
       cols="2" cellspacing="0" cellpadding="0">
<tr valign="top" align="left">
<td width="21%"></td>
<td width="77%">
<p>Path specification of the directory subtree to convert.
All of the YANG and NCX source modules contained in the
specified directory sub-tree will be processed.</p>
<!-- INDENTATION -->
<p>If the <b>format</b> parameter is present, then one file
with the default name will be generated for each YANG file
found in the sub-tree.</p>
<!-- INDENTATION -->
<p>Note that symbolic links are not followed during the
directory traversal. Only real directories will be searched
and regular files will be checked as modules. Processing
will continue to the next file if a module contains
errors.</p>
</td></tr>
</table>
<!-- INDENTATION -->
<table width="100%" border="0" rules="none" frame="void"
       cols="2" cellspacing="0" cellpadding="0">
<tr valign="top" align="left">
<td width="10%"></td>
<td width="89%">
<p>--<b>output</b>=filespec</p></td></tr>
</table>
<!-- INDENTATION -->
<table width="100%" border="0" rules="none" frame="void"
       cols="2" cellspacing="0" cellpadding="0">
<tr valign="top" align="left">
<td width="21%"></td>
<td width="77%">
<p>Output file name to use. Default is STDOUT if none
specified and the <b>defname</b> parameter is also
missing.</p>
<!-- INDENTATION -->
<p>If this parameter represents an existing directory, then
the <b>defnames</b> parameter will be assumed by default,
and the translation output file(s) will be generated in the
specified directory.</p>
<!-- INDENTATION -->
<p>If this parameter represents a file name, then the
<b>defnames</b> parameter will be ignored, and all
translation output will be directed to the specified
file.</p>
</td></tr>
</table>
<!-- INDENTATION -->
<table width="100%" border="0" rules="none" frame="void"
       cols="2" cellspacing="0" cellpadding="0">
<tr valign="top" align="left">
<td width="10%"></td>
<td width="89%">
<p>--<b>defnames</b></p></td></tr>
</table>
<!-- INDENTATION -->
<table width="100%" border="0" rules="none" frame="void"
       cols="2" cellspacing="0" cellpadding="0">
<tr valign="top" align="left">
<td width="21%"></td>
<td width="77%">
<p>Output to a file with the default name for the format, in
the current directory.</p>
<!-- INDENTATION -->
<p>If the <b>output</b> parameter is present and represents
an existing directory, then the default filename will be
created in that directory, instead of the current
directory.</p>
</td></tr>
</table>
<!-- INDENTATION -->
<table width="100%" border="0" rules="none" frame="void"
       cols="2" cellspacing="0" cellpadding="0">
<tr valign="top" align="left">
<td width="10%"></td>
<td width="89%">
<p>--<b>format</b>=string</p></td></tr>
</table>
<!-- INDENTATION -->
<table width="100%" border="0" rules="none" frame="void"
       cols="2" cellspacing="0" cellpadding="0">
<tr valign="top" align="left">
<td width="21%"></td>
<td width="77%">
<p>Type of conversion desired, if any. If this parameter is
missing, then no translation will be done, but the module
will be validated, and any requested reports will be
generated.</p>
<!-- INDENTATION -->
<p>The following translation formats are available:</p>
<!-- INDENTATION -->
<pre>   xsd :  XSD 1.0
   html:  XHTML 1.0
   yang:  Canonical YANG  (in progress)
   sqldb: Netconf Central database SQL output (in progress)

</pre>
</td></tr>
</table>
<!-- INDENTATION -->

<table width="100%" border="0" rules="none" frame="void"
       cols="2" cellspacing="0" cellpadding="0">
<tr valign="top" align="left">
<td width="10%"></td>
<td width="89%">
<p>--<b>modversion</b></p></td></tr>
</table>
<!-- INDENTATION -->
<table width="100%" border="0" rules="none" frame="void"
       cols="2" cellspacing="0" cellpadding="0">
<tr valign="top" align="left">
<td width="21%"></td>
<td width="77%">
<p>Validate the file, write the [sub]module name, version
and source filespec, then exit.</p>
</td></tr>
</table>
<!-- INDENTATION -->
<table width="100%" border="0" rules="none" frame="void"
       cols="2" cellspacing="0" cellpadding="0">
<tr valign="top" align="left">
<td width="10%"></td>
<td width="89%">
<p>--<b>exports</b></p></td></tr>
</table>
<!-- INDENTATION -->
<table width="100%" border="0" rules="none" frame="void"
       cols="2" cellspacing="0" cellpadding="0">
<tr valign="top" align="left">
<td width="21%"></td>
<td width="77%">
<p>Validate the file, write information for the symbols that
this [sub]module exports, then exit. Report includes the
following info for the specific file, not the entire module,
if submodules are used:</p>
<!-- INDENTATION -->
<pre>   - [sub]module name
   - version
   - source filespec
   - namespace (module only)
   - prefix (module only)
   - belongs-to (submodule only)
   - typedefs
   - groupings
   - objects, rpcs, notifications
   - extensions

</pre>
</td></tr>
</table>
<!-- INDENTATION -->

<table width="100%" border="0" rules="none" frame="void"
       cols="2" cellspacing="0" cellpadding="0">
<tr valign="top" align="left">
<td width="10%"></td>
<td width="89%">
<p>--<b>dependencies</b></p></td></tr>
</table>
<!-- INDENTATION -->
<table width="100%" border="0" rules="none" frame="void"
       cols="2" cellspacing="0" cellpadding="0">
<tr valign="top" align="left">
<td width="21%"></td>
<td width="77%">
<p>Validate the file, write the module name, version and
module source for each file that this [sub]module imports
and includes, then exit.</p>
<!-- INDENTATION -->
<p>Each dependency type, name, version, and source is listed
once.</p>
<!-- INDENTATION -->
<p>If the dependency version and source are missing, then
that import or include file was not found.</p>
</td></tr>
</table>
<!-- INDENTATION -->
<table width="100%" border="0" rules="none" frame="void"
       cols="2" cellspacing="0" cellpadding="0">
<tr valign="top" align="left">
<td width="10%"></td>
<td width="89%">
<p>--<b>identifiers</b></p></td></tr>
</table>
<!-- INDENTATION -->
<table width="100%" border="0" rules="none" frame="void"
       cols="2" cellspacing="0" cellpadding="0">
<tr valign="top" align="left">
<td width="21%"></td>
<td width="77%">
<p>Validate the file, write the list of object identifiers,
that this [sub]module contains, then exit.</p>
<!-- INDENTATION -->
<p>Each accessible object node is listed once, including all
child nodes. Notifications and RPC methods are considered
top-level objects, and have object identifiers as well as
configuration and state data..</p>
</td></tr>
</table>
<!-- TABS -->
<table width="100%" border="0" rules="none" frame="void"
       cols="5" cellspacing="0" cellpadding="0">
<tr valign="top" align="left">
<td width="11%"></td>
<td width="8%">

<p>--<b>help</b></p>
</td>
<td width="13%"></td>
<td width="50%">

<p>Print yangdump help file and exit.</p>
</td>
<td width="16%">
</td></tr>
</table>
<!-- INDENTATION -->
<table width="100%" border="0" rules="none" frame="void"
       cols="2" cellspacing="0" cellpadding="0">
<tr valign="top" align="left">
<td width="10%"></td>
<td width="89%">
<p>--<b>html-div</b></p></td></tr>
</table>
<!-- INDENTATION -->
<table width="100%" border="0" rules="none" frame="void"
       cols="2" cellspacing="0" cellpadding="0">
<tr valign="top" align="left">
<td width="21%"></td>
<td width="77%">
<p>If HTML translation is requested, then this parameter
will cause the output to be a single &lt;div&gt; element,
instead of an entire HTML file. This allows the HTML
translation to be easily integrated within more complex WEB
pages, but the proper CSS definitions need to be present for
the HTML to render properly.</p>
<!-- INDENTATION -->
<p>The default filename extension will be &rsquo;.div&rsquo;
instead of &rsquo;.html&rsquo; if this parameter is present.
The contents will be well-formed XHTML 1.0, but without any
namespace declarations.</p>
</td></tr>
</table>
<!-- INDENTATION -->
<table width="100%" border="0" rules="none" frame="void"
       cols="2" cellspacing="0" cellpadding="0">
<tr valign="top" align="left">
<td width="10%"></td>
<td width="89%">
<p>--<b>html-toc</b>=string</p></td></tr>
</table>
<!-- INDENTATION -->
<table width="100%" border="0" rules="none" frame="void"
       cols="2" cellspacing="0" cellpadding="0">
<tr valign="top" align="left">
<td width="21%"></td>
<td width="77%">
<p>The HTML Table of Contents output mode. Ignored unless
the <b>format</b> parameter is set to <b>html</b>. Default
is <b>menu</b>.</p>
<!-- INDENTATION -->
<p>Values:</p>
<!-- INDENTATION -->
<pre>   - none: no ToC generated
   - plain: plain list ToC generated
   - menu: drop-down menu ToC generated.

</pre>
</td></tr>
</table>
<!-- INDENTATION -->

<table width="100%" border="0" rules="none" frame="void"
       cols="2" cellspacing="0" cellpadding="0">
<tr valign="top" align="left">
<td width="10%"></td>
<td width="89%">
<p>--<b>indent</b>=number</p></td></tr>
</table>
<!-- INDENTATION -->
<table width="100%" border="0" rules="none" frame="void"
       cols="2" cellspacing="0" cellpadding="0">
<tr valign="top" align="left">
<td width="21%"></td>
<td width="77%">
<p>Number of spaces to indent (0..9) in formatted output.
The default is 3 spaces.</p>
</td></tr>
</table>
<!-- INDENTATION -->
<table width="100%" border="0" rules="none" frame="void"
       cols="2" cellspacing="0" cellpadding="0">
<tr valign="top" align="left">
<td width="10%"></td>
<td width="89%">
<p>--<b>log</b>=filespec</p></td></tr>
</table>
<!-- INDENTATION -->
<table width="100%" border="0" rules="none" frame="void"
       cols="2" cellspacing="0" cellpadding="0">
<tr valign="top" align="left">
<td width="21%"></td>
<td width="77%">
<p>Filespec for the log file to use instead of STDOUT.</p>
</td></tr>
</table>
<!-- INDENTATION -->
<table width="100%" border="0" rules="none" frame="void"
       cols="2" cellspacing="0" cellpadding="0">
<tr valign="top" align="left">
<td width="10%"></td>
<td width="89%">
<p>--<b>log-append</b></p></td></tr>
</table>
<!-- INDENTATION -->
<table width="100%" border="0" rules="none" frame="void"
       cols="2" cellspacing="0" cellpadding="0">
<tr valign="top" align="left">
<td width="21%"></td>
<td width="77%">
<p>If present, the log will be appended not over-written. If
not, the log will be over-written. Only meaningful if the
<b>log</b> parameter is also present.</p>
</td></tr>
</table>
<!-- INDENTATION -->
<table width="100%" border="0" rules="none" frame="void"
       cols="2" cellspacing="0" cellpadding="0">
<tr valign="top" align="left">
<td width="10%"></td>
<td width="89%">
<p>--<b>log-level</b>=enum</p></td></tr>
</table>
<!-- INDENTATION -->
<table width="100%" border="0" rules="none" frame="void"
       cols="2" cellspacing="0" cellpadding="0">
<tr valign="top" align="left">
<td width="21%"></td>
<td width="77%">
<p>Sets the debug logging level for the program.</p>
</td></tr>
</table>
<!-- INDENTATION -->
<table width="100%" border="0" rules="none" frame="void"
       cols="2" cellspacing="0" cellpadding="0">
<tr valign="top" align="left">
<td width="10%"></td>
<td width="89%">
<p>--<b>modpath</b>=list</p></td></tr>
</table>
<!-- INDENTATION -->
<table width="100%" border="0" rules="none" frame="void"
       cols="2" cellspacing="0" cellpadding="0">
<tr valign="top" align="left">
<td width="21%"></td>
<td width="77%">
<p>Directory search path for YANG and NCX modules.</p>
</td></tr>
</table>
<!-- INDENTATION -->
<table width="100%" border="0" rules="none" frame="void"
       cols="2" cellspacing="0" cellpadding="0">
<tr valign="top" align="left">
<td width="10%"></td>
<td width="89%">
<p>--<b>objview</b>=string</p></td></tr>
</table>
<!-- INDENTATION -->
<table width="100%" border="0" rules="none" frame="void"
       cols="2" cellspacing="0" cellpadding="0">
<tr valign="top" align="left">
<td width="21%"></td>
<td width="77%">
<p>Determines how objects are generated in HTML and YANG
outputs. The default mode is the <b>raw</b> view. XSD output
is always <b>cooked</b>, since refined groupings and
locally-scoped definitions are not supported in XSD.
Values:</p>
<!-- INDENTATION -->
<pre>   raw -- output includes augment and uses clauses, not the
          expanded results of those clauses.

  cooked -- output does not include augment or uses clauses,
            just the objects generated from those clauses.
</pre>
</td></tr>
</table>
<!-- INDENTATION -->

<table width="100%" border="0" rules="none" frame="void"
       cols="2" cellspacing="0" cellpadding="0">
<tr valign="top" align="left">
<td width="10%"></td>
<td width="89%">
<p>--<b>no-subdirs</b></p></td></tr>
</table>
<!-- INDENTATION -->
<table width="100%" border="0" rules="none" frame="void"
       cols="2" cellspacing="0" cellpadding="0">
<tr valign="top" align="left">
<td width="21%"></td>
<td width="77%">
<p>If present, the file search paths for modules, scripts,
and data files will not include sub-directories if they
exist in the specified path.</p>
<!-- INDENTATION -->
<p>If missing, then these file search paths will include
sub-directories, if present. Any directory name beginning
with a dot (<b>.</b>) character, or named <b>CVS</b>, will
be ignored.</p>
</td></tr>
</table>
<!-- INDENTATION -->
<table width="100%" border="0" rules="none" frame="void"
       cols="2" cellspacing="0" cellpadding="0">
<tr valign="top" align="left">
<td width="10%"></td>
<td width="89%">
<p>--<b>no-versionnames</b></p></td></tr>
</table>
<!-- INDENTATION -->
<table width="100%" border="0" rules="none" frame="void"
       cols="2" cellspacing="0" cellpadding="0">
<tr valign="top" align="left">
<td width="21%"></td>
<td width="77%">
<p>If present, the default filenames will not contain the
module version string. Normally, the [sub]module name and
version string are both used to generate a default file
name, when the <b>defnames</b> output parameter is used.
This flag will cause filenames and links to be generated
which do not contain the version string.</p>
</td></tr>
</table>
<!-- INDENTATION -->
<table width="100%" border="0" rules="none" frame="void"
       cols="2" cellspacing="0" cellpadding="0">
<tr valign="top" align="left">
<td width="10%"></td>
<td width="89%">
<p>--<b>simurls</b></p></td></tr>
</table>
<!-- INDENTATION -->
<table width="100%" border="0" rules="none" frame="void"
       cols="2" cellspacing="0" cellpadding="0">
<tr valign="top" align="left">
<td width="21%"></td>
<td width="77%">
<p>If HTML translation is requested, then this parameter
will cause the format of URLs within links to be generated
in simplified form, for WEB development engines such as
CherryPy, which support this format.</p>
<!-- INDENTATION -->
<pre>   Normal URL format:
     example.html?parm1=foo&amp;parm2=bar#frag

  Simplified URL format:
     example/foo/bar#frag

</pre>
</td></tr>
</table>
<!-- INDENTATION -->

<table width="100%" border="0" rules="none" frame="void"
       cols="2" cellspacing="0" cellpadding="0">
<tr valign="top" align="left">
<td width="10%"></td>
<td width="89%">
<p>--<b>unified</b></p></td></tr>
</table>
<!-- INDENTATION -->
<table width="100%" border="0" rules="none" frame="void"
       cols="2" cellspacing="0" cellpadding="0">
<tr valign="top" align="left">
<td width="21%"></td>
<td width="77%">
<p>If present, then submodules will be processed within the
main module, in a unified report, instead of separately, one
report for each file.</p>
<!-- INDENTATION -->
<p>For translation purposes, this parameter will cause any
sub-modules to be treated as if they were defined in the
main module. Actual definitions will be generated instead of
an &rsquo;include&rsquo; directive, for each submodule.</p>
<!-- INDENTATION -->
<p>Normally, a separate output file is generated for each
input file, so that XSD output and other reports for a main
module will not include information for submodules.</p>
<!-- INDENTATION -->
<p>If this mode is selected, then submodules entered with
the <b>module</b> parameter will be ignored.</p>
</td></tr>
</table>
<!-- INDENTATION -->
<table width="100%" border="0" rules="none" frame="void"
       cols="2" cellspacing="0" cellpadding="0">
<tr valign="top" align="left">
<td width="10%"></td>
<td width="89%">
<p>--<b>urlstart</b>=string</p></td></tr>
</table>
<!-- INDENTATION -->
<table width="100%" border="0" rules="none" frame="void"
       cols="2" cellspacing="0" cellpadding="0">
<tr valign="top" align="left">
<td width="21%"></td>
<td width="77%">
<p>If present, then this string will be used to prepend to
HREF links and URLs generated for SQL and HTML translation.
It is expected to be a URL ending with a directory path. The
trailing separator &rsquo;/&rsquo; will be added if it is
missing.</p>
<!-- INDENTATION -->
<p>If not present (the default), then relative URLs,
starting with the file name will be generated instead.</p>
<!-- INDENTATION -->
<p>For example, if this parameter is set to</p>
<!-- INDENTATION -->
<pre>  &rsquo;http://acme.com/public&rsquo;

</pre>
<!-- INDENTATION -->
<p>then the URL generated for the &rsquo;bar&rsquo; type on
line 53, in the module FOO (version 2008-01-01) would
be:</p>
<!-- INDENTATION -->
<pre>  if no-versionnames set:

   &rsquo;http://acme.com/public/FOO.html#bar.53&rsquo;

  OR

 if no-versionnames not set (default):

  &rsquo;http://acme.com/public/FOO_2008-01-01.html#bar.53&rsquo;

</pre>
</td></tr>
</table>
<!-- INDENTATION -->

<table width="100%" border="0" rules="none" frame="void"
       cols="2" cellspacing="0" cellpadding="0">
<tr valign="top" align="left">
<td width="10%"></td>
<td width="89%">
<p>--<b>version</b></p></td></tr>
</table>
<!-- INDENTATION -->
<table width="100%" border="0" rules="none" frame="void"
       cols="2" cellspacing="0" cellpadding="0">
<tr valign="top" align="left">
<td width="21%"></td>
<td width="77%">
<p>Print yangdump version string and exit.</p>
</td></tr>
</table>
<!-- INDENTATION -->
<table width="100%" border="0" rules="none" frame="void"
       cols="2" cellspacing="0" cellpadding="0">
<tr valign="top" align="left">
<td width="10%"></td>
<td width="89%">
<p>--<b>xsd-schemaloc</b>=string</p></td></tr>
</table>
<!-- INDENTATION -->
<table width="100%" border="0" rules="none" frame="void"
       cols="2" cellspacing="0" cellpadding="0">
<tr valign="top" align="left">
<td width="21%"></td>
<td width="77%">
<p>If present, then this string will be used to prepend to
output XSD filenames, when generating schemaLocation
clauses. It is expected to be a URL ending with a directory
path. The trailing separator &rsquo;/&rsquo; will be added
if it is missing. This parameter is also prepended to URLs
generated fpr include and import directives within the
XSD.</p>
<!-- INDENTATION -->
<p>If not present (the default), then the schemaLocation
element is not generated during XSD translation. Relative
URLs for include and import directives will be generated,
starting with the file name.</p>
<!-- INDENTATION -->
<p>For example, if this parameter is set to</p>
<!-- INDENTATION -->
<pre>  &rsquo;http://acme.com/public&rsquo;

</pre>
<!-- INDENTATION -->
<p>then the schemaLocation XSD for the module FOO (version
01-01-2008) would be:</p>
<!-- INDENTATION -->
<pre>   if no-versionnames set:

     &rsquo;http://acme.com/public/FOO.xsd&rsquo;

 OR

  if no-versionnames not set (default):

     &rsquo;http://acme.com/public/FOO_2008-01-01.xsd&rsquo;
</pre>
</td></tr>
</table>
<a name="INPUT FILES"></a>
<h2>INPUT FILES</h2>
<!-- INDENTATION -->

<table width="100%" border="0" rules="none" frame="void"
       cols="2" cellspacing="0" cellpadding="0">
<tr valign="top" align="left">
<td width="10%"></td>
<td width="89%">
<p>Operations can be performed on one or more files with the
<b>module</b> parameter, or an entire directory tree with
the <b>subtree</b> parameter. Unless the <b>help</b> or
<b>version</b> parameters is entered, one of these input
file parameters is mandatory.</p>
</td></tr>
</table>
<a name="SEARCH PATH"></a>
<h2>SEARCH PATH</h2>
<!-- INDENTATION -->
<table width="100%" border="0" rules="none" frame="void"
       cols="2" cellspacing="0" cellpadding="0">
<tr valign="top" align="left">
<td width="10%"></td>
<td width="89%">
<p>When a module name is entered as input, or when a module
or submodule name is specified in an import or include
statement within the file, the following search algorithm is
used to find the file:</p>
<!-- INDENTATION -->
<pre>  1) file is in the current directory
  2) YANG_MODPATH environment var (or set by modpath parameter)
  3) $(HOME)/modules directory
  4) $(YANG_HOME)/modules directory
  5) $(YANG_INSTALL)/modules directory OR
     default install module location, &rsquo;/usr/share/yang/modules&rsquo;

</pre>
<!-- INDENTATION -->
<p>By default, the entire directory tree for all locations
(except step 1) will be searched, not just the specified
directory. The <b>no-subdirs</b> parameter can be used to
prevent sub-directories from being searched.</p>
<!-- INDENTATION -->
<p>Any directory name beginning with a dot character
(<b>.</b>) will be skipped. Also, any directory named
<b>CVS</b> will be skipped in directory searches.</p>
</td></tr>
</table>
<a name="OUTPUT MODES"></a>
<h2>OUTPUT MODES</h2>
<!-- INDENTATION -->
<table width="100%" border="0" rules="none" frame="void"
       cols="2" cellspacing="0" cellpadding="0">
<tr valign="top" align="left">
<td width="10%"></td>
<td width="89%">
<p>By default, any translation output will be sent to
<b>STDOUT</b>.</p>
<!-- INDENTATION -->
<p>The <b>output</b> parameter can be used to specify the
full filespec of the output file to use instead.</p>
<!-- INDENTATION -->
<p>The <b>defname</b> parameter can be used to generate a
default filename in the current directory for the
output.</p>
<!-- INDENTATION -->
<p>E.g., the default XSD filename is
<b>&lt;name&gt;_&lt;version&gt;.xsd</b>.</p>
<!-- INDENTATION -->
<p>This is the default mode when <b>subtree</b> input mode
is selected.</p>
</td></tr>
</table>
<a name="ERROR LOGGING"></a>
<h2>ERROR LOGGING</h2>
<!-- INDENTATION -->
<table width="100%" border="0" rules="none" frame="void"
       cols="2" cellspacing="0" cellpadding="0">
<tr valign="top" align="left">
<td width="10%"></td>
<td width="89%">
<p>By default, warnings and errors are sent to STDOUT.</p>
<!-- INDENTATION -->
<p>A log file can be specified instead with the
<b>log</b>&rsquo; parameter.</p>
<!-- INDENTATION -->
<p>Existing log files can be reused with the
&rsquo;logappend&rsquo; parameter, otherwise log files are
overwritten.</p>
<!-- INDENTATION -->
<p>The logging level can be controlled with the
<b>log-level</b> parameter.</p>
<!-- INDENTATION -->
<p>The default log level is &rsquo;info&rsquo;. The
log-levels are additive:</p>
<!-- INDENTATION -->
<pre>     off:    suppress all errors (not recommended!)
             A program return code of &rsquo;1&rsquo; indicates some error.
     error:  print errors
     warn:   print warnings
     info:   print generally interesting trace info
     debug:  print general debugging trace info
     debug2: print verbose debugging trace info

</pre>
</td></tr>
</table>
<a name="ENVIRONMENT"></a>
<h2>ENVIRONMENT</h2>
<!-- INDENTATION -->

<table width="100%" border="0" rules="none" frame="void"
       cols="2" cellspacing="0" cellpadding="0">
<tr valign="top" align="left">
<td width="10%"></td>
<td width="89%">
<p>The following optional environment variables can be used
to control module search behavior:</p>
</td></tr>
</table>
<!-- TABS -->
<table width="100%" border="0" rules="none" frame="void"
       cols="5" cellspacing="0" cellpadding="0">
<tr valign="top" align="left">
<td width="11%"></td>
<td width="5%">

<p><b>HOME</b></p>
</td>
<td width="13%"></td>
<td width="66%">

<p>The user&rsquo;s home directory (e.g., /home/andy)</p>
</td>
<td width="2%">
</td></tr>
</table>
<!-- INDENTATION -->
<table width="100%" border="0" rules="none" frame="void"
       cols="2" cellspacing="0" cellpadding="0">
<tr valign="top" align="left">
<td width="10%"></td>
<td width="89%">
<p><b>YANG_HOME</b></p></td></tr>
</table>
<!-- INDENTATION -->
<table width="100%" border="0" rules="none" frame="void"
       cols="2" cellspacing="0" cellpadding="0">
<tr valign="top" align="left">
<td width="21%"></td>
<td width="77%">
<p>The root of the user&rsquo;s YANG work directory (e.g.,
/home/andy/swdev/netconf)</p>
</td></tr>
</table>
<!-- INDENTATION -->
<table width="100%" border="0" rules="none" frame="void"
       cols="2" cellspacing="0" cellpadding="0">
<tr valign="top" align="left">
<td width="10%"></td>
<td width="89%">
<p><b>YANG_INSTALL</b></p></td></tr>
</table>
<!-- INDENTATION -->
<table width="100%" border="0" rules="none" frame="void"
       cols="2" cellspacing="0" cellpadding="0">
<tr valign="top" align="left">
<td width="21%"></td>
<td width="77%">
<p>The root of the directory that yangdump is installed on
this system (default is, /usr/share/yang)</p>
</td></tr>
</table>
<!-- INDENTATION -->
<table width="100%" border="0" rules="none" frame="void"
       cols="2" cellspacing="0" cellpadding="0">
<tr valign="top" align="left">
<td width="10%"></td>
<td width="89%">
<p><b>YANG_MODPATH</b></p></td></tr>
</table>
<!-- INDENTATION -->
<table width="100%" border="0" rules="none" frame="void"
       cols="2" cellspacing="0" cellpadding="0">
<tr valign="top" align="left">
<td width="21%"></td>
<td width="77%">
<p>Colon-separated list of directories to search for modules
and submodules.</p>
<!-- INDENTATION -->
<p>(e.g.:
&rsquo;./workdir/modules:/home/andy/test-modules&rsquo;)</p>
<!-- INDENTATION -->
<p>The <b>modpath</b> parameter will override this
environment variable, if both are present.</p>
</td></tr>
</table>
<a name="CONFIGURATION FILES"></a>
<h2>CONFIGURATION FILES</h2>
<!-- INDENTATION -->
<table width="100%" border="0" rules="none" frame="void"
       cols="2" cellspacing="0" cellpadding="0">
<tr valign="top" align="left">
<td width="10%"></td>
<td width="89%">
<p><b>yangdump.conf</b></p></td></tr>
</table>
<!-- INDENTATION -->
<table width="100%" border="0" rules="none" frame="void"
       cols="2" cellspacing="0" cellpadding="0">
<tr valign="top" align="left">
<td width="21%"></td>
<td width="77%">
<p>YANG config file The default is:
<b>/etc/yangdump.conf</b></p>
<!-- INDENTATION -->
<p>An ASCII configuration file format is supported to store
command line parameters.</p>
<!-- INDENTATION -->
<p>The <b>config</b> parameter is used to specify a specific
config file, otherwise the default config file will be
checked.</p>
<!-- INDENTATION -->
<pre>   - A hash mark until EOLN is treated as a comment
   - All text is case-sensitive
   - Whitespace within a line is not significant
   - Whitespace to end a line is significant/
     Unless the line starts a multi-line string,
     an escaped EOLN (backslash EOLN) is needed
     to enter a leaf on multiple lines.
   - For parameters that define lists, the key components
     are listed just after the parameter name, without
     any name,  e.g.,

           interface eth0 {
              # name = eth0 is not listed inside the braces
              ifMtu 1500
              ifName mySystem
            }

</pre>
<!-- INDENTATION -->
<p>A config file can contain any number of parameter sets
for different programs.</p>
<!-- INDENTATION -->
<p>Each program must have its own section, identifies by its
name:</p>
<!-- INDENTATION -->
<pre>     # this is a comment
     yangdump {
        log-level debug
        output &quot;~/swdev/testfiles&quot;
     }

    netconfd {
        ...
     }

</pre>
</td></tr>
</table>
<a name="FILES"></a>
<h2>FILES</h2>
<!-- INDENTATION -->

<table width="100%" border="0" rules="none" frame="void"
       cols="2" cellspacing="0" cellpadding="0">
<tr valign="top" align="left">
<td width="10%"></td>
<td width="89%">
<p>The following data files must be present in the module
search path in order for this program to function:</p>
<!-- INDENTATION -->
<p>* <b>yangdump.ncx</b> default:
/usr/share/yang/modules/netconfcentral/yangdump.ncx</p>
<!-- INDENTATION -->
<p>* <b>ncxtypes.ncx</b> default:
/usr/share/yang/modules/netconfcentral/ncxtypes.ncx</p>
</td></tr>
</table>
<a name="BUGS"></a>
<h2>BUGS</h2>
<!-- INDENTATION -->
<table width="100%" border="0" rules="none" frame="void"
       cols="2" cellspacing="0" cellpadding="0">
  <tr valign="top" align="left">
    <td width="10%"></td>
    <td width="89%">
      <ul>
	<li><b>keyref</b> Xpath expressions are not validated</li>
	<li><b>must-stmt</b> Xpath expressions are not validated</li>
	<li>
	  <b>yangdump.ncx and ncxtypes.ncx</b> must be installed under
	  <b>~/modules</b> or <b>/usr/share/yang/modules</b> directories,
	  or else the
	  <b>YANG_HOME</b> environment variable must be used to specify
	  the alternate location of these files. The <b>modpath</b>
	  parameter does not work for these two 'bootstrap' modules.
	</li>
      </ul>
    </td>
  </tr>
</table>
<a name="DIAGNOSTICS"></a>
<h2>DIAGNOSTICS</h2>
<!-- INDENTATION -->
<table width="100%" border="0" rules="none" frame="void"
       cols="2" cellspacing="0" cellpadding="0">
<tr valign="top" align="left">
<td width="10%"></td>
<td width="89%">
<p>Internal diagnostics may generate the following type of
message if any bugs are detected at runtime:</p>
<!-- INDENTATION -->
<p>[E0] filename.c:linenum error-number (error-msg)</p>
</td></tr>
</table>
<a name="AUTHOR"></a>
<h2>AUTHOR</h2>
<!-- INDENTATION -->
<table width="100%" border="0" rules="none" frame="void"
       cols="2" cellspacing="0" cellpadding="0">
<tr valign="top" align="left">
<td width="10%"></td>
<td width="89%">
<p>Andy Bierman, &lt;andy at netconfcentral dot com&gt;</p>
</td></tr>
</table>
<a name="SEE ALSO"></a>
<h2>SEE ALSO</h2>
<!-- INDENTATION -->
<table width="100%" border="0" rules="none" frame="void"
       cols="2" cellspacing="0" cellpadding="0">
<tr valign="top" align="left">
<td width="10%"></td>
<td width="89%">
<p><b>pyang</b>(1) <b>smidump</b>(1)</p>
</td></tr>
</table>
</body>
</html>
