# Ansible playbooks for RDQM

This directory contains two playbooks relating to RDQM:

1. an ansible playbook `rdqm.yml` to configure three systems so that they are ready to become an RDQM HA Group

2. an ansible playbook `client.yml` to configure one or more systems as MQ client systems to use an RDQM HA queue manager on the HA Group

The systems must be running either RHEL 7 or RHEL 8.

You will need to update three files before using the rdqm.yml playbook to configure RHEL 7 systems:

1. hosts.ini
2. group_vars/all/vars.yaml
3. roles/rdqm-el/vars/main.yml

If you are configuring RHEL 8 systems then you will need to update an additional file:

1. roles/rdqm-el8/vars/main.yml

## hosts.ini

Before running either playbook, you will need to update the hosts.ini file to reflect the systems
you wish to configure, both RDQM and client.

There are two groups of hosts in the hosts.ini file:

1. rdqm
2. client

There must be exactly three hosts in the rdqm group for the rdqm playbook to run. For each host, two IP addresses must be specified:

1. ansible_host must be the address which ansible can use to connect to the host
2. rdqm_ha_replication must be the address of the host that is to be used in the generated rdqm.ini file

This approach allows for two different IP addresses to be specified for each host. In the future, I would like to support specifying one or two additional RDQM IP addresses to be specified so that all of the options for the rdqm.ini file can be supported, but for the moment the generated rdqm.ini file will only have one IP address per host. If you want to specify more, you can edit the generated rdqm.ini file before configuring the RDQM HA Group.

If you want to use the same IP address for both ansible and RDQM, at the moment the address has to be specified twice. I hope to remove the requirement for this in the future.

There must be at least one host specified in the client group but more is allowed, and each host will be configured in the same way.

## group_vars/all/vars.yaml

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

### mqm_gid

The group id to be used for the mqm group. The mqm group is created explicitly before MQ is installed.

### mqm_uid

The user id to be used for the mqm user. The mqm user is created explicitly before MQ is installed, to guarantee that the same value is used on all hosts.

### DRBD_device

The device which should be used to create a volume group for DRBD/RDQM.

## roles/rdqm-el/vars/main.yml

### rdqm_rpm

The .rpm file containing the MQ RDQM package.

## roles/rdqm-el8/vars/main.yml

The following variables are only used in the el8 role so they are defined in roles/rdqm-el8/vars/main.yml

### appstream_repo_file

The file that contains the definition of the standard RHEL 8 AppStream repository.

### appstream_repo_id

The ID of the standard RHEL 8 AppStream repository.

## RDQM Playbook

Once the `hosts.ini` file has been updated and any variables, the RDQM playbook can be run with the command:

```bash
ansible-playbook -l rdqm rdqm.yml
```

## Accept MQ license

On each of the rdqm hosts, as root, run `/opt/mqm/bin/mqlicense -accept` which should produce:

```bash
5724-H72 (C) Copyright IBM Corp. 1994, 2021.
License agreement accepted. Run the dspmqlic command to view the MQ
license agreement.
```

## Configure RDQM HA Group

On the first node, run:

```bash
su - rdqmadmin
rdqmadm -c
```

which should produce something like:

```bash
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

```bash
Node colgrave-vsi-rdqm-eu-gb-1 is online
Node colgrave-vsi-rdqm-eu-gb-2 is online
Node colgrave-vsi-rdqm-eu-gb-3 is online
```

## Create RDQM HA Queue Manager

As rdqmadmin on the first rdqm node, run `crtmqm -p 1414 -sx QM1` which should produce something like:

```bash
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

```bash
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

```bash
ansible-playbook -l client client.yml
```

### Accept MQ License

The first thing to do on the client system is to accept the MQ license by running as root:

```bash
/opt/mqm/bin/mqlicense -accept
```

### Internet Access

In order to be able to clone the ibm-messaging/mq-rdqm repository to get the samples, it is necessary to have Internet access from the client system(s).

### Make samples

Once I had a floating IP, I went to the client system and did:

```bash
su - mquser
mkdir -p github.com/ibm-messaging
cd github.com/ibm-messaging
git clone https://github.com/ibm-messaging/mq-rdqm.git
cd mq-rdqm/samples/C/linux
make
cd
```

The output of the make should be something like:

```bash
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
