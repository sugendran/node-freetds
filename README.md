FreeTDS driver for node.js
==========================

Introduction
------------

FreeTDS is an open library [FreeTDS](http://freetds.schemamania.org/) used to connect to Sybase Databases and Microsoft SQL Servers.  FreeTDS is used in [PHP](http://www.php.net/manual/en/mssql.requirements.php) and [Python](http://pymssql.sourceforge.net/) and now [nodejs](http://nodejs.org).

You can use node-freetds to connect to Microsoft Azure.


Example Code
-----------

	var sql = require('node-freetds');
	
	var azure = new sql.FreeTDS("abcd1234.database.windows.net:1433", "MyDatabase");

	azure.login("username@abcd1234", "horsebatterystaplecorrect");
	azure.executeSql("SELECT * FROM sys.Tables", 
		function(res) {
			// Prints all tables for this database.
			console.log(res);
		}
	);

Requirements
------------

This driver assumes that you have the [freetds](http://www.freetds.org/) libraries installed.  With Ubuntu you may install it like so:

	apt-get install freetds-dev libsybdb5 libv8-dev nodejs-dev

For those of you with homebrew installed you can just do the following:

	brew install freetds


Install
-------

### Linux

This module is not currently in the ``npm`` repository; we will post here if / when it does.  For now you can install using ``node-gyp`` (which is not a bad option either).

	cd node-freetds
	node-gyp configure
	node-gyp build
	node-gyp install

### Windows

It helps a great deal to have GIT installed.  Most likely if you are reading this from the Github site then you already have GIT installed.  Otherwise get [GIT](http://git-scm.com/) and make sure to install it into ``Program Files`` *NOT* ``Program Files (x86)``.  The Windows install comes with a GIT terminal that you can use.  The rest of this documentation commands have been tested in the GIT terminal.

You will need to install [Python](http://www.python.org/download) (anything version < 3 will do).  Download the Windows x86-64 installer for [Python](http://www.python.org/ftp/python/2.7.5/python-2.7.5.amd64.msi) (or [32bit](http://www.python.org/ftp/python/2.7.5/python-2.7.5.msi)).  You may need to add the new installation to your environment ``PATH`` variable.  You can do this in the GIT terminal like so:

	export PATH="/c/Python27:$PATH"    # Make sure this is the path where you installed Python-2.7 to

Assuming that you have Microsoft .NET installed you will then also need to add ``MSBuild.exe`` to your ``PATH`` environment variable:

	export PATH="/c/Windows/Microsoft.NET/Framework64/v4.0.30319:$PATH"

Get [FreeTDS for Windows](sourceforge.net/projects/freetdswindows).  Better yet, build it yourself directly from [FreeTDS](http://www.freetds.org) using [MinGW-w64](http://mingw-w64.sourceforge.net/) or [Cygwin](http://www.cygwin.com).  The rest of this installation will cover how to build FreeTDS for Windows are required for this project.

Use the [MinGW-w64 installer](http://sourceforge.net/projects/mingwbuilds/files/mingw-builds-install/mingw-builds-install.exe/download) to install MinGW-w64 compiler.  In my case I had to select x64 architecture.  Add MinGW-w64 binaries to your environment ``PATH`` variable:

	export PATH="/c/Program Files/mingw-builds/x64-4.8.1-posix-seh-rev1/mingw64/bin:$PATH"
	# Use mingw32-make
	cp /c/Program Files/mingw-builds/x64-4.8.1-posix-seh-rev1/mingw64/bin/mingw32-make.exe \
		/c/Program Files/mingw-builds/x64-4.8.1-posix-seh-rev1/mingw64/bin/make.exe

This project is configured to look for the FreeTDS header files in a ``freetds-*`` folder.  Download the [FreeTDS source](ftp://ftp.freetds.org/pub/freetds/stable/freetds-stable.tgz) and then untar while in the node-freetds folder:

	cd [path to]/node-freetds    # This project's folder
	tar xf ~/Downloads/freetds-stable.tgz
	cd freetds-stable

Configure FreeTDS - there is no need to actually build it:

	./configure

The clone nodejs which yuo will need for thier header files:

	git submodule add https://github.com/jpyent/node.git

** Unfortunately I am getting an build error with ``node-gyp`` unable to find 'node_isolate' even though it is defined in the nodejs include file ``node_internals.h``.



Recent Changes
--------------

2013-06-23

Significant changes were made from v0.1.0.

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

THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
