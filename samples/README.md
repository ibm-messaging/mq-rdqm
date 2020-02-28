The "High availability sample programs" shipped with MQ (AMQSGHAC, AMQSMHAC and AMQSPHAC) are samples that use a reconnectable connection to implement a highly available client. They are not written to work with a highly available queue manager.

The samples here were developed and tested with an RDQM HA queue manager but should work with any other kind of HA queue manager, or even an ordinary queue manager as whether a queue manager is an HA queue manager or not is not visible to a client application.

The main differences between the samples here and the samples shipped with MQ are:

1. the samples here force the use of persistent messages whereas the product samples force the use of non-persistent messages which is not usually appropriate for an HA queue manager
2. the samples here force the use of a syncpoint whereas the product samples force the use of no syncpoint; using a syncpoint is recommended when working with persistent messages and is an important part of application recovery after a queue manager failover
3. the samples here work with a specified number of batches where each batch is of a specified number of messages, which means that the total number of messages is predictable so can be checked once a test completes
4. the samples here allow a userid and password to be specified to connect to a queue manager where they are required


# rdqmget and rdqmput

The rdqmget sample is the equivalent of AMQSGHAC and the rdqmput sample is the equivalent of AMQSPHAC. There is no equivalent of AMQSMHAC so only a single queue is used.

The syntax of the two samples is the same:
<pre>
&lt;sample&gt; [options] &lt;queue manager name&gt; &lt;queue name&gt;
</pre>
where:
&lt;sample&gt; is either rdqmget or rdqmput and [options] is zero or more of:
<pre>
-b &lt;number of batches&gt;
-l &lt;message size&gt;
-m &lt;batch size&gt;
-p &lt;password&gt;
-s &lt;sleep time between messages&gt;
-u &lt;userid&gt;
-v &lt;verbosity&gt;
</pre>

The default number of batches is 20 and the default batch size is 10 so by default rdqmput will put a total of 200 messages and rdqmget will get a total of 200 messages.

The default message size is 2048 bytes. If you specify a size with rdqmget that is smaller than the size used with rdqmput then truncated messages will be retrieved and if you show the message content you will see that it is truncated.

By using a specified number of messages you can check that the right number of messages was processed:
* if you run just the putter then you can tell that the queue is of the correct depth at the end of a test; for example if the CURDEPTH of the queue is 0 before you run a test and you just run rdqmput with the defaults, the CURDEPTH of the queue should be 200 when rdqmput completes, even if the queue manager has failed over during the test.
* if you run the putter and getter with the same values for -b and -m or values that multiply to the same total number of messages, then at the end of the test the CURDEPTH of the queue should be the same as it was before the test
* if you run the getter with smaller values for -b and/or -m then the CURDEPTH should have increased by the difference, for example if you run the putter with the default values but run the getter with `-b 10` then at the end of the test the CURDEPTH of the queue should have increased by 100, even if the queue manager failed over, and perhaps moved back, during the test

The -u and -p options allow you to pass in a userid and password on the command line. If you specify just -u then you will be prompted to enter the password.

The -s option allows you to specify the time to wait between messages, in seconds. The default is 1 so rdqmput will sleep for 1 second between putting messages and rdqmget will sleep for 1 second between getting them. If you specify a value of 0 then the applications will go as fast as they can.

The -v option allows you to control the verbosity of the samples. The default value is 0 which prints out very little information, an example is given below. Other values are 1, 2 or 3 which produce increasing amounts of output. Samples of these are also below.

## Test configuration

I created two RDQM HA queue managers to test with:
1. HAQM1 which has security configured so I need to supply a userid and password
2. HAQM2 which has no security configured so I can show a command without a userid or password

## Default tests

I checked the CURDEPTH was zero on QUEUE1 on each queue manager.

I ran just rdqmput first to check that the expected number of messages were published.

I set the MQSERVER environment variable to include the three possible addresses for the queue manager:
<pre>
export MQSERVER='RDQM.SVRCONN/TCP/IP1(1601),IP2(1601),IP3(1601)'
</pre>
where IP1 is the IP address for the queue manager on the first RDQM node etc. Port 1601 is the listener port for the queue manager HAQM1.

The command for HAQM1 was:
<pre>
./rdqmput -u rdqmuser HAQM1 QUEUE1
password:
Connected to queue manager HAQM1
Opened queue QUEUE1
Completed
</pre>

I checked the CURDEPTH of QUEUE1 at the end of the run and it was 200, which is correct for the default of 20 batches of 10 messages.

I then ran rdqmget:
<pre>
./rdqmget -u rdqmuser HAQM1 QUEUE1
password:
Connected to queue manager HAQM1
Opened queue QUEUE1
Completed
</pre>

I checked the CURDEPTH again and it was zero, which is as expected.

I then did the same test against HAQM2 so I changed the value of the MQSERVER environment variable to use port 1602 which is the port I used for HAQM2.

The rdqmput command was:
<pre>
./rdqmput HAQM2 QUEUE1
Connected to queue manager HAQM2
Opened queue QUEUE1
Completed
</pre>

The CURDEPTH after this was 200.

The rdqmget command was:
<pre>
./rdqmget HAQM2 QUEUE1
Connected to queue manager HAQM2
Opened queue QUEUE1
Completed
</pre>

The CURDEPTH after this was 0.

## Verbosity tests

These tests demonstrate the different verbosity levels.

To keep the output to a reasonable amount I reduced both the number of batches and the number of messages in each batch so these tests also demonstrate those options.

The default verbosity is 0 so the first test I ran was with verbosity 1, which reports the status of each batch:
<pre>
./rdqmput -b 2 -m 2 -v 1 HAQM2 QUEUE1
Connected to queue manager HAQM2
Opened queue QUEUE1
Batch 1 put successfully, committing...
Batch 1 committed successfully
Batch 2 put successfully, committing...
Batch 2 committed successfully
Completed

./rdqmget -b 2 -m 2 -v 1 HAQM2 QUEUE1
Connected to queue manager HAQM2
Opened queue QUEUE1
Batch 1 got successfully, committing...
Batch 1 committed successfully
Batch 2 got successfully, committing...
Batch 2 committed successfully
Completed
</pre>

The next test was with verbosity 2, which reports the status of each message:
<pre>
./rdqmput -b 2 -m 2 -v 2 HAQM2 QUEUE1
Connected to queue manager HAQM2
Opened queue QUEUE1
About to put message 1 of batch 1
Message 1 put successfully
About to put message 2 of batch 1
Message 2 put successfully
Batch 1 put successfully, committing...
Batch 1 committed successfully
About to put message 1 of batch 2
Message 1 put successfully
About to put message 2 of batch 2
Message 2 put successfully
Batch 2 put successfully, committing...
Batch 2 committed successfully
Completed

./rdqmget -b 2 -m 2 -v 2 HAQM2 QUEUE1
Connected to queue manager HAQM2
Opened queue QUEUE1
About to get message 1 of batch 1
Message 1 got successfully
About to get message 2 of batch 1
Message 2 got successfully
Batch 1 got successfully, committing...
Batch 1 committed successfully
About to get message 1 of batch 2
Message 1 got successfully
About to get message 2 of batch 2
Message 2 got successfully
Batch 2 got successfully, committing...
Batch 2 committed successfully
Completed
</pre>

The final test was with verbosity 3 as that is the maximum value, which reports the options specified, the details of the queue manager and queue names and the content of each message:
<pre>
./rdqmput -b 2 -m 2 -v 3 HAQM2 QUEUE1
numberOfBatches is 2
batchSize is 2
sleepSeconds is 1
verbosity is 3
QMgrName is "                                           HAQM2"
QName is "                                          QUEUE1"
Connected to queue manager HAQM2
Opened queue QUEUE1
About to put message 1 of batch 1
Message is "Batch 1, Message 1"
Message 1 put successfully
About to put message 2 of batch 1
Message is "Batch 1, Message 2"
Message 2 put successfully
Batch 1 put successfully, committing...
Batch 1 committed successfully
About to put message 1 of batch 2
Message is "Batch 2, Message 1"
Message 1 put successfully
About to put message 2 of batch 2
Message is "Batch 2, Message 2"
Message 2 put successfully
Batch 2 put successfully, committing...
Batch 2 committed successfully
Completed

./rdqmget -b 2 -m 2 -v 3 HAQM2 QUEUE1
numberOfBatches is 2
batchSize is 2
sleepSeconds is 1
verbosity is 3
QMgrName is "                                           HAQM2"
QName is "                                          QUEUE1"
Connected to queue manager HAQM2
Opened queue QUEUE1
About to get message 1 of batch 1
Message 1 got successfully
Message is "Batch 1, Message 1"
About to get message 2 of batch 1
Message 2 got successfully
Message is "Batch 1, Message 2"
Batch 1 got successfully, committing...
Batch 1 committed successfully
About to get message 1 of batch 2
Message 1 got successfully
Message is "Batch 2, Message 1"
About to get message 2 of batch 2
Message 2 got successfully
Message is "Batch 2, Message 2"
Batch 2 got successfully, committing...
Batch 2 committed successfully
Completed
</pre>

## Failover tests

I will demonstrate two failover tests:
1. a managed failover where the original queue manager instance has time to shut down cleanly
2. an unmanaged failover where the original queue manager does not have time to shut down

The main differences are in the way that the client reconnects and the timing of the reconnection. In the first case a message is sent to the client to tell it to reconnect; in the second case the client reacts to the loss of the TCP connection.

My queue managers have a preferred location for the node on which they are initially running so in each test the queue manager moves back to that node once it restarts. I have chosen the number of batches and messages to be sufficient to show both reconnections.

I run both tests with verbosity 2 so that you can see where the failures are detected.

I run different numbers of batches and messages to demonstrate a failure being detected during a call to MQPUT and during a call to MQCMIT.

### Managed failover test

In this case, I used only one batch but put 200 messages so there was a good chance that the failures would be detected during calls to MQPUT.

I checked the CURDEPTH was 0 before running the test.

I rebooted the node running the queue manager with the standard linux `reboot` command.

I did this shortly after seeing message 10 put.

I have removed some of the output for brevity.

<pre>
./rdqmput -b 1 -m 200 -v 2 HAQM2 QUEUE1
Connected to queue manager HAQM2
Opened queue QUEUE1
About to put message 1 of batch 1
Message 1 put successfully
About to put message 2 of batch 1
Message 2 put successfully
...
About to put message 15 of batch 1
MQPUT failed with MQRC_BACKED_OUT
backout successful
About to put message 1 of batch 1
Message 1 put successfully
</pre>

Here we can see that it took until the MQPUT of message 15 for the queue manager to be shut down and the MQPUT call failed with MQRC_BACKED_OUT. The application then called MQBACK and resumed putting the messages of the current batch, so went back to message 1 of batch 1 as there was only one batch. The application is now connected to the queue manager running on one of the other nodes.

<pre>
About to put message 2 of batch 1
Message 2 put successfully
About to put message 3 of batch 1
Message 3 put successfully
...
About to put message 74 of batch 1
Message 74 put successfully
About to put message 75 of batch 1
MQPUT failed with MQRC_BACKED_OUT
backout successful
About to put message 1 of batch 1
Message 1 put successfully
</pre>

This failure is caused by the queue manager moving back to the first node once the first node rebooted. It is handled in exactly the same way as the first failure, so the application reconnects, in this case to the first node again, and begins putting messages again, with message 1 of batch 1, so it has to put all 200 messages again.

<pre>
About to put message 2 of batch 1
Message 2 put successfully
...
About to put message 199 of batch 1
Message 199 put successfully
About to put message 200 of batch 1
Message 200 put successfully
Batch 1 put successfully, committing...
Batch 1 committed successfully
Completed
</pre>

There were no more failures so this time the application put all 200 messages successfully.

The queue manager was running on the original node at the end of the test and I checked the CURDEPTH there at the end of the run and it was 200 which is the correct number, even though the queue manager has moved twice.

### Unmanaged failover test

For the unmanaged failover, I am going to reboot the first node with the command:
<pre>
echo b > /proc/sysrq-trigger
</pre>

This command reboots the node without doing a normal shutdown, so Pacemaker does not get chance to stop the HA queue managers which means the queue managers do not get chance to tell applications to reconnect, so applications have to respond to their TCP connections being closed by the queue managers.

For this test I reversed the settings for the number of batches and the number of messages per batch, so 200 batches each of 1 message. This makes it very likely that the failures will happen during MQCMIT calls as there will be an MQCMIT call for each MQPUT and the MQCMIT calls take longer than the MQPUT calls.

I ran the rdqmget command at the same time as the rdqmput command but I did not clear the first 200 messages from the first failover test, so the CURDEPTH at the end of this second failover test should be 200.

I triggered the reboot after the message for batch 21 was committed successfully.

At the end of the test I checked the CURDEPTH and it was 200 as expected, so the queue manager had moved twice and both applications had reconnected twice and the final result was as expected.

The output from the rdqmput application was, again with some output omitted for brevity:
<pre>
./rdqmput -b 200 -m 1 -v 2 HAQM2 QUEUE1
Connected to queue manager HAQM2
Opened queue QUEUE1
About to put message 1 of batch 1
Message 1 put successfully
Batch 1 put successfully, committing...
Batch 1 committed successfully
About to put message 1 of batch 2
Message 1 put successfully
Batch 2 put successfully, committing...
Batch 2 committed successfully
...
About to put message 1 of batch 21
Message 1 put successfully
Batch 21 put successfully, committing...
Batch 21 committed successfully
About to put message 1 of batch 22
Message 1 put successfully
Batch 22 put successfully, committing...
MQCMIT failed with MQCC_FAILED and MQRC_CALL_INTERRUPTED
About to put message 1 of batch 22
Message 1 put successfully
Batch 22 put successfully, committing...
Batch 22 committed successfully
</pre>

As expected, the failure was returned from MQCMIT this time not MQPUT.

It was the commit of batch 22 that failed so the application resumed by putting the first (and in this case the only) message of batch 22 again, it did not have to start from batch 1 again as those had been successfully committed.

<pre>
About to put message 1 of batch 23
Message 1 put successfully
Batch 23 put successfully, committing...
Batch 23 committed successfully
...
About to put message 1 of batch 42
Message 1 put successfully
Batch 42 put successfully, committing...
Batch 42 committed successfully
About to put message 1 of batch 43
Message 1 put successfully
Batch 43 put successfully, committing...
MQCMIT failed with MQCC_FAILED and MQRC_BACKED_OUT
About to put message 1 of batch 43
Message 1 put successfully
Batch 43 put successfully, committing...
Batch 43 committed successfully
</pre>

This failure was due to the queue manager moving back to the first node. Again the failure was returned from MQCMIT and the application just retried the current batch.

<pre>
About to put message 1 of batch 44
Message 1 put successfully
Batch 44 put successfully, committing...
Batch 44 committed successfully
...
About to put message 1 of batch 200
Message 1 put successfully
Batch 200 put successfully, committing...
Batch 200 committed successfully
Completed
</pre>

The output from the rdqmget command, also with some output omitted for brevity, was:
<pre>
./rdqmget -b 200 -m 1 -v 2 HAQM2 QUEUE1
Connected to queue manager HAQM2
Opened queue QUEUE1
About to get message 1 of batch 1
Message 1 got successfully
Batch 1 got successfully, committing...
Batch 1 committed successfully
About to get message 1 of batch 2
Message 1 got successfully
Batch 2 got successfully, committing...
Batch 2 committed successfully
...
About to get message 1 of batch 23
Message 1 got successfully
Batch 23 got successfully, committing...
MQCMIT failed with MQCC_FAILED and MQRC_CALL_INTERRUPTED
About to get message 1 of batch 23
Message 1 got successfully
Batch 23 got successfully, committing...
Batch 23 committed successfully
</pre>

Here also the failure was returned from a call to MQCMIT and the application retried just the current batch.

<pre>
About to get message 1 of batch 24
Message 1 got successfully
Batch 24 got successfully, committing...
Batch 24 committed successfully
...
About to get message 1 of batch 43
Message 1 got successfully
Batch 43 got successfully, committing...
MQCMIT failed with MQCC_FAILED and MQRC_BACKED_OUT
About to get message 1 of batch 43
Message 1 got successfully
Batch 43 got successfully, committing...
Batch 43 committed successfully
</pre>

This is the failure caused by the queue manager moving back to the original node, and again it was reported from a call to MQCMIT, and again the application responded by retrying just the current batch.

<pre>
About to get message 1 of batch 44
Message 1 got successfully
Batch 44 got successfully, committing...
Batch 44 committed successfully
...
About to get message 1 of batch 200
Message 1 got successfully
Batch 200 got successfully, committing...
Batch 200 committed successfully
Completed
</pre>

The rdqmget application then ran to completion, retrieving all 200 messages.
