# Ansible playbooks for RDQM

This directory contains:

1. an ansible playbook `configure_rdqm.yaml` to configure three systems ready to become an RDQM HA Group

2. an ansible playbook `configure_client.yaml` to configure one or more systems as MQ client systems to use an RDQM HA queue manager on the HA Group

## hosts.ini

Before running either playbook, you will need to update the hosts.ini file to reflect the systems
you wish to configure, both RDQM and client.

## Variables

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

### appstream_repo_file

The file that contains the definition of the standard RHEL 8 AppStream repository.

### DRBD_device

The ID of the standard RHEL 8 AppStream repository.

## RDQM Playbook

Once the `hosts.ini` file has been updated and any variables, the RDQM playbook can be run with the command:
```
ansible-playbook -l rdqm configure_rdqm.yaml
```

## Client Playbook

Once the `hosts.ini` file has been updated and any variables, the client playbook can be run with the command:
```
ansible-playbook -l client configure_client.yaml
```

## Testing

The approach I usually use to test a new deployment is described [here](Testing.md)
