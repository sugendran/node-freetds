FreeTDS driver for node.js
==========================

Introduction
------------

FreeTDS is an open library [FreeTDS](http://freetds.schemamania.org/) used to
connect to Sybase Databases and Microsoft SQL Servers.  FreeTDS is used in
[PHP](http://www.php.net/manual/en/mssql.requirements.php) and
[Python](http://pymssql.sourceforge.net/) and now NodeJS!

Example Code
-----------

	var sql = require('node-freetds');
	var http = require('http');
	
	const PROGRAM_NAME = "NodeJS_FreeTDS_Example";
	
	console.log(sql.version());
	
	var db = sql.login({
		userId: 'user name',
		password: 'password',
		server: 'server[\\instance]',
		database: 'database'
	});
	
	http.createServer(function(request, response) {
		
		sql.executeSql(db, "SELECT * FROM sys.Tables", function(error, results) {
			
			if(error) {
				console.log("Error: " + error);
				return;
			}
			
			for(var i=0; i<results.length; i++) {
				response.write("[" + i + "] => \n\n");
				for(col in results[i]) {
					response.write("[" + col + "] " + results[i][col] + "\n");
				}
				response.write("\n\n\n");
			}
			
			response.end();
			
		});
		
	}).listen(8081);
	
	//sql.logout(db);
	//sql.cleanup();

Install
-------

This module is not currently in the ``npm`` repository; we will post here if /
when it does.  For now you can install using ``node-gyp`` (which is not a bad
option either).

	cd node-freetds
	node-gyp configure
	node-gyp build
	node-gyp install


Requirements
------------

This driver assumes that you have the [freetds](http://www.freetds.org/)
libraries installed.

For those of you with homebrew installed you can just do the following:

	brew install freetds

Issues
------

### Symbol Lookup Error

`` node: symbol lookup error:
 [PATH TO]/node-freetds/build/Release/node-freetds.node: undefined symbol: 
 dbinit ``

You may experience issues when running if ``node-gyp`` is not able to find
``libsybdb.so``.  Within the the ``binding.gyp`` configuration is a shell
command for ``node-gyp`` which finds your installed ``libsybdb.so`` dynamic
library.

	"conditions": [ [
		'OS=="linux"', {
			"libraries": [
				"$(shell find /usr/lib -type l -name 'libsybdb.so')"
			]
		}]
	]

You will have issues running your NodeJS script if it is not installed.
Install it by with ``apt``:

	apt-get install freetds-dev libsybdb5

Copyright
---------

[gpl](http://www.gnu.org/copyleft/gpl.html)