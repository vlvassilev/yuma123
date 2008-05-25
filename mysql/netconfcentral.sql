#
# mySQL database
#
# Host: netconfcentral.org    Database: netconf
#--------------------------------------------------------
#
# 2008-02-16   abb   initial version
#
# identifier strings == 64 bytes
# version strings == 32 bytes
# object id format
#   modname/obj1/child1/child1-1/


CREATE TABLE ncquickmod (
  `id` int(11) NOT NULL auto_increment,
  `modname` varchar(64) NOT NULL,
  PRIMARY KEY  (`id`),
  UNIQUE KEY `ncquickmod_key` (`modname`)
);


CREATE TABLE ncmodule (
  `id` int(11) NOT NULL auto_increment,
  `modname` varchar(64) NOT NULL,
  `submodname` varchar(64) NOT NULL,
  `version` varchar(32) NOT NULL,
  `modprefix` varchar(64) NOT NULL,
  `namespace` varchar(255) NOT NULL,
  `organization` varchar(255) NOT NULL,
  `abstract` text,
  `description` text,
  `reference` text,
  `contact` text,
  `revcomment` text,
  `xsdurl` varchar(255),
  `docurl` varchar(255),
  `srcurl` varchar(255),
  `sourcespec` varchar(255),
  `sourcename` varchar(255),
  `iscurrent` boolean,
  `islatest` boolean,
  `ismod` boolean,
  `isyang` boolean,
  `created_on` datetime NOT NULL,
  `updated_on` datetime NOT NULL,
  PRIMARY KEY  (`id`),
  UNIQUE KEY `ncmodule_key` (`submodname`, `version`)
);


CREATE TABLE nctypedef (
  `id` int(11) NOT NULL auto_increment,
  `modname` varchar(64) NOT NULL,
  `submodname` varchar(64) NOT NULL,
  `version` varchar(32) NOT NULL,
  `name` varchar(64) NOT NULL,
  `linenum` int,
  `description` text NOT NULL,
  `reference` text,
  `docurl` varchar(255),
  `basetypename` varchar(32),
  `parenttypename` varchar(64),
  `parentmodname` varchar(64),
  `parentlinenum` int,
  `iscurrent` boolean,
  `islatest` boolean,
  `created_on` datetime NOT NULL,
  `updated_on` datetime NOT NULL,
  PRIMARY KEY  (`id`),
  UNIQUE KEY `typedef_key` (`modname`, `name`, `version`)
);


CREATE TABLE ncgrouping (
  `id` int(11) NOT NULL auto_increment,
  `modname` varchar(64) NOT NULL,
  `submodname` varchar(64) NOT NULL,
  `version` varchar(32) NOT NULL,
  `name` varchar(64) NOT NULL,
  `linenum` int,
  `description` text NOT NULL,
  `reference` text,
  `docurl` varchar(255),
  `objectlist` text,
  `iscurrent` boolean,
  `islatest` boolean,
  `created_on` datetime NOT NULL,
  `updated_on` datetime NOT NULL,
  PRIMARY KEY  (`id`),
  UNIQUE KEY `ncgrouping_key` (`modname`, `name`, `version`)
);


CREATE TABLE ncobject (
  `id` int(11) NOT NULL auto_increment,
  `modname` varchar(64) NOT NULL,
  `submodname` varchar(64) NOT NULL,
  `version` varchar(32) NOT NULL,
  `name` varchar(64) NOT NULL,
  `linenum` int,
  `objectid` varchar(900) NOT NULL,
  `description` text,
  `reference` text,
  `docurl` varchar(255),
  `objtyp` varchar(16) NOT NULL,
  `parentid` varchar(900) NOT NULL,
  `istop` boolean,
  `isdata` boolean,
  `typename` text,
  `augwhen` text,
  `childlist` text,
  `defval` text,
  `listkey` text,
  `config` boolean,
  `mandatory` boolean,
  `level` int,
  `minelements` int,
  `maxelements` int,
  `iscurrent` boolean,
  `islatest` boolean,
  `created_on` datetime NOT NULL,
  `updated_on` datetime NOT NULL,
  PRIMARY KEY  (`id`),
  UNIQUE KEY `ncobject_key` (`modname`, `objectid`,  `version`)
);


CREATE TABLE ncextension (
  `id` int(11) NOT NULL auto_increment,
  `modname` varchar(64) NOT NULL,
  `submodname` varchar(64) NOT NULL,
  `version` varchar(32) NOT NULL,
  `name` varchar(64) NOT NULL,
  `linenum` int,
  `description` text NOT NULL,
  `reference` text,
  `docurl` varchar(255),
  `argument` varchar(64),
  `yinelement` boolean,
  `iscurrent` boolean,
  `islatest` boolean,
  `created_on` datetime NOT NULL,
  `updated_on` datetime NOT NULL,
  PRIMARY KEY  (`id`),
  UNIQUE KEY `extension_key` (`modname`, `name`, `version`)
);


CREATE TABLE ncarticle (
  `id` int(11) NOT NULL auto_increment,
  `article_id` int(11) NOT NULL,
  `published` datetime NOT NULL,
  `title` varchar(255) NOT NULL,
  `author` varchar(255) NOT NULL,
  `newstext` text NOT NULL,
  `created_on` datetime NOT NULL,
  `updated_on` datetime NOT NULL,
  PRIMARY KEY  (`id`),
  UNIQUE KEY `article_key` (`article_id`)
);


CREATE TABLE ncdoc (
  `id` int(11) NOT NULL auto_increment,
  `docid` int(11) NOT NULL,
  `version` varchar(32) NOT NULL,
  `published` datetime NOT NULL,
  `title` varchar(255) NOT NULL,
  `author` varchar(255) NOT NULL,
  `description` text NOT NULL,
  `docurl` varchar(255),
  `sourcespec` varchar(255),
  `iscurrent` boolean,
  `islatest` boolean,
  `created_on` datetime NOT NULL,
  `updated_on` datetime NOT NULL,
  PRIMARY KEY  (`id`),
  UNIQUE KEY `docs_key` (`docid`, `version`)
);


CREATE TABLE nctool (
  `id` int(11) NOT NULL auto_increment,
  `toolid` int(11) NOT NULL,
  `version` varchar(32) NOT NULL,
  `published` datetime NOT NULL,
  `title` varchar(255) NOT NULL,
  `author` varchar(255) NOT NULL,
  `description` text NOT NULL,
  `tooldochome` varchar(255) NOT NULL,
  `tooldldhome` varchar(255) NOT NULL,
  `docurl` varchar(255),
  `iscurrent` boolean,
  `islatest` boolean,
  `created_on` datetime NOT NULL,
  `updated_on` datetime NOT NULL,
  PRIMARY KEY  (`id`),
  UNIQUE KEY `tools_key` (`toolid`, `version`)
);


