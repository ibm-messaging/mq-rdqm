There are two types of configuration that have to be done:
1. Azure
2. MQ

There are different scopes of configuration.

For Azure there are three scopes:
1. Broader in scope than an individual Virtual Machine
2. Configuration specific to one Virtual Machine
3. Configuration specific to one MQ Queue Manager

For MQ there are two scopes:
1. RDQM HA Group
2. Individual RDQM HA Queue Manager

# Naming Conventions

I have changed my naming conventions in this approach as I am using IBM MQ Advanced 9.1.5 now and a future approach will illustrate
the combined DR and HA capability that is now available so I will end up with a resource group spanning two Azure regions
and a set of resources in each region.

# Initial Azure Configuration

The first thing to do is to create the resource group, vnet etc.
In this approach I have used an Azure Resource Manager (ARM) template.

I deployed it with the PowerShell command:
<pre>
New-AzResourceGroupDeployment -Name test3-1 -ResourceGroupName rg-jc-rdqm-test3 -TemplateFile ./rdqm.group.azuredeploy.json -TemplateParameterFile /Users/colgrave/MQ/RDQM/Clouds/Azure/HA/Approach2/rdqm.group.azuredeploy.parameters.json
</pre>

You will have to copy the supplied parameters file and set the parameters appropriately and supply that as the value of ```TemplateParameterFile```

The outputs of the template are:

1. the public IP address of the load balancer that is used for MQ traffic
2. the public IP address of the VM in zone 1
3. the public IP address of the VM in zone 2
4. the public IP address of the VM in zone 3

In a production deployment you may use a jump server or the Azure Bastion service rather than expose a public IP address for each VM.

# VM Configuration

There are a couple of configuration steps that should be done on each VM before configuring the RDQM HA Group:

1. as the mqm user, ssh between each pair of nodes
2. configure and start the MQ health probe service

### ssh

On each node:

<pre>
sudo su - mqm
ssh &lt;remote node 1 RDQM IP address &gt;
ssh &lt;remote node 1 RDQM IP address &gt;
exit
</pre>

The ```RDQM IP address``` is the IP address of the private nic for each VM, which should be the IP address of the ```eth1``` interface on each VM.

In each case, answer yes to the question about connecting.

### MQ Health Probe

To configure and start the MQ Health Probe, on each node:

1. ```sudo -s``` 
2. Run ```ip address list``` and note the IP address for eth0 which is the private IP address of the interface associated with the public IP address
3. Edit ```/lib/systemd/system/mq_probe.service``` and specify this IP address as the value of ```NODE_ADDRESS```
4. ```systemctl start mq_probe```
5. ```systemctl enable mq_probe```
6. ```systemctl status mq_probe``` and check that it is running
7. ```netstat -antp``` and check that there is a process listening on port 3001 of the IP address you specified
8. exit

# RDQM HA Group

On node 1 I did:

1. ```sudo su - rdqmadmin```
2. Edit ```/var/mqm/rdqm.ini``` to specify the three private/rdqm IP addresses. For example, my file contained:
<pre>
Node:
  HA_Replication=10.241.2.5
Node:
  HA_Replication=10.241.2.6
Node:
  HA_Replication=10.241.2.4
</pre>
3. ```rdqmadm -c``` which should produce output similar to:
<pre>
Configuring the replicated data subsystem.
The replicated data subsystem has been configured on this node.
The replicated data subsystem has been configured on 'vm-2-uksouth'.
The replicated data subsystem has been configured on 'vm-3-uksouth'.
The replicated data subsystem configuration is complete.
Command '/opt/mqm/bin/rdqmadm' run with sudo.
</pre>

# RDQM HA Queue Manager

## Azure Configuration

There is some configuration of the load balancer required for an individual RDQM HA queue manager and some configuration
of the network security group for the public subnet.

A set of PowerShell scripts are supplied to manage the Azure configuration for an individual RDQM HA queue manager.

I am going to create an RDQM HA queue manager named RDQMHAQM1 so I ran:
<pre>
./Add-QueueManagerConfig.ps1 -LoadBalancerName lbe-jc-uksouth -ResourceGroupName rg-jc-rdqm-test3 -QueueManagerName RDQMHAQM1 -ProbePort 3001 -FrontendPort 2414 -BackendPort 1414 -NetworkSecurityGroupName nsg-jc-public-uksouth -SecurityRulePriority 1235
</pre>

You can check the configurations that this script added by looking at the load balancer health probes and rules and the inbound rules for the public network security group.

The other scripts are:

1. Set-QueueManagerConfig.ps1 which updates an existing configuration
2. Remove-QueueManagerConfig.ps1 which removes an existing configuration

These scripts use functions that are in functions.ps1

The scripts use ```Write-Information``` to display messages as they progress.

## MQ Configuration

As rdqmadmin on node 1 I did:

<pre>
crtmqm -p 1414 -sx RDQMHAQM1
</pre>

which produced:

<pre>
Creating replicated data queue manager configuration.
Secondary queue manager created on 'vm-2-uksouth'.
Secondary queue manager created on 'vm-3-uksouth'.
IBM MQ queue manager created.
Directory '/var/mqm/vols/rdqmhaqm1/qmgr/rdqmhaqm1' created.
The queue manager is associated with installation 'Installation1'.
Creating or replacing default objects for queue manager 'RDQMHAQM1'.
Default objects statistics : 84 created. 0 replaced. 0 failed.
Completing setup.
Setup completed.
Enabling replicated data queue manager.
Replicated data queue manager enabled.
Command '/opt/mqm/bin/crtmqm' run with sudo.
</pre>

I checked the RDQM status of the queue manager with:

<pre>
rdqmstatus -m RDQMHAQM1
</pre>

which produced:
<pre>
Node:                                   vm-1-uksouth
Queue manager status:                   Running
CPU:                                    0.00%
Memory:                                 183MB
Queue manager file system:              58MB used, 2.9GB allocated [2%]
HA role:                                Primary
HA status:                              Normal
HA control:                             Enabled
HA current location:                    This node
HA preferred location:                  This node
HA floating IP interface:               None
HA floating IP address:                 None

Node:                                   vm-2-uksouth
HA status:                              Normal

Node:                                   vm-3-uksouth
HA status:                              Normal
</pre>

Once the queue manager is running you can check the health probe of the load balancer to ensure that the load balancer thinks that
the queue manager is healthy on one node.
In the Azure Portal, go to the load balancer and click on Metrics under Monitoring.
Select the Metric Health Probe Status

In the bar above where you selected the Metric, click on Apply splitting and select Backend IP Address which should give you three numbers at the bottom
of the display, representing the status on each of the three vms. If you move the cursor to the right edge of the display you should see a value of 100
for the IP address where the queue manager is running and values of 0 for the other two IP addresses.

There is some configuration of the queue manager that is necessary to run the sample scripts so I did:

<pre>
runmqsc RDQMHAQM1 &lt; configure.mqsc 
</pre>

which produced:

<pre>
5724-H72 (C) Copyright IBM Corp. 1994, 2020.
Starting MQSC for queue manager RDQMHAQM1.


       : * (C) Copyright IBM Corporation 2020
       : *
       : * Licensed under the Apache License, Version 2.0 (the "License");
       : * you may not use this file except in compliance with the License.
       : * You may obtain a copy of the License at
       : *
       : * http://www.apache.org/licenses/LICENSE-2.0
       : *
       : * Unless required by applicable law or agreed to in writing, software
       : * distributed under the License is distributed on an "AS IS" BASIS,
       : * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
       : * See the License for the specific language governing permissions and
       : * limitations under the License.
       : 
     1 : define ql(queue1)
AMQ8006I: IBM MQ queue created.
     2 : define channel(rdqm.svrconn) chltype(svrconn) trptype(tcp)
AMQ8014I: IBM MQ channel created.
     3 : set authrec objtype(qmgr) principal('rdqmuser') authadd(allmqi)
AMQ8862I: IBM MQ authority record set.
     4 : set authrec profile(rdqm.svrconn) objtype(channel) principal('rdqmuser') authadd(allmqi)
AMQ8862I: IBM MQ authority record set.
     5 : set authrec profile(queue1) objtype(queue) principal('rdqmuser') authadd(allmqi)
AMQ8862I: IBM MQ authority record set.
     6 : refresh security(*)
AMQ8560I: IBM MQ security cache refreshed.
     7 : end
6 MQSC commands read.
No commands have a syntax error.
All valid MQSC commands were processed.
</pre>