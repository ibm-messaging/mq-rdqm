# Deploying IBM(R) MQ Advanced RDQM on AWS

This sample shows how to deploy the Replicated Data Queue Manager (RDQM) support that is part of IBM MQ Advanced 9.0.4 on Linux to AWS.

At the moment RDQM requires Red Hat Enterprise Linux 7.3 or 7.4 on x86_64.

This sample includes a CloudFormation Template that creates three Instances, one in each Availability Zone of a Region. A script, ```createStack```, is provided which creates a stack using the AWS CLI so if you wish to use this script you have to install and configure the AWS CLI.

As three Instances have to be created you will probably want to create your own AMI so that you can do most of the configuration, including installing MQ, only once.

Administration and configuration of RDQM is easiest if the mqm user has some specific sudo privileges and can ssh between the three instances without a password.

The description of creating the AMI below includes setting up the sudo access. Setting up the passwordless ssh is done once the instances have been created.

## Creating a new AMI

I created an instance in the AWS Console using the AMI `Red Hat Enterprise Linux 7.4 (HVM), SSD Volume Type - ami-223f945a`

When choosing an instance type for your instances the two main factors as far as the performance of RDQM are concerned are the storage performance and the network performance. I suggest that you find an instance type and storage type that supports your desired workload without RDQM and then experiment with the various networking options to find one that allows for the performance you need when RDQM is used.

I chose a size of 16GB for the initial storage volume.

RDQM requires a volume group named drbdpool so I am going to add a second volume to the AMI to support that. Note that when you create an instance of your AMI the size of the second volume has to be at least as large as the volume used in the AMI. I used an `Amazon EBS Provisioned IOPS SSD (io1)` volume.

### Linux Kernel Configuration

There are some changes that should be made to avoid MQ producing a warning when it is installed.

There is also one additional package that needs to be installed before installing MQ.

I ran the following:
```
sudo -s
yum install bc
echo 'kernel.shmmax=268435456' >> /etc/sysctl.conf
echo 'vm.overcommit_memory=2' >> /etc/sysctl.conf
sysctl -p
echo '* hard nofile 10240' >> /etc/security/limits.conf
echo '* soft nofile 10240' >> /etc/security/limits.conf
echo 'root hard nofile 10240' >> /etc/security/limits.conf
echo 'root soft nofile 10240' >> /etc/security/limits.conf
exit
sudo -i
```

### Installing IBM MQ

I installed the Trial version of IBM MQ Advanced 9.0.4 for Linux. If you have spare licenses you could install the IBM MQ Advanced product.

I created a directory /root/MQ and copied the file `MQ_V9.0.4_CDR_TRIAL_LIN_X86_64-BI.tar.gz` to it. I then ran the following:
```
cd MQ
tar -xvzf MQ_V9.0.4_CDR_TRIAL_LIN_X86_64-BI.tar.gz
cd MQServer
./mqlicense.sh -accept
cd Advanced/RDQM/
./installRDQMsupport
/opt/mqm/bin/setmqinst -i -p /opt/mqm
usermod -a -G haclient,mqm ec2-user
```

I then added the following line to /home/ec2-user/.bash_profile:

```. /opt/mqm/bin/setmqenv -s```

I logged out completely and logged back in again.

I ran mqconfig which reported PASS for everything.

I ran `dspmqver` which produced:

```
Name:        IBM MQ
Version:     9.0.4.0
Level:       p904-L171031.TRIAL
BuildType:   IKAP - (Production)
Platform:    IBM MQ for Linux (x86-64 platform)
Mode:        64-bit
O/S:         Linux 3.10.0-693.el7.x86_64
InstName:    Installation1
InstDesc:    
Primary:     Yes
InstPath:    /opt/mqm
DataPath:    /var/mqm
MaxCmdLevel: 904
LicenseType: Trial
```

### Setting up drbdpool

To set up the drbdpool volume group I did:

```
sudo -s
yum install lvm2
pvcreate /dev/nvme1n1
vgcreate drbdpool /dev/nvme1n1
```

### Granting sudo access

To grant the required sudo access to the mqm user, as root I created a file `/etc/sudoers.d/mqm` containing:

```
mqm ALL=(root) NOPASSWD: /opt/mqm/bin/crtmqm, /opt/mqm/bin/dltmqm, /opt/mqm/bin/rdqmadm, /opt/mqm/bin/rdqmstatus
```

### Configuring SELinux for DRBD

If you plan to run SELInux in your instances you need to run `semanage permissive -a drbd_t` as root.

### Creating the AMI

I stop my Instance before creating the AMI so I exited and stopped the Instance in the AWS Console.

Create your image by running `Actions > Image > Create Image` and note the ID id the image created.

## Create a stack using this AMI

You can use the AMI you created to create a stack. You can create a stack in the AWS console or with the supplied createStack script. If you use the supplied CloudFormation template it will create:

1. a Virtual Private Cloud (VPC) to contain the other resources
2. an InternetGateway to allow access to the virtual servers over the Internet
3. a VPCGatewayAttachment to associate the InternetGateway with the VPC
4. a RouteTable for the VPC
5. three SubnetRouteTableAssociations, one for each of the SubnetRouteTableAssociations
6. a Route that allows access to any of the IP addresses
7. three Subnets, one for each Availability Zone
8. three Instances, one for each Availability Zone
9. a number of SecurityGroups controlling access for various components:
   - InstanceSecurityGroup that allows ssh access via port 22 to any IP address
   - MQSecurityGroup that allows TCP access to the ports 1414, 1515 and 1616
   - PacemakerSecurityGroup that allows UDP access using the ports that Pacemaker uses
   - RDQMSecurityGroup that allows ping access
   - DRBDSecurityGroup that allows tcp access to any port on any IP address, to allow the DRBD instances to communicate with each other

### createStack command

The syntax of the `createStack` command is:

```
createStack <AMI> <stack name> <instance type> <region> <ssh key pair> <owner>
```

The arguments are:

1. the ID if the AMI you want to use to create the instances
2. the name of the stack which is used to generate various resource names
3. the instance type you want to use for the three instances
4. the region in which to deploy the stack, which must have three availability zones
5. the ssh key pair to use to access the instances
6. the owner of the stack which is used to tag the created resources

The outputs of the stack creation are the public and private IP addresses of the three instances.

You can check the progress of the creation of the stack in the AWS Console or by using the describeStack command.

### describeStack command

The syntax of the `describeStack` command is:

```
describeStack <stack name> <region>
```

## Configuring the instances

Once the stack has been created there is some configuration that has to be done on each instance before you can create an HA Group out of the three nodes and create your first RDQM.

### Setting up passwordless ssh

The easiest way to manage RDQM is to enable passwordless ssh for the mqm user in addition to the sudo access that was set up in the AMI.

The first part of enabling passwordless ssh is to do the following on each node as root:

```
usermod -d /home/mqm mqm
mkhomedir_helper mqm
passwd mqm
su mqm
ssh-keygen -t rsa -f /home/mqm/.ssh/id_rsa -N ''
exit
Edit /etc/ssh/sshd_config and change PasswordAuthentication to yes
systemctl restart sshd.service
```

Once these steps have been done, copy the SSH key from each node to both of the other two nodes with commands like:

```
su mqm
ssh-copy-id -i /home/mqm/.ssh/id_rsa.pub <private IP address of node>
```

Once the SSH keys have been copied change PasswordAuthentication back to no and restart the sshd service again.

Test that the mqm user can ssh between all nodes without having to enter a password.

### Configure HA Group

Once ssh is working, as ec2-user on one node edit the file /var/mqm/rdqm.ini and enter the private IP address for each node. The file should look similar to:

```
Node:
  HA_Replication=10.0.1.134
Node:
  HA_Replication=10.0.2.211
Node:
  HA_Replication=10.0.3.139
```

Once the file is correct run the command `rdqmadm -c` which should produce something like:

```
Configuring the replicated data subsystem.
The replicated data subsystem has been configured on this node.
The replicated data subsystem has been configured on
'ip-10-0-2-244.us-west-2.compute.internal'.
The replicated data subsystem has been configured on
'ip-10-0-3-152.us-west-2.compute.internal'.
The replicated data subsystem configuration is complete.
Command '/opt/mqm/bin/rdqmadm' run with sudo.
```

### Create an RDQM

The simplest command to create an RDQM is something like `crtmqm -p 1414 -sx RDQM1` which should produce something like:

```
There are 90 days left in the trial period for this copy of IBM MQ.
Creating replicated data queue manager configuration.
Secondary queue manager created on 'ip-10-0-2-244.us-west-2.compute.internal'.
Secondary queue manager created on 'ip-10-0-3-152.us-west-2.compute.internal'.
IBM MQ queue manager created.
Directory '/var/mqm/vols/RDQM1/qmgr/RDQM1' created.
The queue manager is associated with installation 'Installation1'.
Creating or replacing default objects for queue manager 'RDQM1'.
Default objects statistics : 85 created. 0 replaced. 0 failed.
Completing setup.
Setup completed.
Enabling replicated data queue manager.
Replicated data queue manager enabled.
Command '/opt/mqm/bin/crtmqm' run with sudo.
```

If you check on the status of the RDQM with the command `rdqmstatus -m RDQM1` you should see something like:

```
Node:                                  
ip-10-0-1-238.us-west-2.compute.internal
Queue manager status:                   Running
CPU:                                    0.17
Memory:                                 169MB
Queue manager file system:              58MB used, 2.9GB allocated [2%]
HA role:                                Primary
HA status:                              Synchronization in progress
HA control:                             Enabled
HA current location:                    This node
HA preferred location:                  This node
HA floating IP interface:               None
HA floating IP address:                 None

Node:                                  
ip-10-0-2-244.us-west-2.compute.internal
HA status:                              Synchronization in progress
HA synchronization progress:            6.3%
HA estimated time to completion:        2018-01-15 16:14:23

Node:                                  
ip-10-0-3-152.us-west-2.compute.internal
HA status:                              Synchronization in progress
HA synchronization progress:            6.4%
HA estimated time to completion:        2018-01-15 16:14:22
Command '/opt/mqm/bin/rdqmstatus' run with sudo.
```

Once the initial synchronization has completed the command should produce something like:

```
Node:                                  
ip-10-0-1-238.us-west-2.compute.internal
Queue manager status:                   Running
CPU:                                    0.00
Memory:                                 169MB
Queue manager file system:              58MB used, 2.9GB allocated [2%]
HA role:                                Primary
HA status:                              Normal
HA control:                             Enabled
HA current location:                    This node
HA preferred location:                  This node
HA floating IP interface:               None
HA floating IP address:                 None

Node:                                  
ip-10-0-2-244.us-west-2.compute.internal
HA status:                              Normal

Node:                                  
ip-10-0-3-152.us-west-2.compute.internal
HA status:                              Normal
Command '/opt/mqm/bin/rdqmstatus' run with sudo.
```

Congratulations, you have got a running RDQM.

### Deleting the stack

You can delete the stack with the `deleteStack` script which takes the same arguments as the `describeStack` script.

## Summary

This sample has shown you how to deploy RDQM to AWS.

Future enhancements will include things like setting up a bastion host and an ElasticLoadBalancer.
