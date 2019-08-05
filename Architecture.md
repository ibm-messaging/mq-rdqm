This page contains a high-level description of the architecture of RDQM, primarily to aid in troubleshooting. Both RDQM HA and RDQM DR are covered.

# Resource Names

Various resources are created for each RDQM and these resources have names based on the Directory name of the queue manager, which can be found in /var/mqm/mqs.ini and which is referred to here as \<qm>

For example, for an HA RDQM named TMPQM1, \<qm> would be tmpqm1

# HA

The architecture of RDQM HA involves both DRBD, for data replication, and Pacemaker, for managing where HA RDQMs run.

When you create an HA RDQM the following happens:
1. a DRBD resource is created to replicate the data for the queue manager
2. a queue manager is created and configured to use the DRBD resource for its storage
3. a set of Pacemaker resources is created to monitor and manage the queue manager

## DRBD

Each HA RDQM has a DRBD resource generated for it: /etc/drbd.d/\<qm>.res. For example, when I created an HA RDQM named HAQM1 the DRBD resource file was /etc/drbd.d/haqm1.res.

The most important information in the .res file is the device minor number for this particular DRBD resource as a lot of the messages that DRBD logs are in terms of this minor number. In my case I see:
```
device minor 100;
```
so I would look for DRBD messages like:
```
Jul 31 00:17:24 mqhavm13 kernel: drbd haqm1/0 drbd100 mqhavm15.hursley.ibm.com: drbd_sync_handshake:
```

The fact that the message mentions drbd100 means that the message relates to HAQM1.

Not all messages logged by DRBD use the device minor number, some use the DRBD resource name, which is the same as the Directory name of the HA RDQM. For example, I see messages like:
```
Jul 31 00:17:22 mqhavm13 kernel: drbd haqm1 mqhavm15.hursley.ibm.com: Connection closed
```

## Pacemaker

There are a number of Pacemaker resources generated for an HA RDQM:

1. \<qm> - this is the main resource representing the HA RDQM
2. p_rdqmx_\<qm> - this is an internal resource
3. p_fs_\<qm> - this is a standard Filesystem resource that mounts the volume for the queue manager on /var/mqm/vols/\<qm>
4. ms_drbd_\<qm> - this is the master/slave resource for the DRBD resource for the RDQM
5. p_drbd_\<qm> - this is the primitive resource for the DRBD resource for the RDQM

If a floating IP address is configured for an HA RDQM then an additional resource is configured:

6. p_ip_\<qm>

# DR

The architecture of RDQM DR is simpler as Pacemaker is not involved, only DRBD.