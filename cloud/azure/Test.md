# Testing a queue manager

This page describes how to test HA takeover and DR takeover.
It assumes a queue manager named RDQMDRHAQM1.
If you have not configured DR then just use the intial HA test.

## Initial HA Test

Because an Azure load balancer is used,
we only need to configure a single IP address for the queue manager.

I created a file `run_rdqmput_RDQMDRHAQM1_uksouth` and edited it to use a single IP address,
which is the public address of the load balancer, and set the PORT to 2414.

The file I ended up with was:

```bash
#!/bin/bash

export MQDIR=~/MQ/Client/9.1.1.0
export DYLD_LIBRARY_PATH=${MQDIR}/lib64

IP=<Public IP address of load balancer>
PORT=2414

export MQSERVER=RDQM.SVRCONN/TCP/${IP}\(${PORT}\)
./rdqmput $*
```

The first thing to test is that you can communicate with the queue manager when it is running
on any of the three nodes in the RDQM HA Group.

I then ran `./run_rdqmput_RDQMDRHAQM1_uksouth -u rdqmuser -b 1 -m 1 -v 3 RDQMDRHAQM1 QUEUE1`

which produced:

```bash
password:
numberOfBatches is 1
messageSize is 2048
batchSize is 1
sleepSeconds is 1
verbosity is 3
QMgrName is "                                     RDQMDRHAQM1"
QName is "                                          QUEUE1"
Connected to queue manager RDQMDRHAQM1
Opened queue QUEUE1
About to put message 1 of batch 1
Message is "Batch 1, Message 1"
Message 1 put successfully
Batch 1 put successfully, committing...
Batch 1 committed successfully
Completed
```

It is now necessary to move the queue manager to another node and check that it can still be accessed. On the second node, as the rdqmadmin user, run `rdqmadm -m RDQMDRHAQM1 -p`

You can check that the queue manager has moved to the current VM by running `rdqmstatus` which should show something like:

```bash
Node:                                   vm-jc-2-uksouth

Queue manager name:                     RDQMDRHAQM1
Queue manager status:                   Running
HA current location:                    This node
DR role:                                Primary
```

If you are not using DR then the DR role will not be shown.

Once the queue manager is running on the second node, run the sample program again to check that it can connect to the queue manager on its new node.

The default configuration of an Azure load balancer is to have a probe interval of 15 seconds and the minimum number of failed attempts before a target is removed is 2 so it will take at least 30 seconds from when a queue manager stops running on a node before the load balancer will consider it failed. If you run the application too quickly you may get a failure with a return code of 2538. If you wait a little longer it should be able to connect.

The output from the sample should be exactly the same as from the first run.

Once it can connect to the second node, move the queue manager to the third node to check that it can connect to the queue manager running there too.

Once you have tested the connectivity to each VM you are ready to test moving the queue manager while the sample is running.

Before that test, I recommend that you empty the queue so that you can be sure that exactly the expected number of messages are on the queue at the end of the test.
To do that, on the node where the queue manager is running, do:

```bash
runmqsc RDQMDRHAQM1
clear ql(queue1)
end
```

To test moving the queue manager during the test, run the rdqmput sample with the default values for the number of batches and the size of each batch, which are 20 and 10 respectively, so there should be 200 messages on the queue at the end of the test, even though the queue manager has moved during the test.

While the test is running, run the rdqmadm command on one of the other nodes to move the queue manager.

I usually run a test like this with the verbosity at 1 so I ran `./run_rdqmput_RDQMDRHAQM1_uksouth -u rdqmuser -v 1 RDQMDRHAQM1 QUEUE1`

I changed the preferred location of the queue manager just after I saw that batch 3 was committed.

The output I got from the sample was:

```bash
password:
Connected to queue manager RDQMDRHAQM1
Opened queue QUEUE1
Batch 1 put successfully, committing...
Batch 1 committed successfully
Batch 2 put successfully, committing...
Batch 2 committed successfully
Batch 3 put successfully, committing...
Batch 3 committed successfully
MQPUT failed with MQRC_BACKED_OUT
backout successful
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

To check that 200 messages were put to the queue, on the node where the queue manager is now running, do:

```bash
runmqsc RDQMDRHAQM1
dis ql(queue1) curdepth
end
```

The output you should get is:

```bash
     1 : dis ql(queue1) curdepth
AMQ8409I: Display Queue details.
   QUEUE(QUEUE1)                           TYPE(QLOCAL)
   CURDEPTH(200)
```

The current depth of the queue is 200, which is correct.

If you are going to go on and do a DR test then you should leave the 200 messages in the queue.
If you are just doing an HA test then you can run the rdqmget sample to retrieve the messages.
To do this, copy the `run_rdqmput_RDQMDRHAQM1_uksouth` file to `run_rdqmget_RDQMDRHAQM1_uksouth` and edit it to run the rdqmget executable. Then run `./run_rdqmget_RDQMDRHAQM1_uksouth -u rdqmuser -v 1 RDQMDRHAQM1 QUEUE1`

You should see the output:

```bash
password:
Connected to queue manager RDQMDRHAQM1
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

If you check the curdepth of the queue again it should be 0.

You could try moving the queue manager while the rdqmget sample is running and after rdqmget completes the curdepth should still be 0.

## DR Test

To move the queue manager to the second location, on the node in the first location where the queue manager is current running issue the command `rdqmdr -m RDQMDRHAQM1 -s` which should produce:

```bash
Queue manager 'RDQMDRHAQM1' has been made the DR secondary on this node.
Command '/opt/mqm/bin/rdqmdr' run with sudo.
```

Then on the node in the second location where you want the queue manager to run issue the command `rdqmdr -m RDQMDRHAQM1 -p` which should produce:

```bash
Queue manager 'RDQMDRHAQM1' has been made the DR primary on this node.
Command '/opt/mqm/bin/rdqmdr' run with sudo.
```

Once the queue manager is running in the second location, check the curdepth of the queue again.
It should be 200.

You could configure two addresses if you are using DR, one for the load balancer in each location, but I chose
to have a separate configuration for each location so I copied `run_rdqmget_RDQMDRHAQM1_uksouth` to `run_rdqmget_RDQMDRHAQM1_northeurope` and edited the new file to use the IP address of the load balancer in the northeurope location.

I then ran `./run_rdqmget_RDQMDRHAQM1_northeurope -u rdqmuser -v 1 RDQMDRHAQM1 QUEUE1` which produced:

```bash
password:
Connected to queue manager RDQMDRHAQM1
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

I chose to let the test complete and then checked the curdepth, which should be 0.

You could move the queue manager to the other nodes in the HA Group in the second location while the get sample is running
or do a separate connectivity test once the initial test has completed, which is what I did.

Once the connectivity tests have been completed, if you did them separately then clear the queue.

You have now completed both HA and DR tests.
