# Building a custom Azure image for RDQM

This page describes how to build a custom Image for RDQM.

For this approach I used the Azure CLI in the Azure Cloud Shell (Bash) to create the VM and the Image.

I will give the exact commands I used, apart from my SSH public key.

For my DR/HA deployment I used the uksouth and northeurope regions so I created an image in each.

The steps described here are for the uksouth image. I used a single resource group for both and for the other commands,
wherever uksouth was used for the first image I substituted northeurope for the second.

## Resource Group

I created a separate resource group for the resources relating to the image:

```bash
az group create --name rg-jc-rdqm-image --location uksouth
```

## Master VM

I created a VM that I will use as the basis of the image:

```bash
az vm create --name vm-jc-rdqm-master-uksouth --resource-group rg-jc-rdqm-image --image RedHat:RHEL:7.7:7.7.2020020415 --location uksouth --ssh-key-values "<your ssh public key>"
```

Note that I have specified a RHEL 7.7 image. The default RHEL image is now 7.8 which is not supported by MQ 9.1.5 so I need
to ensure that RHEL 7.7 is used.

You will need the value of "publicIpAddress" to be able to ssh to the VM.

The first thing to do is to log in and check /etc/redhat-release, which should be `Red Hat Enterprise Linux Server release 7.7 (Maipo)`.

## Separate Disk for DRBD

```bash
az vm disk attach --vm-name vm-jc-rdqm-master-uksouth --name vm-jc-rdqm-master-uksouth_DrbdDisk --new --resource-group rg-jc-rdqm-image --size-gb 128
```

I specified 128 GB, you should choose a size that will meet your needs, allowing for the extra space needed for DR snapshots.

Check the block devices by running the `lsblk` command. You should see something similar to:

```bash
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
sdc                 8:32   0  128G  0 disk
sr0                11:0    1  628K  0 rom  
```

Here sdc is the additional disk added so create the drbdpool volume group using that:

```bash
sudo vgcreate drbdpool /dev/sdc
```

## Changes to Kernel Configuration

There are some changes to the Linux kernel configuration necessary for MQ and Azure.

```bash
sudo -s
echo 'kernel.sem = 32 4096 32 128' >> /etc/sysctl.conf
echo 'kernel.threads-max = 32768' >> /etc/sysctl.conf
echo 'fs.file-max = 524288' >> /etc/sysctl.conf
echo "net.ipv4.tcp_timestamps = 0" >> /etc/sysctl.conf
sysctl -p
echo '* - nofile 10240' >> /etc/security/limits.conf
echo 'root - nofile 10240' >> /etc/security/limits.conf
exit
```

## Install IBM MQ Advanced 9.1.5

I uploaded the MQ Advanced 9.1.5 image `IBM_MQ_9.1.5_LINUX_X86-64.tar.gz` to the master VM,
to the home directory of the default user, in my case john.

I then did:

```bash
sudo -i
mv /home/john/IBM_MQ_9.1.5_LINUX_X86-64.tar.gz .
tar -xzf IBM_MQ_9.1.5_LINUX_X86-64.tar.gz
cd MQServer
./mqlicense.sh -accept
cd Advanced/RDQM
./installRDQMsupport
```

I made this new installation the primary:

```bash
/opt/mqm/bin/setmqinst -i -p /opt/mqm
```

## New User Accounts

I created two new user accounts:

1. rdqmadmin - this user is a member of the mqm group and is the user that issues all MQ commands
2. rdqmuser - this user is not in the mqm group and is the user that applications use to access MQ

The commands I used were:

```bash
useradd -g mqm -G haclient rdqmadmin
passwd rdqmadmin
useradd rdqmuser
passwd rdqmuser
```

You will need to enter the password for rdqmuser when running the sample applications (unless you disable MQ authentication).

```bash
echo '. /opt/mqm/bin/setmqenv -s' >> /home/rdqmadmin/.bash_profile
```

I then checked that I could run dspmqver as rdqmadmin:

```bash
su - rdqmadmin
dspmqver
exit
```

## Sudo access

I granted the necessary sudo access to the mqm user by:

```bash
echo 'mqm ALL=(root) NOPASSWD: /opt/mqm/bin/crtmqm, /opt/mqm/bin/dltmqm, /opt/mqm/bin/rdqmadm, /opt/mqm/bin/rdqmstatus, /opt/mqm/bin/rdqmdr' > /etc/sudoers.d/mqm
```

## SELinux

To configure SELinux I did:

```bash
yum -y install policycoreutils-python
semanage permissive -a drbd_t
```

## Firewall

I ran the supplied script to open the standard ports for RDQM:

```bash
/opt/mqm/samp/rdqm/firewalld/configure.sh
```

This only opens port 1414 for MQ traffic so if you know a fixed range of ports you will need you could edit the mq service before running this script, or open additional ports later.

It is also necessary to open port 3001 for the health probe:

```bash
firewall-cmd --zone=public --add-port=3001/tcp --permanent
```

## Passwordless ssh

It is possible to do almost all of the configuration of passwordless ssh in the image, as long as you are happy for any instance to be able to ssh to any other instance, even in a different HA Group.

The steps are:

```bash
usermod -d /home/mqm mqm
mkhomedir_helper mqm
su mqm
ssh-keygen -t rsa -f /home/mqm/.ssh/id_rsa -N ''
cp /home/mqm/.ssh/id_rsa.pub /home/mqm/.ssh/authorized_keys
exit
```

## Health Probe

The final step is to set up the MQ health probe. This is a simple node.js wrapper around a script that calls ```dspmq``` to see if a specific queue manager is running on the node.

The first thing to do is to install node.js and configure the rdqmadmin user to use it, primarily in case you have to debug the health probe.

```bash
yum -y install rh-nodejs8
echo 'source scl_source enable rh-nodejs8' >> /home/rdqmadmin/.bashrc
```

Finally, copy some files from the GitHub repository and install them as necessary. The following files are needed:

1. configure.mqsc
2. mq_probe.service
3. mqazurehealthprobe.js
4. mqazureqmstatus

```bash
mv /home/john/configure.mqsc /home/rdqmadmin
chown rdqmadmin:mqm /home/rdqmadmin/configure.mqsc
mv /home/john/mqazurehealthprobe.js /home/rdqmadmin
chown rdqmadmin:mqm /home/rdqmadmin/mqazurehealthprobe.js
mv /home/john/mqazureqmstatus /home/rdqmadmin
chown rdqmadmin:mqm /home/rdqmadmin/mqazureqmstatus
mv /home/john/mq_probe.service /lib/systemd/system
chown root:root /lib/systemd/system/mq_probe.service
```

## Create the Image

To create the image, as root in the VM:

```bash
waagent -force -deprovision
exit
exit
```

Then, in the Azure Cloud Shell:

```bash
az vm deallocate --resource-group rg-jc-rdqm-image --name vm-jc-rdqm-master-uksouth
az vm generalize --resource-group rg-jc-rdqm-image --name vm-jc-rdqm-master-uksouth
az image create --resource-group rg-jc-rdqm-image --name image-rdqm-uksouth --source vm-jc-rdqm-master-uksouth
```

You will need the id of the image to put into the parameters file for the template.

## Second region

If you are doing a full DR/HA deployment involving two Azure regions then you will also need an image in the second region.
I went through the same process in the northeurope region to create an image image-rdqm-northeurope.
