.. _main:


****************************
`pgAdmin Main Window`:index:
****************************

.. image:: images/main.png

In the main window, the structure of the databases is displayed.
You can create new objects, delete and edit existing objects if the
privileges of the user that you are using on the current connection
to the database allow this.

The left side of the main window shows a tree with all servers,
and the objects they contain.

The upper right side shows details of the object currently
selected in the tree. Some objects might have statistics in addition
to their properties, these can be shown if you select the Statistics
tab.

The lower right side contains a reverse engineered SQL script. You
can copy this to any editor using cut & paste, or save it to a file
using Save definition... from the File menu, or use it as a template
if you select the :ref:`Query Tool <query>`. If the
:ref:`Copy SQL from main window to query tool <options-query_tool>` option is selected, the SQL
query will be copied automatically to the tool.

The status line will show you some status information, as well as
the time the last action took pgAdmin III to complete.

You can resize the main window, and change the sizes of the three
main regions as you prefer. These adjustments will be preserved when
you exit the program.

pgAdmin is bandwidth friendly. The status of objects in the browser 
is only refreshed on request or after changes made with the built-in tools. 
Be aware that this does not cover changes made via manual SQL or from other 
users or other clients. It is generally advisable to manually refresh 
objects before working on them in such environments.

.. _getting-started:

Getting started
===============

After you have added the desired server(s) to the tree on the left
side using the :ref:`Add server <connect>` menu or toolbar
button, each server will show up under the top node "Servers".

To open a connection to a server, select the desired server in the
tree, and double click on it or use Connect from the Tools menu. The
connection will be established, and the properties of the top level
objects are retrieved from the database server. If you've been
connected to that database previously, pgAdmin III will restore the
previous selection of database and schema for you. The current
situation is saved when exiting the program, so that pgAdmin III is
able to restore the previous environment.

Using the menu or toolbar buttons, you can create new objects,
delete objects and edit properties of existing objects if the user
that you entered when adding the server connection has sufficient
privileges. You may find some options grayed out if displaying
properties. This means, that the database server you're currently
using doesn't support the feature, or that this property can't be
changed by design, or that your user privileges won't allow you to
change it.

You can search for objects in the database using the :ref:`Search Tool <search_object>`

Contents:

.. toctree::
   :maxdepth: 2

   search_object