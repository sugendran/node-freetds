FreeTDS driver for node.js
==========================

Introduction
------------

FreeTDS is an open library [FreeTDS](http://freetds.schemamania.org/) used to connect to Sybase Databases and Microsoft SQL Servers.  FreeTDS is used in [PHP](http://www.php.net/manual/en/mssql.requirements.php) and [Python](http://pymssql.sourceforge.net/) and now [nodejs](http://nodejs.org).

You can use node-freetds to connect to Microsoft Azure.


Example Code
-----------

	var sql = require('node-freetds');
	
	var azure = sql.FreeTDS("abcd1234.database.windows.net:1433", "MyDatabase");

	azure.login("username@abcd1234", "horsebatterystaplecorrect");
	azure.executeSql("SELECT * FROM sys.Tables", 
		function(res) {
			// Prints all tables for this database.
			console.log(res);
		}
	);

Install
-------

This module is not currently in the ``npm`` repository; we will post here if / when it does.  For now you can install using ``node-gyp`` (which is not a bad option either).

	cd node-freetds
	node-gyp configure
	node-gyp build
	node-gyp install


Requirements
------------

This driver assumes that you have the [freetds](http://www.freetds.org/) libraries installed.  With Ubuntu you may install it like so:

	apt-get install freetds-dev libsybdb5 libv8-dev nodejs-dev

For those of you with homebrew installed you can just do the following:

	brew install freetds


Recent Changes
--------------

2013-06-23

Significant changes were made v0.1.0.

* Updated for node.js 0.11.x
* Implemented libuv for asynchonous tasks
* Added header file to attempt to be more like node.js programming structure
* Objects wrapped using node::ObjectWrap


Issues
------

( _This is beta_ )
	
* Memory leaks
* No way to select database version (default is 8.0)
* No easy way to compile for Windows


Copyright
---------

[gpl](http://www.gnu.org/copyleft/gpl.html)
