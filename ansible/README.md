# Ansible playbooks for RDQM

This directory contains:

1. an ansible playbook `rdqm.yaml` to configure three systems ready to become an RDQM HA Group

2. an ansible playbook `client.yaml` to configure one or more systems as MQ client systems to use an RDQM HA queue manager on the HA Group

## hosts.ini

Before running either playbook, you will need to update the hosts.ini file to reflect the systems
you wish to configure, both RDQM and client.

## General variables

The following sections describe each of the variables defined in the file `group_vars/all/vars.yaml`

### mq_release

The MQ release you are installing.
The value of this variable is used to create a directory to hold the unpacked image.

### mq_client_image

The full path to the file containing the MQ client product to be installed.
This file is copied to each client system and unpacked.

### mq_server_image

The full path to the file containing the MQ server product to be installed.
This file is copied to each of the RDQM nodes and unpacked.

### mqm_password

The encrypted password used temporarily while configuring passwordless ssh for the mqm user.

### mquser_password

The encrypted password for the mquser account on all nodes.

### rdqmadmin_password

The encrypted password for the rdqmadmin account on the RDQM nodes.

### timezone

The timezone desired for each of the nodes.

### DRBD_device

The device which should be used to create a volume group for DRBD/RDQM.

## el8 Variables

The following variables are only used in the el8 role so they are defined in roles/rdqm-el8/vars/main.yml

### appstream_repo_file

The file that contains the definition of the standard RHEL 8 AppStream repository.

### appstream_repo_id

The ID of the standard RHEL 8 AppStream repository.

## RDQM Playbook

Once the `hosts.ini` file has been updated and any variables, the RDQM playbook can be run with the command:
```
ansible-playbook -l rdqm rdqm.yml
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

## Client Playbook

Once the `hosts.ini` file has been updated and any variables, the client playbook can be run with the command:
```
ansible-playbook -l client client.yml
```
## Configure client system

### Accept MQ License

The first thing to do on the client system is to accept the MQ license by running as root:
```
/opt/mqm/bin/mqlicense -accept
```

### Internet Access

In order to be able to clone the ibm-messaging/mq-rdqm repository to get the samples, it is necessary to have Internet access from the client system(s).

### Make samples

Once I had a floating IP, I went to the client system and did:
```
su - mquser
mkdir -p github.com/ibm-messaging
cd github.com/ibm-messaging
git clone https://github.com/ibm-messaging/mq-rdqm.git
cd mq-rdqm/samples/C/linux
make
cd
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
## Testing

The approach I usually use to test a new deployment is described [here](Testing.md)
