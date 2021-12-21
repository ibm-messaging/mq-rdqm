# RDQM Architecture

This page contains a high-level description of the architecture of RDQM, primarily to aid in troubleshooting. Both RDQM HA and RDQM DR are covered.

## Resource Names

Various resources are created for each RDQM queue manager, and these resources have names based on the Directory name of the queue manager, which can be found in /var/mqm/mqs.ini and which is referred to here as \<qm>

For example, for an RDQM HA queue manager named TMPQM1, \<qm> would be tmpqm1

## HA

The architecture of RDQM HA involves both DRBD, for data replication, and Pacemaker, for managing where RDQM HA queue managers run.

When you create an RDQM HA queue manager, the following happens:

1. a DRBD resource is created to replicate the data for the queue manager
2. a queue manager is created and configured to use the DRBD resource for its storage
3. a set of Pacemaker resources is created to monitor and manage the queue manager

### DRBD Resource

Each RDQM HA queue manager has a DRBD resource generated for it, defined in the file: /etc/drbd.d/\<qm>.res. For example, when I created an RDQM HA queue manager named HAQM1, the DRBD resource file was /etc/drbd.d/haqm1.res.

The most important information in the .res file is the device minor number for this particular DRBD resource as a lot of the messages that DRBD logs are in terms of this minor number. In my case I see:

```text
device minor 100;
```

so I would look for DRBD messages like:

```text
Jul 31 00:17:24 mqhavm13 kernel: drbd haqm1/0 drbd100 mqhavm15.hursley.ibm.com: drbd_sync_handshake:
```

The fact that the message mentions drbd100 means that the message relates to HAQM1.

Not all messages logged by DRBD use the device minor number, some use the DRBD resource name, which is the same as the Directory name of the HA RDQM. For example, I see messages like:

```text
Jul 31 00:17:22 mqhavm13 kernel: drbd haqm1 mqhavm15.hursley.ibm.com: Connection closed
```

### DRBD HA Connections

In normal operation there will be a pair of DRBD connections between each pair of nodes:

1. a control or meta Connection
2. the main data Connection

The usual pattern is that one of these connections is established by one node of a pair and the other connection is established by the other node in the pair.

For RDQM HA, the DRBD listener port for the queue manager is chosen automatically when the queue manager is created. The port numbers begin at 7000. You can find the port for a queue manager by looking for the `address` lines in the appropriate DRBD .res file,
for example:

```text
address ipv4 10.51.7.58:7000;
```

Once you know the appropriate port number, you can find the TCP connections relating to that queue manager with a command such as:

```bash
ss -nt '( src :7000 or dst :7000 )'
```

As an example, I have an RDQM HA deployment consisting of the nodes:

| Node | IP Address |
| ---- | ---------- |
| node-1 | 10.51.6.190 |
| node-2 | 10.51.6.210 |
| node-3 | 10.51.7.19 |

I have a single RDQM HA queue manager configured and running, with all DRBD connections connected.

When I run the `ss` command on node-3 I get the output:

```bash
State        Recv-Q         Send-Q                 Local Address:Port                  Peer Address:Port         Process
ESTAB        0              0                         10.51.7.19:36111                  10.51.6.210:7000
ESTAB        0              0                         10.51.7.19:7000                   10.51.6.210:50901
ESTAB        0              0                         10.51.7.19:7000                   10.51.6.190:42379
ESTAB        0              0                         10.51.7.19:54205                  10.51.6.190:7000
```

The first connection listed was established by node-3 to node-2.

The second connection listed was established by node-2 to node-3. As mentioned above, the two DRBD connections between a pair of nodes are usually established in opposite directions.

The third connection listed was established by node-1 to node-3.

The fourth connection listed was established by node-3 to node-1.

The easiest way to distinguish the control connection from the data connection for an active queue manager is by the amount of data flowed over the connection,
which will usually be smaller for the control Connection.

To see the number of bytes flowed over each connection I use the `ss` command with the `i` option. As an example, in my current environment I ran `ss -nit '( src :7000 or dst :7000 )'` which produced:

```bash
State           Recv-Q           Send-Q                     Local Address:Port                      Peer Address:Port           Process
ESTAB           0                0                             10.51.7.19:36111                      10.51.6.210:7000
     cubic wscale:7,7 rto:201 rtt:0.209/0.085 ato:40 mss:1448 pmtu:1500 rcvmss:1448 advmss:1448 cwnd:10 ssthresh:27 bytes_sent:2587329 bytes_retrans:20 bytes_acked:2587310 bytes_received:3704817 segs_out:2980 segs_in:1566 data_segs_out:2046 data_segs_in:932 send 554258373bps lastsnd:229936217 lastrcv:3102096 lastack:3102101 pacing_rate 1106531336bps delivery_rate 1672556144bps delivered:2047 busy:131ms retrans:0/1 dsack_dups:1 rcv_rtt:7283.92 rcv_space:277568 rcv_ssthresh:2662500 minrtt:0.128
ESTAB           0                0                             10.51.7.19:7000                       10.51.6.210:50901
     cubic wscale:7,7 rto:201 rtt:0.3/0.027 ato:40 mss:1448 pmtu:1500 rcvmss:536 advmss:1448 cwnd:10 bytes_sent:401848 bytes_acked:401848 bytes_received:388128 segs_out:23848 segs_in:46420 data_segs_out:23343 data_segs_in:23086 send 386133333bps lastsnd:9624 lastrcv:9624 lastack:9624 pacing_rate 770340808bps delivery_rate 176406088bps delivered:23344 app_limited busy:6597ms rcv_rtt:68387.1 rcv_space:28976 rcv_ssthresh:64336 minrtt:0.097
ESTAB           0                0                             10.51.7.19:7000                       10.51.6.190:42379
     cubic wscale:7,7 rto:201 rtt:0.538/0.059 ato:40 mss:1448 pmtu:1500 rcvmss:536 advmss:1448 cwnd:10 bytes_sent:885176 bytes_acked:885176 bytes_received:415992 segs_out:53022 segs_in:27238 data_segs_out:29765 data_segs_in:23599 send 215315985bps lastsnd:8025 lastrcv:8025 lastack:8025 pacing_rate 430531936bps delivery_rate 132237440bps delivered:29766 app_limited busy:7935ms rcv_rtt:299060 rcv_space:28960 rcv_ssthresh:68624 minrtt:0.116
ESTAB           0                0                             10.51.7.19:54205                      10.51.6.190:7000
     cubic wscale:7,7 rto:201 rtt:0.245/0.079 ato:40 mss:1448 pmtu:1500 rcvmss:1448 advmss:1448 cwnd:10 ssthresh:28 bytes_sent:2947194 bytes_retrans:20 bytes_acked:2947175 bytes_received:3170253590 segs_out:67287 segs_in:78314 data_segs_out:3556 data_segs_in:76191 send 472816327bps lastsnd:229936217 lastrcv:229936217 lastack:2573657 pacing_rate 942267408bps delivery_rate 875365232bps delivered:3557 app_limited busy:1187ms retrans:0/1 dsack_dups:1 rcv_rtt:17.234 rcv_space:3043672 rcv_ssthresh:3145728 minrtt:0.078
```

There are a number of values in the second line of output for each connection that you can use to decide which is the control connection and which is the data connection:

1. `bytes_sent` although this is only shown on RHEL 8 systems
2. `bytes_acked`
3. `bytes_received`

If a queue manager is idle then it is possible that the connection with the greater amount of data flowing is actually the control connection, because of the DRBD ping that is sent every 10 seconds and the response to it.
If you have an idle queue manager then the most reliable way to identify the control connection is to run tcpdump on one of the connections using a command such as `tcpdump src port 36111`.
If the connection with the specified local port is the control connection then you should see some activity every 10 seconds.

If there is no output from tcpdump then the connection is almost certainly the data connection.
You can verify that by copying a file into the userdata directory of the queue manager, which will cause the data to be sent over the data connection.

### DRBD Keepalive

DRBD has its own keepalive implementation that is used on the control connection. The data connection relies on the standard Linux keepalive feature.

RDQM uses the default DRBD configuration of:

```bash
ping-int     10; # seconds, default
ping-timeout  5; # 1/10 seconds, default
```

This configuration means that DRBD sends a ping request every 10 seconds and expects a response within half a second. If a response is not received within that time, you will see a message in the syslog saying `PingAck did not arrive in time.`.

### Pacemaker

There are a number of Pacemaker resources generated for an HA RDQM:

* \<qm> - this is the main resource representing the HA RDQM
* p_rdqmx_\<qm> - this is an internal resource
* p_fs_\<qm> - this is a standard Filesystem resource that mounts the volume for the queue manager on /var/mqm/vols/\<qm>
* ms_drbd_\<qm> - this is the master/slave resource for the DRBD resource for the RDQM
* p_drbd_\<qm> - this is the primitive resource for the DRBD resource for the RDQM

If a floating IP address is configured for an HA RDQM then an additional resource is configured:

* p_ip_\<qm>

## DR

The architecture of RDQM DR is simpler as Pacemaker is not involved, only DRBD.

The architecture of RDQM DR/HA is a combination of the architecture for DR and the architecture for HA.

### DRBD DR Connections

The DRBD DR connections for a DR/HA queue manager are between the node where the queue manager is running and the node that is the HA preferred location in the RDQM HA Group that is currently the DR Secondary.
There is only one pair of connections between the HA Groups for each queue manager. The node in the DR Secondary HA Group is responsible for synchronously replicating the updates it receives to the other two nodes in the HA Group.

The DR replication is described as asynchronous but it is not fully asynchronous. We map asynchronous replication to the DRBD Protocol A and the way that DRBD implements that is that an update is regarded as having completed as soon as the data has been copied to the local TCP sendbuffer of the connection, assuming the connection is established. If the connection is not established, the update is remembered and the update completes immediately, or in the case of DR/HA, as soon as the update is complete on the other HA nodes.

As an example, I set up a DR/HA configuration with two HA Groups, each having different IP addresses for DR and HA.

The first HA Group was made up of the following nodes:

| Node | HA IP Address | DR IP Address |
| ---- | ---------- | ------------- |
| node-a1 | 10.51.20.185 | 192.168.198.50 |
| node-a2 | 10.51.20.192 | 192.168.198.66 |
| node-a3 | 10.51.20.203 | 192.168.199.236 |

The second HA Group was made up of the following nodes:

| Node | HA IP Address | DR IP Address |
| ---- | ---------- | ------------- |
| node-b1 | 10.51.7.58 | 192.168.208.25 |
| node-b2 | 10.51.7.190 | 192.168.208.26 |
| node-b3 | 10.51.15.65 | 192.168.208.28 |

The queue manager QM1 was running on node-a1 and when I ran `rdqmstatus -m QM1` it produced:

```bash
Node:
node-a1
Queue manager status:                   Running
CPU:                                    0.00%
Memory:                                 181MB
Queue manager file system:              58MB used, 2.9GB allocated [2%]
HA role:                                Primary
HA status:                              Normal
HA control:                             Enabled
HA current location:                    This node
HA preferred location:                  This node
HA blocked location:                    None
HA floating IP interface:               None
HA floating IP address:                 None
DR role:                                Primary
DR status:                              Normal
DR port:                                8001
DR local IP address:                    192.168.198.50
DR remote IP address list:              192.168.208.25,192.168.208.26,192.168.208.28
DR current remote IP address:           192.168.208.25

Node:
node-a2
HA status:                              Normal

Node:
node-a3
HA status:                              Normal
```

The value of `DR current remote IP address` is the DR IP address of the node where the DR Secondary instance is currently receiving the data from the DR Primary, in this case node-b1.

Running `rdqmstatus -m QM1` on that node produced:

```bash
Node:
node-b1
Queue manager status:                   Ended immediately
HA role:                                Primary
HA status:                              Normal
HA control:                             Enabled
HA current location:                    This node
HA preferred location:                  This node
HA blocked location:                    None
HA floating IP interface:               None
HA floating IP address:                 None
DR role:                                Secondary
DR status:                              Normal
DR port:                                8001
DR local IP address:                    192.168.208.25
DR remote IP address list:              192.168.198.50,192.168.198.66,192.168.199.236
DR current remote IP address:           192.168.198.50

Node:
node-b2
HA status:                              Normal

Node:
node-b3
HA status:                              Normal
```

The value of `DR port` is the port number used for the DR replication of this particular queue manager, as specified on the original `crtmqm` command.

I looked at the connections involving this port on node-a1:

```bash
ss -nt '( src :8001 or dst :8001 )'
State      Recv-Q Send-Q                   Local Address:Port                                  Peer Address:Port
ESTAB      0      0                          192.168.198.50:8001                                192.168.208.25:57373
ESTAB      0      0                          192.168.198.50:59630                               192.168.208.25:8001
```

As with the HA connections, the DR connections will normally be established in opposite directions. Here, the first connection was established by node-b1 to node-a1 and the second connection was established by node-a1 to node-b1.

As with the HA connections, one of these is the control or meta connection and the other is the data connection. Including the information about each connection to see the bytes received:

```bash
ss -int '( src :8001 or dst :8001 )'
State      Recv-Q Send-Q                   Local Address:Port                                  Peer Address:Port
ESTAB      0      0                          9.20.198.50:8001                                   9.20.208.25:57373
     cubic wscale:7,7 rto:202 rtt:1.648/2.188 ato:40 mss:1448 rcvmss:592 advmss:1448 cwnd:10 bytes_acked:733 bytes_received:1181 segs_out:14 segs_in:16 send 70.3Mbps lastsnd:4841379 lastrcv:4841380 lastack:4841373 pacing_rate 140.6Mbps rcv_rtt:6 rcv_space:28960
ESTAB      0      0                          9.20.198.50:59630                                  9.20.208.25:8001
     cubic wscale:7,7 rto:201 rtt:0.61/0.044 ato:40 mss:1448 rcvmss:536 advmss:1448 cwnd:10 bytes_acked:7785 bytes_received:7728 segs_out:970 segs_in:487 send 189.9Mbps lastsnd:1397 lastrcv:1397 lastack:1397 pacing_rate 379.4Mbps rcv_space:29200
```

The first connection has a `bytes_received` value of 1181 and the second connection has a `bytes_received` value of 7728 which suggests that the first connection is the control connection and the second connection is the data connection,
although these values are similar enough that it might be the case that the connection with the higher value is actually the control connection that has overtaken a recently-idle data connection due to the DRBD ping traffic.
