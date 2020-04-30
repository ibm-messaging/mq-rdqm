The approach to testing the second approach is very similar to that used in the first approach.
The only difference is that because an Azure load balancer is used,
we only need to configure a single IP address for the queue manager.

I copied the `run_rdqmput_azure` file to ```run_rdqmput_azure_RDQMHAQM1``` and edited it to use a single IP address,
which is the public address of the load balancer, and set the PORT to 2414.

The file I ended up with was:

<pre>
#!/bin/bash

export MQDIR=~/MQ/Client/9.1.1.0
export DYLD_LIBRARY_PATH=${MQDIR}/lib64

IP=&lt;Public IP address of load balancer&gt;
PORT=2414

export MQSERVER=RDQM.SVRCONN/TCP/${IP}\(${PORT}\)
./rdqmput $*
</pre>

I then ran:
<pre>
./run_rdqmput_azure_RDQMHAQM1 -u rdqmuser -b 1 -m 1 -v 3 RDQMHAQM1 QUEUE1
</pre>
which produced:
<pre>
password:
numberOfBatches is 1
messageSize is 2048
batchSize is 1
sleepSeconds is 1
verbosity is 3
QMgrName is "                                       RDQMHAQM1"
QName is "                                          QUEUE1"
Connected to queue manager RDQMHAQM1
Opened queue QUEUE1
About to put message 1 of batch 1
Message is "Batch 1, Message 1"
Message 1 put successfully
Batch 1 put successfully, committing...
Batch 1 committed successfully
Completed
</pre>

It is now necessary to move the queue manager to another VM and check that it can still be accessed. On the second VM, as the rdqmadmin user, run:
<pre>
rdqmadm -m RDQM1 -p
</pre>

You can check that the queue manager has moved to the current VM by running:
<pre>
rdqmstatus
</pre>
which should show something like:
<pre>
Node:                                   vm-2-uksouth

Queue manager name:                     RDQMHAQM1
Queue manager status:                   Running
HA current location:                    This node
</pre>

Once the queue manager is running on the second node, run the sample program again to check that it can connect to the queue manager on its new node.

The default configuration of an Azure load balancer is to have a probe interval of 15 seconds and the minimum number of failed attempts before a target is removed is 2 so it will take at least 30 seconds from when a queue manager stops running on a node before the load balancer will consider it failed. If you run the application too quickly you may get a failure with a return code of 2538. If you wait a little longer it should be able to connect.

Once it can connect to the second node, move the queue manager to the third node to check that it can connect to the queue manager running there too.

Once you have tested the connectivity to each VM you are ready to test moving the queue manager while the sample is running.

Before that test, I recommend that you empty the queue so that you can be sure that exactly the expected number of messages are on the queue at the end of the test.
To do that, on the node where the queue manager is running, do:
<pre>
runmqsc RDQMHAQM1
clear ql(queue1)
end
</pre>

To test moving the queue manager during the test, run the rdqmput sample with the default values for the number of batches and the size of each batch, which are 20 and 10 respectively, so there should be 200 messages on the queue at the end of the test, even though the queue manager has moved during the test.

While the test is running, run the rdqmadm command on one of the other nodes to move the queue manager.

I usually run a test like this with the verbosity at 1 so I ran:
<pre>
./run_rdqmput_azure_RDQMHAQM1 -u rdqmuser -v 1 RDQMHAQM1 QUEUE1
</pre>

I changed the preferred location of the queue manager just after I saw that batch 3 was committed.

The output I got from the sample was:
<pre>
password:
Connected to queue manager RDQMHAQM1
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
</pre>

To check that 200 messages were put to the queue, on the node where the queue manager is now running, do:
<pre>
runmqsc RDQMHAQM1
dis ql(queue1) curdepth
end
</pre>

The output you should get is:
<pre>
     1 : dis ql(queue1) curdepth
AMQ8409I: Display Queue details.
   QUEUE(QUEUE1)                           TYPE(QLOCAL)
   CURDEPTH(200)                        
</pre>

The current depth of the queue is 200, which is correct.

If you want to run the rdqmget sample to retrieve the messages, copy the `run_rdqmput_azure_RDQMHAQM1` file to `run_rdqmget_azure_RDQMHAQM1` and edit it to run the rdqmget executable. Then run:
<pre>
./run_rdqmget_azure_RDQMHAQM1 -u rdqmuser -v 1 RDQMHAQM1 QUEUE1
</pre>

You should see the output:
<pre>
password:
Connected to queue manager RDQMHAQM1
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
</pre>

If you check the curdepth of the queue again it should be 0.

You could try moving the queue manager while the rdqmget sample is running and after rdqmget completes the curdepth should still be 0.
