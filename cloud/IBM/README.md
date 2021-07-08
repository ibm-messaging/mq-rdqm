# RDQM in the IBM Cloud

This README describes how I created four VSIs to use with the ansible playbooks to install an RDQM HA Group and a client system
suitable for testing a queue manager in the RDQM HA Group.

## Initial Conditions

I have a bastion host which I will use to run the ansible playbooks so I already have a VPC
and a subnet in the London 2 zone which is where the bastion is located.

I also have a resource group that I use for the RDQM deployment.

I also have subnets created in London 1 and London 3 from previous deployments of RDQM in
London.

I created an ssh key pair on the bastion and I registered the public key in the IBM Cloud console so that I can use that when creating VSIs.

I create the Virtual Server Images and the Storage Volumes each time I deploy RDQM.

## Creating a new deployment of RDQM

These steps are what I do to create a new deployment of RDQM in London.

I create one VSI in each of the London Zones for maximum availability.

### colgrave-vsi-rdqm-eu-gb-1

In the IBM Cloud Dashboard I clicked the blue "Create resource" button.

I searched for "virtual server for VPC" and went to the "New virtual server for VPC" page.

I entered a name of colgrave-vsi-rdqm-eu-gb-1

I changed the resource group name to my resource group name.

I changed the location to London 1

I changed the operating system image to Red Hat Enterprise Linux

I selected the key from my bastion as the SSH key.

The network interface eth0 was already attached to my subnet in the London 1 zone.

I clicked the blue "Create virtual server instance" button.

I noted the Private IP as I will need to update the ansible hosts.ini file with that.

### colgrave-vsi-rdqm-eu-gb-2

I did the same steps for another VSI named colgrave-vsi-rdqm-eu-gb-2 except that the location for this instance is London 2

### colgrave-vsi-rdqm-eu-gb-3

I did the same steps for another VSI named colgrave-vsi-rdqm-eu-gb-3 except that the location for this instance is London 3

### Adding a storage volume for RDQM

The easiest way to manage the storage for RDQM is to add another disk/device/volume dedicated to RDQM.

In the list of Virtual server instances for VPC I clicked on colgrave-vsi-rdqm-eu-gb-1

In the Storage volumes section I clicked the blue Attach button.

Under "Block Volumes" I chose "Create a data volume"

I entered a name of colgrave-block-rdqm-eu-gb-1

I changed the resource group to my resource group.

I entered a size of 100

I clicked the blue Save button

I attached a new volume colgrave-block-rdqm-eu-gb-2 to colgrave-vsi-rdqm-eu-gb-2 and a third volume colgrave-block-rdqm-eu-gb-3 to colgrave-vsi-rdqm-eu-gb-3

### Creating a client VSI

The last thing to do is to create a VSI that will be an MQ client used to test the RDQM deployment.

I created a fourth VIS named colgrave-vsi-client-eu-gb-1 in London 1 and using the default CentOS operating system image.

## Running the rdqm ansible playbook

Before running the ansible playbook to configure the three RDQM nodes, you need to edit hosts.ini and set the host names and private IP addresses for the nodes.

You may also have to edit one or more of the variables in group_vars/all/vars.yaml

Once everything is ready, run the rcqm playbook with:
```
ansible-playbook -l rdqm configure_rdqm.yaml
```

You should see a `PLAY RECAP` something like:
```
PLAY RECAP ****************************************************************************************************************************************
colgrave-vsi-rdqm-eu-gb-1  : ok=36   changed=20   unreachable=0    failed=0    skipped=10   rescued=0    ignored=0
colgrave-vsi-rdqm-eu-gb-2  : ok=34   changed=18   unreachable=0    failed=0    skipped=12   rescued=0    ignored=0
colgrave-vsi-rdqm-eu-gb-3  : ok=34   changed=18   unreachable=0    failed=0    skipped=12   rescued=0    ignored=0
```

## Configure RDQM HA Group

The steps to configure the RDQM HA Group are:

1. on each node, as root run `/opt/mqm/bin/mqlicense -accept` which should produce:
```
5724-H72 (C) Copyright IBM Corp. 1994, 2021.
License agreement accepted. Run the dspmqlic command to view the MQ
license agreement.
```
2. on the first node, run:
```
su - rdqmadmin
rdqmadm -c
```
which should produce something like:
```
Configuring the replicated data subsystem.
The replicated data subsystem has been configured on this node.
The replicated data subsystem has been configured on
'colgrave-vsi-rdqm-eu-gb-2'.
The replicated data subsystem has been configured on
'colgrave-vsi-rdqm-eu-gb-3'.
The replicated data subsystem configuration is complete.
Command '/opt/mqm/bin/rdqmadm' run with sudo.
```

You can check the status by running `rdqmstatus -n` which should produce something like:
```
Node colgrave-vsi-rdqm-eu-gb-1 is online
Node colgrave-vsi-rdqm-eu-gb-2 is online
Node colgrave-vsi-rdqm-eu-gb-3 is online
```

## Create RDQM HA Queue Manager

As rdqmadmin on the first rdqm node, run `crtmqm -p 1414 -sx QM1` which should produce something like:
```
Creating replicated data queue manager configuration.
Secondary queue manager created on 'colgrave-vsi-rdqm-eu-gb-2'.
Secondary queue manager created on 'colgrave-vsi-rdqm-eu-gb-3'.
IBM MQ queue manager created.
Directory '/var/mqm/vols/qm1/qmgr/qm1' created.
The queue manager is associated with installation 'Installation1'.
Creating or replacing default objects for queue manager 'QM1'.
Default objects statistics : 84 created. 0 replaced. 0 failed.
Completing setup.
Setup completed.
Enabling replicated data queue manager.
Replicated data queue manager enabled.
Command '/opt/mqm/bin/crtmqm' run with sudo.
```

You can check the status of this queue manager by running `rdqmstatus -m QM1` which should produce something like:
```
Node:                                   colgrave-vsi-rdqm-eu-gb-1
Queue manager status:                   Running
CPU:                                    0.00%
Memory:                                 136MB
Queue manager file system:              58MB used, 2.9GB allocated [2%]
HA role:                                Primary
HA status:                              Normal
HA control:                             Enabled
HA current location:                    This node
HA preferred location:                  This node
HA blocked location:                    None
HA floating IP interface:               None
HA floating IP address:                 None

Node:                                   colgrave-vsi-rdqm-eu-gb-2
HA status:                              Normal

Node:                                   colgrave-vsi-rdqm-eu-gb-3
HA status:                              Normal
```

## Configure client system

To configure the client system, update the hosts.ini file with the correct name and IP address of the client system, or systems if you wish to configure multiple client systems.

Then run the client playbook with `ansible-playbook -l client configure_client.yaml` which should produce a recap something like:
```
PLAY RECAP ****************************************************************************************************************************************
colgrave-vsi-client-eu-gb-1 : ok=13   changed=12   unreachable=0    failed=0    skipped=0    rescued=0    ignored=0
```

### assign a Floating IP

In order to be able to clone the ibm-messaging/mq-rdqm repository to get the samples, it is necessary to assign a floating IP address to the client instance(s). I edited the eth0 network interface of my client and reserved a new floating IP as I had none available.

### Make samples

Once I had a floating IP, I went to the client system and did:
```
su - mquser
mkdir -p github.com/ibm-messaging
cd github.com/ibm-messaging
git clone https://github.com/ibm-messaging/mq-rdqm.git
cd mq-rdqm/samples/C/linux
make
```

The output of the make should be something like:
```
gcc -m64 -I /opt/mqm/inc -c ../complete.c
gcc -m64 -I /opt/mqm/inc -c ../connection.c
gcc -m64 -I /opt/mqm/inc -c ../globals.c
gcc -m64 -I /opt/mqm/inc -c ../log.c
gcc -m64 -I /opt/mqm/inc -c ../options.c
gcc -m64 -I /opt/mqm/inc -o rdqmget ../rdqmget.c complete.o connection.o globals.o log.o options.o -L /opt/mqm/lib64 -l mqic_r -Wl,-rpath=/opt/mqm/lib64 -Wl,-rpath=/usr/lib64
gcc -m64 -I /opt/mqm/inc -o rdqmput ../rdqmput.c complete.o connection.o globals.o log.o options.o -L /opt/mqm/lib64 -l mqic_r -Wl,-rpath=/opt/mqm/lib64 -Wl,-rpath=/usr/lib64
```

You should now be ready to test the RDQM deployment, following the steps [here](../../../ansible/Testing.md)