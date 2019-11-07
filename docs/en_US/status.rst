.. _status:


*******************************
`Database Server Status`:index:
*******************************

.. image:: images/status.png

The Server Status dialogue displays the current connections to each
database, the user that is connected, the process ID, the client address 
and start time (on PostgreSQL 8.1 and above), the current
query being executed (if any), and the query start time (where appropriate) 
on PostgreSQL 7.4 and above on the Status tab.

The Cancel button allows cancelling the query running on a specific
backend. Terminate will shutdown the backend. **Attention:** Both functions should be
used carefully, as the interrupt the client's work
ungracefully. Particularly, the terminate function might disturb the
function of the complete server, force it to restart its services and
thus interrupt all user connections. You should use this function only
if the server is seriously injured by a backend you can't control otherwise.

The Locks tab shows the current locks outstanding in the PostgreSQL Lock
Manager. This information can be useful when attempting to debug or track
down deadlocks on your server. Not all information is necessarily shown
for each lock. In particular, the Relation name may be shown as an OID
instead of by it's name, if the relation is in a different database to 
that being monitored.

**Note:** When the pg_locks view is accessed as is the case whenever 
this dialgue is open, PostgreSQL's internal lock manager data structures are 
momentarily locked, and a copy is made for the dialogue to display. This 
ensures that the dialogue displays a consistent set of results, while not 
blocking normal lock manager operations longer than necessary. Nonetheless 
there could be some impact on database performance if this view is read often. 

On a PostgreSQL server running version 8.1 or newer, the Transaction tab allows
you to view outstanding prepared transactions. Prepared transactions are an aspect
of Two Phase Commit (2PC), used in distributed transaction managers. Usually,
prepared transactions are handled by the transaction manager. In case of a failure,
it might be necessary to commit or rollback a transaction manually; you can use
the 'Commit' or 'Rollback' buttons to do this.

The Logfile tab shows server log files, if configured in
postgresql.conf (redirect_stderr or logging_collector = true, 
log_destination = 'stderr' and log_filename = 'postgresql-%Y-%m-%d_%H%M%S.log'
on PostgreSQL or 'enterprisedb-%Y-%m-%d_%H%M%S.log' on EnterpriseDB's Advanced 
Server). 
pgAdmin will extract a time stamp from the logfile in a separate column, if the
log_line_prefix is configured accordingly. We recommend using '%t:' as
format, because more complicate formats might not be interpretable correctly.

The combobox allows you to select historic logfiles or the current
one. If "current" is selected, pgAdmin will correctly detect
logfile rotation and continue to display them.

The "Rotate" button will force the server to rotate its server
logfile. This function is currently not implemented on 8.0 servers; if
you think this is valuable for you please contact us.

Please note that displaying the logfile requires :ref:`additional functions
<extend>` loaded on the server side, which are
available for 8.x servers only.

To refresh the display click the *Refresh* button. The display will
also be automatically refreshed based on the refresh interval specified.
Note that you have one refresh rate per tab.

You can hide panes by clicking on their close button or by clicking on
the appropiate menu item in the View menu.

You can also copy some lines on the tabs' list. Select the lines you want to
copy and click on the Copy button of the toolbar.
