# Introduction

This first approach is a simple manual approach, suitable for a PoC, to deploy the RDQM HA feature of IBM MQ Advanced to the Microsoft Azure public cloud.

The overall structure of this sample is:
1. Create Azure resource group and resources
2. Prepare  RHEL on each VM
3. Install RDQM on each VM
4. Configure RDQM HA Group
5. Create an RDQM HA queue manager
6. Configure the queue manager for the sample programs
7. Test the queue manager
8. Delete the Azure resource group and all resources

This example uses the UK South region but any Azure region with three Availability Zones could be used.

It uses default VM sizes etc. For a production deployment you should choose appropriate VM, storage and network capabilities to support your workload.

It is assumed that you have the ability to create VMs in Azure and ssh to them.

If you follow the steps in this sample you should have a working RDQM HA deployment in Azure. If you have any problems, please raise an Issue in this repository.

# Create Azure resource group and resources

This sample uses the Azure Cloud Shell.

A new resource group is created to make it easier to remove the configuration.

Once the new Azure resource group has been created, the following resources are created in it:
* three VMs, each in a different Availability Zone
* three data disks, one for each VM
* an inbound security rule in the default network security group created for each VM

## Creating the new resource group

To create the new resource group, run the command:

<pre>
az group create --name rg-rdqm-poc --location uksouth
</pre>

## Creating the three VMs

To create the VMs, one in each of three Availability Zones, run the commands:

<pre>
az vm create --name vm-rdqm-poc-1 --resource-group rg-rdqm-poc --image RHEL --location uksouth --ssh-key-values "&lt;your public key&gt;" --zone 1
az vm create --name vm-rdqm-poc-2 --resource-group rg-rdqm-poc --image RHEL --location uksouth --ssh-key-values "&lt;your public key&gt;" --zone 2
az vm create --name vm-rdqm-poc-3 --resource-group rg-rdqm-poc --image RHEL --location uksouth --ssh-key-values "&lt;your public key&gt;" --zone 3
</pre>

Where &lt;your public key&gt; is your SSH public key for Azure.

You will need the value of publicIpAddress of each VM to be able to log in to them and to connect to a queue manager running on any of the three nodes.

You will need the value of privateIpAddress of each VM to set up the rdqm.ini file as we will configure RDQM to use the three private IP addresses for all RDQM communication.

## Add a data disk to each VM

The easiest way to configure the drbdpool volume group for RDQM is to dedicate a disk to it so add a data disk to each VM using the commands:

<pre>
az vm disk attach --vm-name vm-rdqm-poc-1 --name vm-rdqm-poc-1_DrbdDisk --new --resource-group rg-rdqm-poc --size-gb 128
az vm disk attach --vm-name vm-rdqm-poc-2 --name vm-rdqm-poc-2_DrbdDisk --new --resource-group rg-rdqm-poc --size-gb 128
az vm disk attach --vm-name vm-rdqm-poc-3 --name vm-rdqm-poc-3_DrbdDisk --new --resource-group rg-rdqm-poc --size-gb 128
</pre>

## Add an Inbound security rule to each VM

It is necessary to add an inbound security rule to the network security group created for each VM to allow MQ traffic to reach the queue manager:
<pre>
az network nsg rule create --name MQ-RDQM1 --nsg-name vm-rdqm-poc-1NSG --priority 500 --resource-group rg-rdqm-poc --destination-port-ranges 1414 --direction Inbound --protocol Tcp
az network nsg rule create --name MQ-RDQM1 --nsg-name vm-rdqm-poc-2NSG --priority 500 --resource-group rg-rdqm-poc --destination-port-ranges 1414 --direction Inbound --protocol Tcp
az network nsg rule create --name MQ-RDQM1 --nsg-name vm-rdqm-poc-3NSG --priority 500 --resource-group rg-rdqm-poc --destination-port-ranges 1414 --direction Inbound --protocol Tcp
</pre>

If you want to create more than one queue manager then you will have to allow access to the listener port for each queue manager.
This sample uses one queue manager with the default listener port of 1414.

If you want to use a different single port or use multiple ports you will also have to open up the appropriate ports in the firewalld service of each VM, as described below.

## Summary

This is the end of the Azure configuration, the remainder of this sample is just normal Linux and RDQM configuration.

To see all of the Azure resources created, you can run the command:

<pre>
az resource list --resource-group rg-rdqm-poc -o table
</pre>

You should see something similar to:

<pre>
Name                                                     ResourceGroup    Location    Type                                     Status
-------------------------------------------------------  ---------------  ----------  ---------------------------------------  --------
vm-rdqm-poc-1_DrbdDisk                                   RG-RDQM-POC      uksouth     Microsoft.Compute/disks
vm-rdqm-poc-1_OsDisk_1_7a6e702bd0904c59bea96126cf82b887  RG-RDQM-POC      uksouth     Microsoft.Compute/disks
vm-rdqm-poc-2_DrbdDisk                                   RG-RDQM-POC      uksouth     Microsoft.Compute/disks
vm-rdqm-poc-2_OsDisk_1_b3062037378f4352b1054287223b7663  RG-RDQM-POC      uksouth     Microsoft.Compute/disks
vm-rdqm-poc-3_DrbdDisk                                   RG-RDQM-POC      uksouth     Microsoft.Compute/disks
vm-rdqm-poc-3_OsDisk_1_3a20a160f07941c981646769db9d861b  RG-RDQM-POC      uksouth     Microsoft.Compute/disks
vm-rdqm-poc-1                                            rg-rdqm-poc      uksouth     Microsoft.Compute/virtualMachines
vm-rdqm-poc-2                                            rg-rdqm-poc      uksouth     Microsoft.Compute/virtualMachines
vm-rdqm-poc-3                                            rg-rdqm-poc      uksouth     Microsoft.Compute/virtualMachines
vm-rdqm-poc-1VMNic                                       rg-rdqm-poc      uksouth     Microsoft.Network/networkInterfaces
vm-rdqm-poc-2VMNic                                       rg-rdqm-poc      uksouth     Microsoft.Network/networkInterfaces
vm-rdqm-poc-3VMNic                                       rg-rdqm-poc      uksouth     Microsoft.Network/networkInterfaces
vm-rdqm-poc-1NSG                                         rg-rdqm-poc      uksouth     Microsoft.Network/networkSecurityGroups
vm-rdqm-poc-2NSG                                         rg-rdqm-poc      uksouth     Microsoft.Network/networkSecurityGroups
vm-rdqm-poc-3NSG                                         rg-rdqm-poc      uksouth     Microsoft.Network/networkSecurityGroups
vm-rdqm-poc-1PublicIP                                    rg-rdqm-poc      uksouth     Microsoft.Network/publicIPAddresses
vm-rdqm-poc-2PublicIP                                    rg-rdqm-poc      uksouth     Microsoft.Network/publicIPAddresses
vm-rdqm-poc-3PublicIP                                    rg-rdqm-poc      uksouth     Microsoft.Network/publicIPAddresses
vm-rdqm-poc-1VNET                                        rg-rdqm-poc      uksouth     Microsoft.Network/virtualNetworks
</pre>

The majority of these resources were created implicitly when the VMs were created.

I don't know why the ResourceGroup is shown in upper case for the disks.

If you want to export a template for the resource group, you can run the command:
<pre>
az group export --name rg-rdqm-poc
</pre>

# Prepare RHEL on each VM

To check which release of RHEL you are running, do:
<pre>
cat /etc/redhat-release
</pre>

You should see:
<pre>
Red Hat Enterprise Linux Server release 7.7 (Maipo)
</pre>

This release of RHEL is supported by IBM MQ Advanced 9.1.4 so that is the version that will be used in this sample.

If the default RHEL image changes to RHEL 8 then you will have to specify a RHEL 7 image as RDQM is not yet supported on RHEL 8.


## Check private connectivity

From each VM, check that you can ping the other two VMs, using their private IP addresses.

The maximum round trip time supported by RDQM HA is 5 ms so make sure you see a value less than that.

## Create drbdpool volume group

To check that the disk was added, in each VM run the lsblk command after attaching the data disk.
You should see something similar to:

<pre>
lsblk
NAME              MAJ:MIN RM  SIZE RO TYPE MOUNTPOINT
fd0                 2:0    1    4K  0 disk 
sda                 8:0    0   64G  0 disk 
├─sda1              8:1    0  500M  0 part /boot/efi
├─sda2              8:2    0  500M  0 part /boot
├─sda3              8:3    0    2M  0 part 
└─sda4              8:4    0   63G  0 part 
  ├─rootvg-rootlv 253:0    0    2G  0 lvm  /
  ├─rootvg-usrlv  253:1    0   10G  0 lvm  /usr
  ├─rootvg-tmplv  253:2    0    2G  0 lvm  /tmp
  ├─rootvg-optlv  253:3    0    2G  0 lvm  /opt
  ├─rootvg-homelv 253:4    0    1G  0 lvm  /home
  └─rootvg-varlv  253:5    0    8G  0 lvm  /var
sdb                 8:16   0    7G  0 disk 
└─sdb1              8:17   0    7G  0 part /mnt/resource
<b>sdc                 8:32   0  128G  0 disk </b>
sr0                11:0    1  628K  0 rom  
</pre>

The highlighted line is the new disk that was added to the VM.

To create the drbdpool volume group on each VM, run as root:

<pre>
vgcreate drbdpool /dev/sdc
</pre>

## Linux Kernel Configuration

There are some changes that should be made to avoid MQ producing a warning when it is installed.

In each VM, as root, run the following:
<pre>
echo 'kernel.sem = 32 4096 32 128' >> /etc/sysctl.conf
echo 'kernel.threads-max = 32768' >> /etc/sysctl.conf
echo 'fs.file-max = 524288' >> /etc/sysctl.conf
sysctl -p
echo '* - nofile 10240' >> /etc/security/limits.conf
echo 'root - nofile 10240' >> /etc/security/limits.conf
</pre>

Then exit from being root and sudo (`I use sudo -i`) again ready for the following steps.

# Install RDQM on each VM

## Installing IBM MQ Advanced 9.1.4

Copy the file `IBM_MQ_9.1.4_LINUX_X86-64.tar.gz` to the home directory of your RHEL user then as root moved it to the /root directory.

Then run the following:
<pre>
tar -xzf IBM_MQ_9.1.4_LINUX_X86-64.tar.gz
cd MQServer
./mqlicense.sh -accept
cd Advanced/RDQM
./installRDQMsupport
</pre>

## Set new MQ installation as the primary installation

This is not strictly necessary but you could run:
<pre>
/opt/mqm/bin/setmqinst -i -p /opt/mqm
</pre>

## Modify RHEL user

It is easier if some additional groups are added to your Azure RHEL user, now that MQ is installed. To do this run:
<pre>
usermod -a -G haclient,mqm &lt;user&gt;
</pre>
where &lt;user&gt; is the userid you log in to RHEL on Azure with.

Add the following line to the .bash_profile of the user:
<pre>
. /opt/mqm/bin/setmqenv -s
</pre>

Log out completely from each VM then log back in again and run `dspmqver`.

You should see:

<pre>
Name:        IBM MQ
Version:     9.1.4.0
Level:       p914-L191119
BuildType:   IKAP - (Production)
Platform:    IBM MQ for Linux (x86-64 platform)
Mode:        64-bit
O/S:         Linux 3.10.0-1062.9.1.el7.x86_64
O/S Details: Red Hat Enterprise Linux Server 7.7 (Maipo)
InstName:    Installation1
InstDesc:    
Primary:     Yes
InstPath:    /opt/mqm
DataPath:    /var/mqm
MaxCmdLevel: 914
LicenseType: Production
</pre>

## Granting sudo access

Administration and configuration of RDQM is easiest if the mqm user has some specific sudo privileges and can ssh between the three instances without a password so this sample shows how to configure that.

To grant the required sudo access to the mqm user, on each VM as root create a file `/etc/sudoers.d/mqm` containing:

<pre>
mqm ALL=(root) NOPASSWD: /opt/mqm/bin/crtmqm, /opt/mqm/bin/dltmqm, /opt/mqm/bin/rdqmadm, /opt/mqm/bin/rdqmstatus
</pre>

## Configuring SELinux for DRBD

If you plan to run SELInux in your instances (it is on by default) you need to run as root:
<pre>
yum -y install policycoreutils-python
semanage permissive -a drbd_t
</pre>

## Configuring firewall

RDQM comes with a sample script that configures the RHEL firewalld to allow RDQM to operate.
Only a single MQ port, 1414, is open by default so if you want to use more than one queue manager you will have to allow multiple ports.

As root on each VM run:
<pre>
/opt/mqm/samp/rdqm/firewalld/configure.sh
</pre>

## Creating a normal user

Create a normal (not in the mqm group) user to run the sample programs:
<pre>
useradd rdqmuser
passwd rdqmuser
</pre>

Make sure you use the same password for this user on all three VMs.

## Setting up passwordless ssh

Once MQ has been installed in all three VMs, it is possible to set up passwordless ssh for the mqm user so that it can ssh between the VMs.

The easiest way to manage RDQM is to enable passwordless ssh for the mqm user in addition to the sudo access.

The first part of enabling passwordless ssh is to do the following on each node as root:

<pre>
usermod -d /home/mqm mqm
mkhomedir_helper mqm
passwd mqm
su mqm
ssh-keygen -t rsa -f /home/mqm/.ssh/id_rsa -N ''
exit
Edit /etc/ssh/sshd_config and change PasswordAuthentication to yes
systemctl restart sshd.service
</pre>

Once these steps have been done, copy the SSH key from each node to both of the other two nodes with commands like:

<pre>
su mqm
ssh-copy-id -i /home/mqm/.ssh/id_rsa.pub &lt;private IP address of first remote node&gt;
ssh-copy-id -i /home/mqm/.ssh/id_rsa.pub &lt;private IP address of second remote node&gt;
exit
</pre>

Once the SSH keys have been copied change PasswordAuthentication back to no and restart the sshd service again.

Test that the mqm user can ssh between all nodes without having to enter a password by doing on each VM:

<pre>
su - mqm
ssh &lt;private IP address of first remote node&gt; uname -n
ssh &lt;private IP address of second remote node&gt; uname -n
exit
</pre>

Finally, restore the mqm account:

<pre>
passwd -d mqm
passwd -l mqm
</pre>

# Configure RDQM HA Group

Once ssh and sudo are configured, as your normal user on one node edit the file /var/mqm/rdqm.ini and enter the private IP address for each node.
The file should look similar to:

<pre>
Node:
  HA_Replication=10.0.0.4
Node:
  HA_Replication=10.0.0.5
Node:
  HA_Replication=10.0.0.6
</pre>

Once the file is correct run the command `rdqmadm -c` which should produce something like:

<pre>
Configuring the replicated data subsystem.
The replicated data subsystem has been configured on this node.
The replicated data subsystem has been configured on 'vm-rdqm-poc-2'.
The replicated data subsystem has been configured on 'vm-rdqm-poc-3'.
The replicated data subsystem configuration is complete.
Command '/opt/mqm/bin/rdqmadm' run with sudo.
</pre>

# Create an RDQM HA queue manager

Create an RDQM HA queue manager using the command:
<pre>
crtmqm -p 1414 -sx RDQM1
</pre>

which should produce:

<pre>
Creating replicated data queue manager configuration.
Secondary queue manager created on 'vm-rdqm-poc-2'.
Secondary queue manager created on 'vm-rdqm-poc-3'.
IBM MQ queue manager created.
Directory '/var/mqm/vols/rdqm1/qmgr/rdqm1' created.
The queue manager is associated with installation 'Installation1'.
Creating or replacing default objects for queue manager 'RDQM1'.
Default objects statistics : 84 created. 0 replaced. 0 failed.
Completing setup.
Setup completed.
Enabling replicated data queue manager.
Replicated data queue manager enabled.
Command '/opt/mqm/bin/crtmqm' run with sudo.
</pre>

If you check on the status of the RDQM with the command `rdqmstatus -m RDQM1` on the VM where you created it, you should see something like:

<pre>
Node:                                   vm-rdqm-poc-1
Queue manager status:                   Running
CPU:                                    0.02%
Memory:                                 107MB
Queue manager file system:              58MB used, 2.9GB allocated [2%]
HA role:                                Primary
HA status:                              Normal
HA control:                             Enabled
HA current location:                    This node
HA preferred location:                  This node
HA floating IP interface:               None
HA floating IP address:                 None

Node:                                   vm-rdqm-poc-2
HA status:                              Normal

Node:                                   vm-rdqm-poc-3
HA status:                              Normal
</pre>

You may see the initial sync in progress if you are quick with the `rdqmstatus` command, but once the initial sync has completed you should see something like the above.

# Configure the queue manager for the sample programs

On the VM running the queue manager, as your normal user:
<pre>
runmqsc RDQM1
define ql(queue1)
define channel(rdqm.svrconn) chltype(svrconn) trptype(tcp)
set authrec objtype(qmgr) principal('rdqmuser') authadd(allmqi)
set authrec profile(rdqm.svrconn) objtype(channel) principal('rdqmuser') authadd(allmqi)
set authrec profile(queue1) objtype(queue) principal('rdqmuser') authadd(allmqi)
refresh security(*)
end
</pre>

# Test the queue manager

I used the sample programs in this repository to test the queue manager and I ran them on my MacBook Pro.

The first thing to test is that you can access the queue manager on each of the three VMs.

I copied the `run_rdqmput` file to `run_rdqmput_azure` and edited the `run_rdqmput_azure` file in the macOS directory of the samples.
I configured the three public IP addresses of the VMs and set the PORT to 1414

I then ran:
<pre>
./run_rdqmput_azure -u rdqmuser -b 1 -m 1 -v 3 RDQM1 QUEUE1
</pre>
which produced:
<pre>
password:
numberOfBatches is 1
messageSize is 2048
batchSize is 1
sleepSeconds is 1
verbosity is 3
QMgrName is "                                           RDQM1"
QName is "                                          QUEUE1"
Connected to queue manager RDQM1
Opened queue QUEUE1
About to put message 1 of batch 1
Message is "Batch 1, Message 1"
Message 1 put successfully
Batch 1 put successfully, committing...
Batch 1 committed successfully
Completed
</pre>

It is now necessary to move the queue manager to another VM and check that it can still be accessed. On the second VM, as the normal user, run:
<pre>
rdqmadm -m RDQM1 -p
</pre>

You can check that the queue manager has moved to the current VM by running:
<pre>
rdqmstatus
</pre>
which should show:
<pre>
Node:                                   vm-rdqm-poc-2

Queue manager name:                     RDQM1
Queue manager status:                   Running
HA current location:                    This node
</pre>

Once the queue manager is running on the second node, run the sample program again to check that it can connect to the queue manager on its new node.

Once it can connect to the second node, move the queue manager to the third node to check that it can connect to the queue manager running there too.

Once you have tested the connectivity to each VM you are ready to test moving the queue manager while the sample is running.

Before that test, I recommend that you empty the queue so that you can be sure that exactly the expected number of messages are on the queue at the end of the test.
To do that, on the node where the queue manager is running, do:
<pre>
runmqsc RDQM1
clear ql(queue1)
end
</pre>

To test moving the queue manager during the test, run the rdqmput sample with the default values for the number of batches and the size of each batch, which are 20 and 10 respectively, so there should be 200 messages on the queue at the end of the test, even though the queue manager has moved during the test.

While the test is running, run the rdqmadm command on one of the other nodes to move the queue manager.

I usually run a test like this with the verbosity at 1 so I ran:
<pre>
./run_rdqmput_azure -u rdqmuser -v 1 RDQM1 QUEUE1
</pre>

I changed the preferred location of the queue manager just after I saw that batch 3 was committed.

The output I got from the sample was:
<pre>
password:
Connected to queue manager RDQM1
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
runmqsc RDQM1
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

If you want to run the rdqmget sample to retrieve the messages, copy the `run_rdqmput_azure` file to `run_rdqmget_azure` and edit it to run the rdqmget executable. Then run:
<pre>
./run_rdqmget_azure -u rdqmuser -v 1 RDQM1 QUEUE1
</pre>

You should the output:
<pre>
password:
Connected to queue manager RDQM1
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

# Delete the Azure resource group and all resources

To delete the resource group and all the resources, run the command:
<pre>
az group delete --name rg-rdqm-poc
</pre>

This command takes quite a long time to complete.