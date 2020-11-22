**# README by Datatrans #**

Codes have been changed to adapt PostgreSQL internal changes up to version 13.1:
- No more relhasoids in pg_class.
- No more cache_value, is_cycled, is_called in sequence object (since PostgreSQL 11).
- No more adsrc in pg_attrdef, it should be calculated as pg_catalog.pg_get_expr(adbin, adrelid) instead.
- Declarative Table Partitioning DDL.

**# pgAdmin3 LTS by BigSQL README #**

This is a fork of the pgAdmin3 project that aims to continue to support 
PostgreSQL 8.4 thru 12.0 with Windows and OSX Binaries.  We are in basic
maintenance mode and don't expect to be adding major new features.   We expect
that over time users will mostly migrate to using pgAdmin4 Web as that
matures and stabilizes.

 
Introduction
------------

pgAdmin3 is a popular and feature rich Open Source administration and
development platform for PostgreSQL, the most advanced Open Source database in
the world. Binaries for the application are presently produced on Windows & OSX
to manage PostgreSQL 8.4 and above.

pgAdmin3 is designed to answer the needs of all users, from writing simple 
SQL queries to developing complex databases. The graphical interface supports 
all PostgreSQL features and makes administration easy. The application also 
includes a syntax highlighting SQL editor, a server-side code editor, an 
SQL/batch/shell job scheduling agent, support for the Slony-I replication 
engine and much more. Server connection may be made using TCP/IP or Unix Domain
Sockets (on *nix platforms), and may be SSL encrypted for security. No 
additional drivers are required to communicate with the database server.

pgAdmin3 is Free Software released under the PostgreSQL License.

Typical configure option
------------------------
```
$ bash bootstrap
$ ./configure --prefix=/opt/pgadmin3bigsql --with-pgsql=/opt/pgsql/12 --with-libgcrypt CFLAGS=-fPIC CXXFLAGS=-fPIC
$ make
$ sudo make install
```
