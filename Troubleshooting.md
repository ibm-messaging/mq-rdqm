# Troubleshooting

This page contains information to help you troubleshoot issues with RDQM HA or DR.

## HA

### Example Configuration

In order to give real examples of configurations and errors, the scenarios described are in the context of the following configuration.

The HA Group consists of the three nodes:

1. mqhavm13.hursley.ibm.com (referred to as vm13)
2. mqhavm14.hursley.ibm.com (vm14)
3. mqhavm15.hursley.ibm.com (vm15)

Three HA RDQMs have been created:

1. HAQM1 (created on vm13)
2. HAQM2 (created on vm14)
3. HAQM3 (created on vm15)

### Initial Conditions

The initial conditions on each of the nodes was as follows.

#### vm13

```text
[colgrave@mqhavm13 ~]$ rdqmstatus -m HAQM1
Node:                                   mqhavm13.hursley.ibm.com
Queue manager status:                   Running
CPU:                                    0.00%
Memory:                                 135MB
Queue manager file system:              51MB used, 1.0GB allocated [5%]
HA role:                                Primary
HA status:                              Normal
HA control:                             Enabled
HA current location:                    This node
HA preferred location:                  This node
HA floating IP interface:               None
HA floating IP address:                 None

Node:                                   mqhavm14.hursley.ibm.com
HA status:                              Normal

Node:                                   mqhavm15.hursley.ibm.com
HA status:                              Normal
Command '/opt/mqm/bin/rdqmstatus' run with sudo.

[colgrave@mqhavm13 ~]$ rdqmstatus -m HAQM2
Node:                                   mqhavm13.hursley.ibm.com
Queue manager status:                   Running elsewhere
HA role:                                Secondary
HA status:                              Normal
HA control:                             Enabled
HA current location:                    mqhavm14.hursley.ibm.com
HA preferred location:                  mqhavm14.hursley.ibm.com
HA floating IP interface:               None
HA floating IP address:                 None

Node:                                   mqhavm14.hursley.ibm.com
HA status:                              Normal

Node:                                   mqhavm15.hursley.ibm.com
HA status:                              Normal
Command '/opt/mqm/bin/rdqmstatus' run with sudo.

[colgrave@mqhavm13 ~]$ rdqmstatus -m HAQM3
Node:                                   mqhavm13.hursley.ibm.com
Queue manager status:                   Running elsewhere
HA role:                                Secondary
HA status:                              Normal
HA control:                             Enabled
HA current location:                    mqhavm15.hursley.ibm.com
HA preferred location:                  mqhavm15.hursley.ibm.com
HA floating IP interface:               None
HA floating IP address:                 None

Node:                                   mqhavm14.hursley.ibm.com
HA status:                              Normal

Node:                                   mqhavm15.hursley.ibm.com
HA status:                              Normal
Command '/opt/mqm/bin/rdqmstatus' run with sudo.
```

#### vm14

```text
[colgrave@mqhavm14 ~]$ rdqmstatus -m HAQM1
Node:                                   mqhavm14.hursley.ibm.com
Queue manager status:                   Running elsewhere
HA role:                                Secondary
HA status:                              Normal
HA control:                             Enabled
HA current location:                    mqhavm13.hursley.ibm.com
HA preferred location:                  mqhavm13.hursley.ibm.com
HA floating IP interface:               None
HA floating IP address:                 None

Node:                                   mqhavm13.hursley.ibm.com
HA status:                              Normal

Node:                                   mqhavm15.hursley.ibm.com
HA status:                              Normal
Command '/opt/mqm/bin/rdqmstatus' run with sudo.

[colgrave@mqhavm14 ~]$ rdqmstatus -m HAQM2
Node:                                   mqhavm14.hursley.ibm.com
Queue manager status:                   Running
CPU:                                    0.00%
Memory:                                 135MB
Queue manager file system:              51MB used, 1.0GB allocated [5%]
HA role:                                Primary
HA status:                              Normal
HA control:                             Enabled
HA current location:                    This node
HA preferred location:                  This node
HA floating IP interface:               None
HA floating IP address:                 None

Node:                                   mqhavm13.hursley.ibm.com
HA status:                              Normal

Node:                                   mqhavm15.hursley.ibm.com
HA status:                              Normal
Command '/opt/mqm/bin/rdqmstatus' run with sudo.

[colgrave@mqhavm14 ~]$ rdqmstatus -m HAQM3
Node:                                   mqhavm14.hursley.ibm.com
Queue manager status:                   Running elsewhere
HA role:                                Secondary
HA status:                              Normal
HA control:                             Enabled
HA current location:                    mqhavm15.hursley.ibm.com
HA preferred location:                  mqhavm15.hursley.ibm.com
HA floating IP interface:               None
HA floating IP address:                 None

Node:                                   mqhavm13.hursley.ibm.com
HA status:                              Normal

Node:                                   mqhavm15.hursley.ibm.com
HA status:                              Normal
Command '/opt/mqm/bin/rdqmstatus' run with sudo.
```

#### vm15

```text
[colgrave@mqhavm15 ~]$ rdqmstatus -m HAQM1
Node:                                   mqhavm15.hursley.ibm.com
Queue manager status:                   Running elsewhere
HA role:                                Secondary
HA status:                              Normal
HA control:                             Enabled
HA current location:                    mqhavm13.hursley.ibm.com
HA preferred location:                  mqhavm13.hursley.ibm.com
HA floating IP interface:               None
HA floating IP address:                 None

Node:                                   mqhavm13.hursley.ibm.com
HA status:                              Normal

Node:                                   mqhavm14.hursley.ibm.com
HA status:                              Normal
Command '/opt/mqm/bin/rdqmstatus' run with sudo.

[colgrave@mqhavm15 ~]$ rdqmstatus -m HAQM2
Node:                                   mqhavm15.hursley.ibm.com
Queue manager status:                   Running elsewhere
HA role:                                Secondary
HA status:                              Normal
HA control:                             Enabled
HA current location:                    mqhavm14.hursley.ibm.com
HA preferred location:                  mqhavm14.hursley.ibm.com
HA floating IP interface:               None
HA floating IP address:                 None

Node:                                   mqhavm13.hursley.ibm.com
HA status:                              Normal

Node:                                   mqhavm14.hursley.ibm.com
HA status:                              Normal
Command '/opt/mqm/bin/rdqmstatus' run with sudo.

[colgrave@mqhavm15 ~]$ rdqmstatus -m HAQM3
Node:                                   mqhavm15.hursley.ibm.com
Queue manager status:                   Running
CPU:                                    0.02%
Memory:                                 135MB
Queue manager file system:              51MB used, 1.0GB allocated [5%]
HA role:                                Primary
HA status:                              Normal
HA control:                             Enabled
HA current location:                    This node
HA preferred location:                  This node
HA floating IP interface:               None
HA floating IP address:                 None

Node:                                   mqhavm13.hursley.ibm.com
HA status:                              Normal

Node:                                   mqhavm14.hursley.ibm.com
HA status:                              Normal
Command '/opt/mqm/bin/rdqmstatus' run with sudo.
```

### DRBD Scenarios

The following scenarios focussing on DRBD are described:

1. Loss of quorum
2. Loss of a single DRBD peer
3. Stuck sync

#### DRBD Scenario 1: Loss of quorum

If the node running an HA RDQM loses the DRBD quorum for the DRBD resource corresponding to the RDQM, DRBD will immediately start returning errors from I/O operations, which will cause the RDQM to start producing FDCs and eventually stop.

If the remaining two nodes have a DRBD quorum for the DRBD resource then Pacemaker will choose one of the two nodes to start the RDQM, and because we know that there were no updates on the original node from the time where the quorum was lost, it is safe to start the RDQM somewhere else.

The two main ways you can monitor for a loss of DRBD quorum are:

1. use the rdqmstatus command
2. monitor the syslog of the node where the RDQM is originally running

There are some other DRBD tools that can be used but the focus here is on these two.

##### Use rdqmstatus

If the node vm13 where this HA RDQM is running loses DRBD quorum for the DRBD resource for HAQM1, you will see something like:

```text
[colgrave@mqhavm13 ~]$ rdqmstatus -m HAQM1
Node:                                   mqhavm13.hursley.ibm.com
Queue manager status:                   Running elsewhere
HA role:                                Secondary
HA status:                              <b>Remote unavailable</b>
HA control:                             Enabled
HA current location:                    mqhavm14.hursley.ibm.com
HA preferred location:                  This node
HA floating IP interface:               None
HA floating IP address:                 None

Node:                                   mqhavm14.hursley.ibm.com
HA status:                              Remote unavailable
HA out of sync data:                    0KB

Node:                                   mqhavm15.hursley.ibm.com
HA status:                              Remote unavailable
HA out of sync data:                    0KB
Command '/opt/mqm/bin/rdqmstatus' run with sudo.
```

The most important thing to notice is that the `HA status` has changed to `Remote unavailable` which indicates that both DRBD connections to the other nodes have been lost.

In this case the other two nodes have DRBD quorum for the DRBD resource so the RDQM is running somewhere else, on mqhavm14.hursley.ibm.com as shown as the value of `HA current location`.

##### Monitor syslog

DRBD logs a message when it loses quorum for a resource:

```text
Jul 30 09:38:36 mqhavm13 kernel: drbd haqm1/0 drbd100: quorum( yes -> no )
```

When quorum is restored a similar message is logged:

```text
Jul 30 10:27:32 mqhavm13 kernel: drbd haqm1/0 drbd100: quorum( no -> yes )
```

One configuration that can lead to this situation is if there is a device in the network between the RDQM nodes which will terminate idle connections.
As described in the Architecture page, DRBD sends a ping over the control/meta connection but does not do anything to keep the data connection alive,
so if a queue manager is idle for long enough, a network device may close the data connection.

If a device does close the data connection, I believe that the DRBD primary will also close the control/meta connection and begin the process of establishing the connections again.

On the primary you will probably see a message such as:

```text
sock was reset by peer
```

On the secondary you will probably see a message such as:

```text
meta connection shut down by peer
```

You may have to change the configuration of the Linux keepalive probes on the RDQM nodes to prevent this.

There are three kernel parameters relating to keepalive:

1. `net.ipv4.tcp_keepalive_intvl` which is the interval between keepalive probes, apart from the first probe
2. `net.ipv4.tcp_keepalive_probes` which is the number of unacknowledged probes to send before considering the connection dead
3. `net.ipv4.tcp_keepalive_time` which is the interval between the last data packet sent and the first keepalive probe being sent

On one of my systems which is using the default values, I see:

```bash
# sysctl -a | grep keepalive
net.ipv4.tcp_keepalive_intvl = 75
net.ipv4.tcp_keepalive_probes = 9
net.ipv4.tcp_keepalive_time = 7200
```

The value that you will probably have to change is `net.ipv4.tcp_keepalive_time` as the default value of 7200 means that the first keepalive probe will not be sent for two hours (7200 seconds) after the last data packet.

In a customer situation I was involved in, an external network device was killing the data connection after it had been idle for an hour, so before the first Linux keepalive probe was being sent.
Reducing the value of `net.ipv4.tcp_keepalive_time` down to 1800 (30 minutes) avoided the problem as the first Linux keepalive probe was sent well within an hour,
and then subsequent ones were sent every 75 seconds, so the data connection was never closed by the network device.

#### DRBD Scenario 2: Loss of a single DRBD peer

If only one of the two DRBD secondary nodes is lost then the HA RDQM will not move.

Starting from the same initial conditions as in the first scenario, after blocking just one of the DRBD peers, the status reported by rdqmstatus on vm13 was:

```text
Node:                                   mqhavm13.hursley.ibm.com
Queue manager status:                   Running
CPU:                                    0.01%
Memory:                                 133MB
Queue manager file system:              52MB used, 1.0GB allocated [5%]
HA role:                                Primary
HA status:                              <b>Mixed</b>
HA control:                             Enabled
HA current location:                    This node
HA preferred location:                  This node
HA floating IP interface:               None
HA floating IP address:                 None

Node:                                   mqhavm14.hursley.ibm.com
HA status:                              <b>Remote unavailable
HA out of sync data:                    0KB</b>

Node:                                   mqhavm15.hursley.ibm.com
HA status:                              Normal
Command '/opt/mqm/bin/rdqmstatus' run with sudo.
```

#### DRBD Scenario 3: Stuck sync

Some versions of DRBD had an issue where a sync would appear to be stuck and this will prevent an HA RDQM from failing over to a node when the sync to that node is still in progress.

One way to see this is to use `drbdadm status` which when things are normal produces output like:

```text
[colgrave@mqhavm13 ~]$ drbdadm status
haqm1 role:Primary
  disk:UpToDate
  mqhavm14.hursley.ibm.com role:Secondary
    peer-disk:UpToDate
  mqhavm15.hursley.ibm.com role:Secondary
    peer-disk:UpToDate

haqm2 role:Secondary
  disk:UpToDate
  mqhavm14.hursley.ibm.com role:Primary
    peer-disk:UpToDate
  mqhavm15.hursley.ibm.com role:Secondary
    peer-disk:UpToDate

haqm3 role:Secondary
  disk:UpToDate
  mqhavm14.hursley.ibm.com role:Secondary
    peer-disk:UpToDate
  mqhavm15.hursley.ibm.com role:Primary
    peer-disk:UpToDate
</pre>

If a sync gets stuck then you will see something like:
<pre>
[colgrave@mqhavm13 ~]$ drbdadm status
haqm1 role:Primary
  disk:UpToDate
  mqhavm14.hursley.ibm.com role:Secondary
    peer-disk:UpToDate
  mqhavm15.hursley.ibm.com role:Secondary
    <b>replication:SyncSource peer-disk:Inconsistent done:90.91</b>

haqm2 role:Secondary
  disk:UpToDate
  mqhavm14.hursley.ibm.com role:Primary
    peer-disk:UpToDate
  mqhavm15.hursley.ibm.com role:Secondary
    peer-disk:UpToDate

haqm3 role:Secondary
  disk:UpToDate
  mqhavm14.hursley.ibm.com role:Secondary
    peer-disk:UpToDate
  mqhavm15.hursley.ibm.com role:Primary
    peer-disk:UpToDate
```

In this case the HA RDQM HAQM1 cannot move to vm15 as the disk on vm15 is Inconsistent.

The done value is the percentage complete. If that value is not increasing you can try disconnecting that replica then connecting it again with the following commands, run as root on vm13:

```text
drbdadm disconnect haqm1:mqhavm15.hursley.ibm.com
drbdadm connect haqm1:mqhavm15.hursley.ibm.com
```

If the replication to both Secondary nodes is stuck you can do the disconnect and connect commands without specifying a node and that will disconnect both connections:

```text
drbdadm disconnect haqm1
drbdadm connect haqm1
```

### Pacemaker Scenarios

The following scenarios focussing on Pacemaker are described:

1. Corosync main process was not scheduled
2. An HA RDQM is not running where it should be

#### Pacemaker Scenario 1: Corosync main process was not scheduled

If you see a message in the syslog similar to:

```text
corosync[10800]:  [MAIN  ] Corosync main process was not scheduled for 2787.0891 ms (threshold is 1320.0000 ms). Consider token timeout increase.
```

that is an indication that the system is either too busy to schedule CPU time to the main Corosync process or, more commonly, that the system is a Virtual Machine and the Hypervisor has not scheduled any CPU time to the entire VM.

Both Pacemaker/Corosync and DRBD have timers that are used to detect loss of quorum so messages like this indicate that the node did not run for so long that it would have been dropped from the quorum.

The Corosync timeout is 1.65 seconds and the threshold of 1.32 seconds is 80% of that so the message is printed when the delay in the scheduling of the main Corosync process hits 80% of the timeout. In this case, the process was not scheduled for nearly three seconds which would definitely cause problems.

Whatever is causing this problem needs to be resolved. One thing that might help is to reduce the requirements of the VM, for example reducing the number of vCPUs required, as this makes it easier for the Hypervisor to schedule the VM.

#### Pacemaker Scenario 2: An HA RDQM is not running where it should be

The main tool to help troubleshooting in this scenario is `crm status` and the output for this configuration when everything is working as expected is:

```text
Stack: corosync
Current DC: mqhavm13.hursley.ibm.com (version 1.1.20.linbit-1+20190404+eab6a2092b71.el7.2-eab6a2092b) - partition with quorum
Last updated: Tue Jul 30 09:11:29 2019
Last change: Tue Jul 30 09:10:34 2019 by root via crm_attribute on mqhavm14.hursley.ibm.com

3 nodes configured
18 resources configured

<b>Online: [ mqhavm13.hursley.ibm.com mqhavm14.hursley.ibm.com mqhavm15.hursley.ibm.com ]</b>

Full list of resources:

 Master/Slave Set: ms_drbd_haqm1 [p_drbd_haqm1]
     Masters: [ mqhavm13.hursley.ibm.com ]
     Slaves: [ mqhavm14.hursley.ibm.com mqhavm15.hursley.ibm.com ]
 p_fs_haqm1 (ocf::heartbeat:Filesystem): Started mqhavm13.hursley.ibm.com
 p_rdqmx_haqm1 (ocf::ibm:rdqmx): Started mqhavm13.hursley.ibm.com
 <b>haqm1 (ocf::ibm:rdqm): Started mqhavm13.hursley.ibm.com</b>
 Master/Slave Set: ms_drbd_haqm2 [p_drbd_haqm2]
     Masters: [ mqhavm14.hursley.ibm.com ]
     Slaves: [ mqhavm13.hursley.ibm.com mqhavm15.hursley.ibm.com ]
 p_fs_haqm2 (ocf::heartbeat:Filesystem): Started mqhavm14.hursley.ibm.com
 p_rdqmx_haqm2 (ocf::ibm:rdqmx): Started mqhavm14.hursley.ibm.com
 <b>haqm2 (ocf::ibm:rdqm): Started mqhavm14.hursley.ibm.com</b>
 Master/Slave Set: ms_drbd_haqm3 [p_drbd_haqm3]
     Masters: [ mqhavm15.hursley.ibm.com ]
     Slaves: [ mqhavm13.hursley.ibm.com mqhavm14.hursley.ibm.com ]
 p_fs_haqm3 (ocf::heartbeat:Filesystem): Started mqhavm15.hursley.ibm.com
 p_rdqmx_haqm3 (ocf::ibm:rdqmx): Started mqhavm15.hursley.ibm.com
 <b>haqm3 (ocf::ibm:rdqm): Started mqhavm15.hursley.ibm.com</b>
```

The main things to notice are:

1. all three nodes are Online
2. each HA RDQM is running on the node where it was created: HAQM1 on vm13 etc.

To demonstrate this scenario I am going to deliberately prevent HAQM1 from running on vm14 and then attempt to move HAQM1 to vm14.

To prevent it from running I am going to edit the file /var/mqm/mqs.ini on vm14 and change the Directory of the queue manager HAQM1 to an incorrect value.

I then changed the preferred location for HAQM1 to vm14 by running the following command on vm13:

```text
rdqmadm -m HAQM1 -n mqhavm14.hursley.ibm.com -p
```

This would normally cause HAQM1 to move to vm14 but in this case, when I check the status on vm13 I see:

```text
[colgrave@mqhavm13 ~]$ rdqmstatus -m HAQM1
Node:                                   mqhavm13.hursley.ibm.com
Queue manager status:                   Running
CPU:                                    0.15%
Memory:                                 133MB
Queue manager file system:              52MB used, 1.0GB allocated [5%]
HA role:                                Primary
HA status:                              Normal
HA control:                             Enabled
HA current location:                    This node
HA preferred location:                  mqhavm14.hursley.ibm.com
HA floating IP interface:               None
HA floating IP address:                 None

Node:                                   mqhavm14.hursley.ibm.com
HA status:                              Normal

Node:                                   mqhavm15.hursley.ibm.com
HA status:                              Normal
Command '/opt/mqm/bin/rdqmstatus' run with sudo.
```

HAQM1 is still running on vm13. Why is that?

Looking at the Pacemaker status I see:

```text
[colgrave@mqhavm13 ~]$ crm status
Stack: corosync
Current DC: mqhavm13.hursley.ibm.com (version 1.1.20.linbit-1+20190404+eab6a2092b71.el7.2-eab6a2092b) - partition with quorum
Last updated: Thu Aug  1 14:16:40 2019
Last change: Thu Aug  1 14:16:35 2019 by hacluster via crmd on mqhavm14.hursley.ibm.com

3 nodes configured
18 resources configured

Online: [ mqhavm13.hursley.ibm.com mqhavm14.hursley.ibm.com mqhavm15.hursley.ibm.com ]

Full list of resources:

 Master/Slave Set: ms_drbd_haqm1 [p_drbd_haqm1]
     Masters: [ mqhavm13.hursley.ibm.com ]
     Slaves: [ mqhavm14.hursley.ibm.com mqhavm15.hursley.ibm.com ]
 p_fs_haqm1 (ocf::heartbeat:Filesystem): Started mqhavm13.hursley.ibm.com
 p_rdqmx_haqm1 (ocf::ibm:rdqmx): Started mqhavm13.hursley.ibm.com
 haqm1 (ocf::ibm:rdqm): Started mqhavm13.hursley.ibm.com
 Master/Slave Set: ms_drbd_haqm2 [p_drbd_haqm2]
     Masters: [ mqhavm14.hursley.ibm.com ]
     Slaves: [ mqhavm13.hursley.ibm.com mqhavm15.hursley.ibm.com ]
 p_fs_haqm2 (ocf::heartbeat:Filesystem): Started mqhavm14.hursley.ibm.com
 p_rdqmx_haqm2 (ocf::ibm:rdqmx): Started mqhavm14.hursley.ibm.com
 haqm2 (ocf::ibm:rdqm): Started mqhavm14.hursley.ibm.com
 Master/Slave Set: ms_drbd_haqm3 [p_drbd_haqm3]
     Masters: [ mqhavm15.hursley.ibm.com ]
     Slaves: [ mqhavm13.hursley.ibm.com mqhavm14.hursley.ibm.com ]
 p_fs_haqm3 (ocf::heartbeat:Filesystem): Started mqhavm15.hursley.ibm.com
 p_rdqmx_haqm3 (ocf::ibm:rdqmx): Started mqhavm15.hursley.ibm.com
 haqm3 (ocf::ibm:rdqm): Started mqhavm15.hursley.ibm.com

<b>Failed Resource Actions:
* haqm1_monitor_0 on mqhavm14.hursley.ibm.com 'not installed' (5): call=372, status=complete, exitreason='',
    last-rc-change='Thu Aug  1 14:16:37 2019', queued=0ms, exec=17ms</b>
```

The key thing here is the `Failed Resource Actions` section that has appeared.

The name of the action, `haqm1_monitor_0` tells us that it was a monitor action for the RDQM HAQM1 that failed, and it failed on mqhavm14.hursley.ibm.com, so it looks like Pacemaker tried to do what we expected and start HAQM1 on vm14, but for some reason it couldn't.

We can see when Pacemaker tried do this from the value of `last-rc-change`

#### Understanding the failure

To understand the failure we need to look at the syslog for vm14 at the time of the failure.

Doing so reveals:

```text
Aug  1 14:16:37 mqhavm14 crmd[26377]:  notice: Result of probe operation for haqm1 on mqhavm14.hursley.ibm.com: 5 (not installed)
```

This shows that when Pacemaker tried to check the state of haqm1 on vm14 it got an error because haqm1 is not configured, because of the change I made to /var/mqm/mqs.ini.

#### Correcting the failure

To correct the failure it is necessary to correct the underlying problem, in this case restoring /var/mqm/mqs.ini on vm14.

Once that is done it is necessary to clear the failed action, which is done using the command `crm resource cleanup` on the appropriate resource, which in this case is the resource haqm1 as that is the resource mentioned in the failed action.

On vm13 I ran:

```bash
[colgrave@mqhavm13 ~]$ crm resource cleanup haqm1
Cleaned up haqm1 on mqhavm15.hursley.ibm.com
Cleaned up haqm1 on mqhavm14.hursley.ibm.com
Cleaned up haqm1 on mqhavm13.hursley.ibm.com
```

After that I checked the Pacemaker status again:

```bash
[colgrave@mqhavm13 ~]$ crm status
Stack: corosync
Current DC: mqhavm13.hursley.ibm.com (version 1.1.20.linbit-1+20190404+eab6a2092b71.el7.2-eab6a2092b) - partition with quorum
Last updated: Thu Aug  1 14:23:17 2019
Last change: Thu Aug  1 14:23:03 2019 by hacluster via crmd on mqhavm13.hursley.ibm.com

3 nodes configured
18 resources configured

Online: [ mqhavm13.hursley.ibm.com mqhavm14.hursley.ibm.com mqhavm15.hursley.ibm.com ]

Full list of resources:

 Master/Slave Set: ms_drbd_haqm1 [p_drbd_haqm1]
     Masters: [ mqhavm14.hursley.ibm.com ]
     Slaves: [ mqhavm13.hursley.ibm.com mqhavm15.hursley.ibm.com ]
 p_fs_haqm1 (ocf::heartbeat:Filesystem): Started mqhavm14.hursley.ibm.com
 p_rdqmx_haqm1 (ocf::ibm:rdqmx): Started mqhavm14.hursley.ibm.com
 haqm1 (ocf::ibm:rdqm): Started mqhavm14.hursley.ibm.com
 Master/Slave Set: ms_drbd_haqm2 [p_drbd_haqm2]
     Masters: [ mqhavm14.hursley.ibm.com ]
     Slaves: [ mqhavm13.hursley.ibm.com mqhavm15.hursley.ibm.com ]
 p_fs_haqm2 (ocf::heartbeat:Filesystem): Started mqhavm14.hursley.ibm.com
 p_rdqmx_haqm2 (ocf::ibm:rdqmx): Started mqhavm14.hursley.ibm.com
 haqm2 (ocf::ibm:rdqm): Started mqhavm14.hursley.ibm.com
 Master/Slave Set: ms_drbd_haqm3 [p_drbd_haqm3]
     Masters: [ mqhavm15.hursley.ibm.com ]
     Slaves: [ mqhavm13.hursley.ibm.com mqhavm14.hursley.ibm.com ]
 p_fs_haqm3 (ocf::heartbeat:Filesystem): Started mqhavm15.hursley.ibm.com
 p_rdqmx_haqm3 (ocf::ibm:rdqmx): Started mqhavm15.hursley.ibm.com
 haqm3 (ocf::ibm:rdqm): Started mqhavm15.hursley.ibm.com
```

The failed action has disappeared and HAQM1 is now running on vm14 as expected.

The RDQM status is:

```bash
[colgrave@mqhavm13 ~]$ rdqmstatus -m HAQM1
Node:                                   mqhavm13.hursley.ibm.com
Queue manager status:                   Running elsewhere
HA role:                                Secondary
HA status:                              Normal
HA control:                             Enabled
HA current location:                    mqhavm14.hursley.ibm.com
HA preferred location:                  mqhavm14.hursley.ibm.com
HA floating IP interface:               None
HA floating IP address:                 None

Node:                                   mqhavm14.hursley.ibm.com
HA status:                              Normal

Node:                                   mqhavm15.hursley.ibm.com
HA status:                              Normal
Command '/opt/mqm/bin/rdqmstatus' run with sudo.
```

## DR

RDQM DR is simpler than RDQM HA as Pacemaker is not involved.
