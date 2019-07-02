# Deploying IBM(R) MQ Advanced RDQM on AWS

This version of the sample shows how to deploy the Replicated Data Queue Manager
(RDQM) support that is part of IBM MQ Advanced, in a way that is closer to how a production deployment
would be done.

This version of the sample has several major enhancements:
1. It uses an AutoScalingGroup with a LaunchConfiguration in each Availability Zone to automate the creation of each instance, including replacing an instance automatically if one fails
2. It uses a separate NetworkInterface for each Instance that is dedicated to RDQM
3. It creates two LoadBalancers: one for public traffic and one for private traffic

The updated CloudFormation Template is written to be deployed into an environment with an existing VPC and bastion hosts so these are no longer created. You will need to supply information about the subnets etc. when you deploy the template.

As three Instances have to be created you will probably want to create your own AMI so that you can do most of the configuration, including installing MQ, only once.

Administration and configuration of RDQM is easiest if the mqm user has some specific sudo privileges and can ssh between the three instances without a password.

The description of creating the AMI below includes setting up the sudo access. Setting up the passwordless ssh is done once the instances have been created.

## Creating a new AMI

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

## Deploy the template

When you deploy the template you will be prompted for a lot of information, including the AMI to use where you should specify the ID of the AMI you created.

I suggest you update the defaults in your copy of the template to avoid having to enter the same information each time you try deploying the template.

## Create an RDQM

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
