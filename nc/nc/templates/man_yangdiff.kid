<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN"
          "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml"
      xmlns:py="http://purl.org/kid/ns#"
      py:extends="'master.kid'">
<head>
  <meta content="text/html; charset=utf-8"
    http-equiv="Content-Type" py:replace="''"/>
  <title>Netconf Central yangdiff manual</title>
</head>
<body>
<h1 align="center">yangdiff</h1>
<h2 align="center">Version 0.1.0</h2>
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
      <p>yangdiff &minus; 
	report semantic and syntactic changes between two 
	revisions of a YANG module
      </p>
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
<p><b>yangdiff [parameter=</b> <i>value</i> <b>...]</b></p>
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
<p><b>yangdiff</b> 
compares the semantics
and syntax between two revisions of the same
YANG module.  The conceptual data model is compared,
not the individual files.  
</p>
<p>
For example, unless statement order is significant,
changing the order is not considered a change, and 
is not reported.  Reformatted test (whitespace changes)
are also not reported.
</p>
<p>
If a data type definition is changed in form,
but not content, then a 'modify type' message
will be generated, but no additional sub-fields
will be reported.
</p>
<p>
This version of yangdiff supports
the YANG data modeling language defined in
<a href="http://www3.tools.ietf.org/html/draft-ietf-netmod-yang-00">
<b>draft-ietf-netmod-yang-00.txt</b></a>.
</p>
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
config file <i>/etc/yangdiff.conf</i> will be not be checked
if this parameter is present.</p>
</td></tr>
</table>
<!-- INDENTATION -->
<table width="100%" border="0" rules="none" frame="void"
       cols="2" cellspacing="0" cellpadding="0">
<tr valign="top" align="left">
<td width="10%"></td>
<td width="89%">
<p>--<b>old</b>=string</p></td></tr>
</table>
<!-- INDENTATION -->
<table width="100%" border="0" rules="none" frame="void"
       cols="2" cellspacing="0" cellpadding="0">
<tr valign="top" align="left">
<td width="21%"></td>
<td width="77%">
<p>
The older of the two revisions to compare.
</p>
<p>
If this parameter indicates a filename, then it
represents the YANG source module name 
to compare as the older of the two revisions.
</p>
<p>
If this parameter indicates a directory,
then it will be used to to search for a
file with the same name as identified by
the 'new' parameter.
</p>
<p>
If this string begins with a '~' character,
then a username is expected to follow or
a directory separator character.  If it begins
with a '$' character, then an environment variable
name is expected to follow.
</p>
<pre>

      ~/some/path ==&gt; &lt;my-home-dir&gt;/some/path

      ~fred/some/path ==&gt; &lt;fred-home-dir&gt;/some/path

</pre>
<p>
This parameter must be present unless the 'help'
or 'version' parameters are used.
</p>
</td></tr>
</table>
<!-- INDENTATION -->
<table width="100%" border="0" rules="none" frame="void"
       cols="2" cellspacing="0" cellpadding="0">
<tr valign="top" align="left">
<td width="10%"></td>
<td width="89%">
<p>--<b>new</b>=string</p></td></tr>
</table>
<!-- INDENTATION -->
<table width="100%" border="0" rules="none" frame="void"
       cols="2" cellspacing="0" cellpadding="0">
<tr valign="top" align="left">
<td width="21%"></td>
<td width="77%">
<p>
The newer of the two revisions to compare.
</p>
<p>
If this parameter indicates a filename, then it
represents the YANG source module name 
to compare as the newer of the two revisions.
</p>
<p>
If this parameter indicates a directory
(and the 'old' parameter indicates a filename),
then it will be used to to search for a
file with the same name as the 'new' parameter.
</p>
<p>
If the 'old' parameter identifies a directory
as well (and the 'no-subdirs' parameter is present),
then the modules within the 'new' directory will be 
compared to files with the same name in the 'old' 
directory.  If the 'no-subdirs' parameter is not
present, then all sub-directories within the 'src'
directory will also be checked.
</p>
<p>
If this string begins with a '~' character,
then a username is expected to follow or
a directory separator character.  If it begins
with a '$' character, then an environment variable
name is expected to follow.
</p>
<pre>

      ~/some/path ==&gt; &lt;my-home-dir&gt;/some/path

      ~fred/some/path ==&gt; &lt;fred-home-dir&gt;/some/path

</pre>
<p>
This parameter must be present unless the 'help'
or 'version' parameters are used.
</p>
</td></tr>
</table>
<table width="100%" border="0" rules="none" frame="void"
       cols="2" cellspacing="0" cellpadding="0">
<tr valign="top" align="left">
<td width="10%"></td>
<td width="89%">
<p>--<b>difftype</b>=string</p></td></tr>
</table>
<!-- INDENTATION -->
<table width="100%" border="0" rules="none" frame="void"
       cols="2" cellspacing="0" cellpadding="0">
<tr valign="top" align="left">
<td width="21%"></td>
<td width="77%">
<p>
The type of comparison output requested.<br/>
Allowed values are 'terse', 'normal', and 'revision'.
</p>
<p>
The basic format is:
</p>
<pre>

       [add/delete/modify] field-name [field-value]

</pre>
<p>
The 'terse' option will include the names
of the top-level fields that are different.
</p>
<pre>

         A foo  --&gt;  Added foo in new revision
         D foo  --&gt;  Deleted foo in new revision
         M foo  --&gt;  Modified foo in new revision (value too long)
         M foo from '0' to '1'  --&gt;  Modified foo in new revision

</pre>
<p>
The 'normal' option will also include any changes
for any nested fields or objects.  This is the default
option.
</p>
<p>
The 'revision' option will generate the differences report
in YANG revision-stmt format.  For example:
</p>
<pre>

      revision &lt;todays-date&gt; {
        description 
          \"
              - Added import baxtypes
              - Changed contact to 'support@acme.com'
              - Modified container myobjects
              - Added list first-list\";
      }

</pre>
</td></tr>
</table>
<!-- INDENTATION -->
<table width="100%" border="0" rules="none" frame="void"
       cols="2" cellspacing="0" cellpadding="0">
<tr valign="top" align="left">
<td width="10%"></td>
<td width="89%">
<p>--<b>output</b>=string</p></td></tr>
</table>
<!-- INDENTATION -->
<table width="100%" border="0" rules="none" frame="void"
       cols="2" cellspacing="0" cellpadding="0">
<tr valign="top" align="left">
<td width="21%"></td>
<td width="77%">
<p>
Output directory or file name to use. 
The default is STDOUT if none is specified.
</p>
<p>
If this parameter represents an existing directory,
then the default comparison output file (yangdiff.log)
will be generated in the specified directory.
</p>
<p>
If this parameter represents a file name,
then all comparison output will be directed
to the specified file.  If the file already exists,
it will be overwritten.
</p>
<p>
If this string begins with a '~' character,
then a username is expected to follow or
a directory separator character.  If it begins
with a '$' character, then an environment variable
name is expected to follow.
</p>
<pre>

      ~/some/path ==&gt; &lt;my-home-dir&gt;/some/path

      ~fred/some/path ==&gt; &lt;fred-home-dir&gt;/some/path

</pre>
</td></tr>
</table>

<table width="100%" border="0" rules="none" frame="void"
       cols="5" cellspacing="0" cellpadding="0">
<tr valign="top" align="left">
<td width="11%"></td>
<td width="8%">

<p>--<b>help</b></p>
</td>
<td width="13%"></td>
<td width="50%">

<p>Print yangdiff help file and exit.</p>
</td>
<td width="16%">
</td></tr>
</table>

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
<p>Print yangdiff version string and exit.</p>
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
<p>
To compare one module, use the 'old' and 'new'
parameters to specify YANG module files,
each with a filespec string ending with the '.yang'
file extension.  The filespecs must represent
different files.  If the 'old' parameter represents
a directory, then this directory will be searched
for the 'new' filename.
</p>
<p>
To compare all the modules in a subtree, use 
the 'old' and 'new' parameters to specify a directory
to be searched for YANG modules to be processed.
In this mode, each new module is compared to
a corresponding file within the 'old' subtree.
Also, dependency and include files
will be kept separate, for each subtree.
</p>
<p>           
Unless the 'help' or 'version' parameters is entered, 
the 'old' and 'new' parameters must be present.
</p>
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
<pre>  

  1) if the parameter for the file that generated the
     search request represents a subtree, search that
     subtree first.
  2) file is in the current directory
  3) YANG_MODPATH environment var (or set by modpath parameter)
  4) $(HOME)/modules directory
  5) $(YANG_HOME)/modules directory
  6) $(YANG_INSTALL)/modules directory OR
     default install module location, &rsquo;/usr/share/yang/modules&rsquo;

</pre>
<!-- INDENTATION -->
<p>
By default, the entire directory tree for all locations
(except step 1) will be searched, not just the specified
directory. The <b>no-subdirs</b> parameter can be used to
prevent sub-directories from being searched.
</p>
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
<p>
By default, any translation output will be sent to
<b>STDOUT</b>.
</p>

<p>
The <b>output</b> parameter can be used to specify the
full filespec of the output file,  or a
complete directory specification to be combined
with the default filename (yangdiff.log).
</p>
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
<p>The root of the directory that yangdiff is installed on
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
<p><b>yangdiff.conf</b></p></td></tr>
</table>
<!-- INDENTATION -->
<table width="100%" border="0" rules="none" frame="void"
       cols="2" cellspacing="0" cellpadding="0">
<tr valign="top" align="left">
<td width="21%"></td>
<td width="77%">
<p>YANG config file The default is:
<b>/etc/yangdiff.conf</b></p>
<!-- INDENTATION -->
<p>An ASCII configuration file format is supported to store
command line parameters.</p>
<!-- INDENTATION -->
<p>The <b>config</b> parameter is used to specify a specific
config file, otherwise the default config file will be
checked.</p>
<!-- INDENTATION -->
<pre>  
   - A hash mark until EOLN is treated as a comment
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
<pre>
     # this is a comment
     yangdiff {
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
<p>* <b>yangdiff.ncx</b> default:
/usr/share/yang/modules/netconfcentral/yangdiff.ncx</p>
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
       <li>keyref Xpath expressions are not validated</li>
       <li>must-stmt Xpath expressions are not validated</li>
       <li>sub-modules ignored in directory processing mode,
           and module comparisons do not include sub-modules;
           the 'unified' parameter will be added later to
           support this feature
       </li>
       <li>comparison does not examine beyond the current
           module (i.e. differences in imported modules);
           The exception are typedef chains, which are
           examined for semantic changes, regardless of
           module location of the typedef.
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
<p><b>pyang</b>(1)&nbsp;<b>smidump</b>(1)&nbsp;<b>yangdump</b>(1)</p>
</td></tr>
</table>
</body>
</html>
