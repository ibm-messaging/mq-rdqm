# Deploying the template

The template and scripts in this directory can be used to deploy a single RDQM HA Group in one location or to deploy an
RDQM HA Group in each of two locations with DR between them.

This page describes setting up a full DR/HA configuration.
If you want to set up just an HA configuration then omit everything to do with setting up the second location and DR.

I used PowerShell 7 running on my MacBook to deploy the template.

When choosing a second location for a combined DR and HA configuration you will need to choose a second location with a network
latency of no more than 50 ms from the first location.

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
2. Individual RDQM Queue Manager

## Initial Azure Configuration

### Resource Group

The first thing to do is to create a resource group. I used the same resource group for both RDQM HA Groups.

I did:

```ps1
New-AzResourceGroup -Name rg-jc-rdqm-drha -Location uksouth
```

I made two copies the supplied rdqm.drha.azuredeploy.parameters.json file, one for the uksouth location and the
other for the northeurope location.

You will have to copy the supplied parameters file and set the parameters appropriately and supply that as the value of ```TemplateParameterFile```

The outputs of the template are:

1. the public IP address of the load balancer that is used for MQ traffic
2. the name of the vnet, which you will need for the DR configuration
3. the public IP address of the VM in zone 1
4. the public IP address of the VM in zone 2
5. the public IP address of the VM in zone 3

In a production deployment you may use a jump server or the Azure Bastion service rather than expose a public IP address for each VM.

### Deployment to uksouth

I went to the directory `cloud/azure' in my clone of this repository and deployed the template with the PowerShell command:

```ps1
New-AzResourceGroupDeployment -Name drha-uksouth -ResourceGroupName rg-jc-rdqm-drha -TemplateFile ./rdqm.drha.azuredeploy.json -TemplateParameterFile .../rdqm.drha.azuredeploy.parameters.uksouth.json
```

### Deployment to northeurope

I deployed the template again with the PowerShell command:

```ps1
New-AzResourceGroupDeployment -Name drha-northeurope -ResourceGroupName rg-jc-rdqm-drha -TemplateFile ./rdqm.drha.azuredeploy.json -TemplateParameterFile .../rdqm.drha.azuredeploy.parameters.northeurope.json
```

As I am setting up DR/HA I configured the vnet peering by running:

```ps1
./Add-VnetPeering.ps1 -ResourceGroupName rg-jc-rdqm-drha -Vnet1Name vnet-jc-uksouth -Vnet2Name vnet-jc-northeurope -InformationAction Continue
```

## VM Configuration

There are a couple of configuration steps that should be done on each VM before configuring the RDQM HA Group:

1. as the mqm user, ssh between each pair of nodes
2. configure and start the MQ health probe service

### ssh

On each node:

```bash
sudo su - mqm
ssh <remote node 1 RDQM IP address> uname -n
ssh <remote node 1 RDQM IP address> uname -n
exit
```

The `RDQM IP address` is the private IP address of the VM.

In each case, answer yes to the question about connecting.

### MQ Health Probe

To configure and start the MQ Health Probe, on each node:

1. `sudo -s`
2. Edit `/lib/systemd/system/mq_probe.service` and specify the private IP address of the VM as the value of `NODE_ADDRESS`
3. `systemctl start mq_probe`
4. `systemctl enable mq_probe`
5. `systemctl status mq_probe` and check that it is running
6. `netstat -antp` and check that there is a process listening on port 3001 of the IP address you specified
7. exit

## RDQM HA Group

### First location

On node vm-jc-1-uksouth I did:

`sudo su - rdqmadmin`

Edit `/var/mqm/rdqm.ini` to specify the three private/rdqm IP addresses. For example, my file contained:

```bash
Node:
  HA_Replication=10.240.1.5
Node:
  HA_Replication=10.240.1.6
Node:
  HA_Replication=10.240.1.4
```

`rdqmadm -c` which should produce output similar to:

```bash
Configuring the replicated data subsystem.
The replicated data subsystem has been configured on this node.
The replicated data subsystem has been configured on 'vm-2-uksouth'.
The replicated data subsystem has been configured on 'vm-3-uksouth'.
The replicated data subsystem configuration is complete.
Command '/opt/mqm/bin/rdqmadm' run with sudo.
```

### Second location

If you are setting up a DR/HA configuration, go through the same process for the RDQM HA Group
in the second location, using the appropriate IP addresses.

## RDQM HA Queue Manager

### Azure Configuration

There is some configuration of the load balancer required for an individual RDQM queue manager and some configuration
of the network security group for the subnet.

If you are setting up a DR/HA configuration then this configuration has to be repeated in the second location.

A set of PowerShell scripts are supplied to manage the Azure configuration for an individual RDQM queue manager.

I am going to create an RDQM queue manager named RDQMDRHAQM1 so I ran:

```ps1
./Add-QueueManagerConfig.ps1 -LoadBalancerName lbe-jc-uksouth -ResourceGroupName rg-jc-rdqm-drha -QueueManagerName RDQMDRHAQM1 -ProbePort 3001 -FrontendPort 2414 -BackendPort 1414 -NetworkSecurityGroupName nsg-jc-public-uksouth -SecurityRulePriority 1235 -DrPort 7001 -DrRulePriority 1236 -InformationAction Continue
```

You can check the configurations that this script added by looking at the load balancer health probes and rules and the inbound rules for the public network security group.

The other scripts are:

1. Set-QueueManagerConfig.ps1 which updates an existing configuration
2. Remove-QueueManagerConfig.ps1 which removes an existing configuration

These scripts use functions that are in functions.ps1

The scripts use ```Write-Information``` to display messages as they progress.

As I am setting up a DR/HA configuration, I also ran:

```ps1
./Add-QueueManagerConfig.ps1 -LoadBalancerName lbe-jc-northeurope -ResourceGroupName rg-jc-rdqm-drha -QueueManagerName RDQMDRHAQM1 -ProbePort 3001 -FrontendPort 2414 -BackendPort 1414 -NetworkSecurityGroupName nsg-jc-public-northeurope -SecurityRulePriority 1235 -DrPort 7001 -DrRulePriority 1236 -InformationAction Continue
```

### MQ Configuration

It is necessary to configure the firewall on each node to allow the DR replication traffic,
so on each node as the default user I did:

```bash
sudo firewall-cmd --zone=public --add-port=7001/tcp --permanent
```

As rdqmadmin on node vm-jc-1-uksouth I did:

```bash
crtmqm -p 1414 -sx -rr p -rl 10.240.1.5,10.240.1.6,10.240.1.4 -ri 10.241.1.5,10.241.1.4,10.241.1.6 -rp 7001 RDQMDRHAQM1
```

which produced:

```bash
Creating replicated data queue manager configuration.
Secondary queue manager created on 'vm-jc-2-uksouth'.
Secondary queue manager created on 'vm-jc-3-uksouth'.
IBM MQ queue manager created.
Directory '/var/mqm/vols/rdqmdrhaqm1/qmgr/rdqmdrhaqm1' created.
The queue manager is associated with installation 'Installation1'.
Creating or replacing default objects for queue manager 'RDQMDRHAQM1'.
Default objects statistics : 84 created. 0 replaced. 0 failed.
Completing setup.
Setup completed.
Enabling replicated data queue manager.
Replicated data queue manager enabled.
Issue the following command on the remote HA group to create the DR/HA
secondary queue manager:
crtmqm -sx -rr s -rl 10.241.1.5,10.241.1.4,10.241.1.6 -ri 10.240.1.5,10.240.1.6,10.240.1.4 -rp 7001 RDQMDRHAQM1
Command '/opt/mqm/bin/crtmqm' run with sudo.
```

I ran the generated crtmqm command as rdqmadmin on node vm-jc-1-northeurope:

```bash
crtmqm -sx -rr s -rl 10.241.1.5,10.241.1.4,10.241.1.6 -ri 10.240.1.5,10.240.1.6,10.240.1.4 -rp 7001 RDQMDRHAQM1
Creating replicated data queue manager configuration.
Secondary queue manager created on 'vm-jc-2-northeurope'.
Secondary queue manager created on 'vm-jc-3-northeurope'.
IBM MQ secondary queue manager created.
Enabling replicated data queue manager.
Replicated data queue manager enabled.
Command '/opt/mqm/bin/crtmqm' run with sudo.
```

I checked the RDQM status of the queue manager on vm-jc-1-uksouth with:

```bash
[rdqmadmin@vm-jc-1-uksouth ~]$ rdqmstatus -m RDQMDRHAQM1
Node:                                   vm-jc-1-uksouth
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
DR role:                                Primary
DR status:                              Normal
DR port:                                7001
DR local IP address:                    10.240.1.5
DR remote IP address list:              10.241.1.5,10.241.1.4,10.241.1.6
DR current remote IP address:           10.241.1.5

Node:                                   vm-jc-2-uksouth
HA status:                              Normal

Node:                                   vm-jc-3-uksouth
HA status:                              Normal
```

Once the queue manager is running you can check the health probe of the load balancer to ensure that the load balancer thinks that
the queue manager is healthy on one node.
In the Azure Portal, go to the load balancer and click on Metrics under Monitoring.
Select the Metric Health Probe Status

In the bar above where you selected the Metric, click on Apply splitting and select Backend IP Address which should give you three numbers at the bottom
of the display, representing the status on each of the three vms. If you move the cursor to the right edge of the display you should see a value of 100
for the IP address where the queue manager is running and values of 0 for the other two IP addresses.

There is some configuration of the queue manager that is necessary to run the sample scripts so I did:

```bash
runmqsc RDQMDRHAQM1 < configure.mqsc
```

which produced:

```bash
5724-H72 (C) Copyright IBM Corp. 1994, 2020.
Starting MQSC for queue manager RDQMDRHAQM1.


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
```
