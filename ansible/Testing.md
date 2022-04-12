# Testing an ansible deployment of RDQM

The extra features related to testing that are provided by the ansible playbooks are:

1. an MQSC file on the first RDQM node to configure a queue manager ready to be used with the sample programs

2. a pair of shell scripts on each client node to run the rdqmput and rdqmget samples with the correct IP addresses and port

## Configure the queue manager

Go to the first RDQM node listed in the hosts.ini file and become the rdqmadmin user.

In the home directory of the rdqmadmin user you should see a file `config.mqsc`

Check the queue manager QM1 you created is running on that node with either `dspmq` or `rdqmstatus -m QM1`

If it is not running on the first node then either copy the config.mqsc file to the node where it is running and log in to that node as rdqmadmin, or move the queue manager to the first node using the `rdqmadm` command, as shown in the next section.

Once the config.mqsc file is on the node where the queue manager is running, run:
```
runmqsc QM1 < config.mqsc
```
which should produce something like:
```
5724-H72 (C) Copyright IBM Corp. 1994, 2021.
Starting MQSC for queue manager QM1.


     1 : define ql(queue1) maxdepth(50000)
AMQ8006I: IBM MQ queue created.
     2 : define channel(sample.svrconn) chltype(svrconn) trptype(tcp)
AMQ8014I: IBM MQ channel created.
     3 : set authrec objtype(qmgr) principal('mquser') authadd(allmqi)
AMQ8862I: IBM MQ authority record set.
     4 : set authrec profile(sample.svrconn) objtype(channel) principal('mquser') authadd(allmqi)
AMQ8862I: IBM MQ authority record set.
     5 : set authrec profile(queue1) objtype(queue) principal('mquser') authadd(allmqi)
AMQ8862I: IBM MQ authority record set.
     6 : refresh security(*)
AMQ8560I: IBM MQ security cache refreshed.
     7 : end
6 MQSC commands read.
No commands have a syntax error.
All valid MQSC commands were processed.
```

## Connectivity Test

The first thing to test is that you can connect to the queue manager whichever node it is running on.

Go to the client node and become the mquser user.

With the queue manager QM1 running on the first RDQM node, run:
```
./run_rdqmput -b 1 -m 1 -u mquser -v 1 QM1 QUEUE1
```

You will be prompted for the password for the mquser user. Enter that and then you should see something like:
```
Connected to queue manager QM1
Opened queue QUEUE1
Batch 1 put successfully, committing...
Batch 1 committed successfully
Completed
```

You should check the CURDEPTH of the queue and make sure it is 1, indicating one message is currently
on the queue, the one you have just put.

To check the CURDEPTH, on the node where the queue manager is running, as the rdqmadmin user run:
```
echo "dis ql(queue1) curdepth" | runmqsc QM1
```
which should produce something like:
```
5724-H72 (C) Copyright IBM Corp. 1994, 2021.
Starting MQSC for queue manager QM1.


     1 : dis ql(queue1) curdepth
AMQ8409I: Display Queue details.
   QUEUE(QUEUE1)                           TYPE(QLOCAL)
   CURDEPTH(1)
One MQSC command read.
No commands have a syntax error.
All valid MQSC commands were processed.
```

The important thing is `CURDEPTH(1)`

The next thing to check is that you can also run the run_rdqmget script with the queue manager running
on the first node.

On the client system as mquser run:
```
./run_rdqmget -b 1 -m 1 -u mquser -v 1 QM1 QUEUE1
```

Again you will be prompted for the password for mquser. After entering that, you should see something
like:
```
Connected to queue manager QM1
Opened queue QUEUE1
Batch 1 got successfully, committing...
Batch 1 committed successfully
Completed
```

If that worked, the CURDEPTH of the queue should now be zero, so check it as before and look for
`CURDEPTH(0)`

That completes the connectivity test to the first RDQM node.

The next thing to do is to move the queue manager to the second RDQM node, which we do by changing
the preferred location of the queue manager, which is initially the node on which the queue manager
was created, in this case the first node.

To change the preferred location, as rdqmadmin on the first node, run:
```
rdqmadm -p -m QM1 -n <node 2>
```
which should produce something like:
```
The preferred replicated data node has been set to '<node 2>'
for queue manager 'QM1'.
```

You can check that the queue manager has moved by running `rdqmstatus -m QM1` and checking that the `HA current location` is the new node.

Once the queue manager is running on the second node, run the `run_rdqmput` and `run_rdqmget` scripts again to check that they can still contact the queue manager.

Finally, change the preferred location to node 3 and repeat.

## Failover Tests

I usually do two failover tests:

1. change the preferred location of the queue manager while running `run_rdqmput`

2. change the preferred location of the queue manager while running `run_rdqmget`

In each case I check the CURDEPTH is the expected value at the end of the test,
showing that the correct number of messages were processed even though the queue manager was moved
and the application had to redo some of the work.

### First Failover Test

The `run_rdqmput` command I use for a full failover test is:
```
./run_rdqmput -u mquser -v 1 QM1 QUEUE1
```
and I usually change the preferred location just after a message saying that a batch of messages
was committed successfully.

In this case, I changed the preferred location of the queue manager QM1 to node 1 after batch 8 was committed successfully. The output I saw from running the test was:
```
Connected to queue manager QM1
Opened queue QUEUE1
Batch 1 put successfully, committing...
Batch 1 committed successfully
Batch 2 put successfully, committing...
Batch 2 committed successfully
Batch 3 put successfully, committing...
Batch 3 committed successfully
Batch 4 put successfully, committing...
Batch 4 committed successfully
Batch 5 put successfully, committing...
Batch 5 committed successfully
Batch 6 put successfully, committing...
Batch 6 committed successfully
Batch 7 put successfully, committing...
Batch 7 committed successfully
Batch 8 put successfully, committing...
Batch 8 committed successfully
MQPUT failed with MQRC_BACKED_OUT
backout successful
Batch 9 put successfully, committing...
Batch 9 committed successfully
Batch 10 put successfully, committing...
Batch 10 committed successfully
Batch 11 put successfully, committing...
Batch 11 committed successfully
Batch 12 put successfully, committing...
Batch 12 committed successfully
Batch 13 put successfully, committing...
Batch 13 committed successfully
Batch 14 put successfully, committing...
Batch 14 committed successfully
Batch 15 put successfully, committing...
Batch 15 committed successfully
Batch 16 put successfully, committing...
Batch 16 committed successfully
Batch 17 put successfully, committing...
Batch 17 committed successfully
Batch 18 put successfully, committing...
Batch 18 committed successfully
Batch 19 put successfully, committing...
Batch 19 committed successfully
Batch 20 put successfully, committing...
Batch 20 committed successfully
Completed
```

Note that batch 9 was backed out and retried. The CURDEPTH of the queue at the end of the test should
be 200 as the default settings are to use 20 batches each of 10 messages.

I checked the CURDEPTH on node 1 at the end of the test and it was 200.

This shows that the correct number of messages were published even though the queue manager failed over to another node.

### Second Failover Test

The `run_rdqmget` command I use for a full failover test is:
```
./run_rdqmget -u mquser -v 1 QM1 QUEUE1
```
and I usually change the preferred location just after a message saying that a batch of messages
was committed successfully.

This will attempt to get 20 batches, each of 10 messages, so should reduce the CURDEPTH by 200.

In this case, I changed the preferred location of the queue manager QM1 to node 2 after batch 12 was committed successfully. The output I saw from running the test was:
```
Connected to queue manager QM1
Opened queue QUEUE1
Batch 1 got successfully, committing...
Batch 1 committed successfully
Batch 2 got successfully, committing...
Batch 2 committed successfully
Batch 3 got successfully, committing...
Batch 3 committed successfully
Batch 4 got successfully, committing...
Batch 4 committed successfully
Batch 5 got successfully, committing...
Batch 5 committed successfully
Batch 6 got successfully, committing...
Batch 6 committed successfully
Batch 7 got successfully, committing...
Batch 7 committed successfully
Batch 8 got successfully, committing...
Batch 8 committed successfully
Batch 9 got successfully, committing...
Batch 9 committed successfully
Batch 10 got successfully, committing...
Batch 10 committed successfully
Batch 11 got successfully, committing...
Batch 11 committed successfully
Batch 12 got successfully, committing...
Batch 12 committed successfully
MQGET failed with MQRC_BACKED_OUT
backout successful
Batch 13 got successfully, committing...
Batch 13 committed successfully
Batch 14 got successfully, committing...
Batch 14 committed successfully
Batch 15 got successfully, committing...
Batch 15 committed successfully
Batch 16 got successfully, committing...
Batch 16 committed successfully
Batch 17 got successfully, committing...
Batch 17 committed successfully
Batch 18 got successfully, committing...
Batch 18 committed successfully
Batch 19 got successfully, committing...
Batch 19 committed successfully
Batch 20 got successfully, committing...
Batch 20 committed successfully
Completed
```

Note that batch 13 was backed out and retried. The CURDEPTH of the queue at the end of the test should
be 0.

At the end of the test I changed the preferred location of the queue manager back to node 1 and then checked the CURDEPTH and it was 0.