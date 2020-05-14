# IBM MQ Advanced 9.1.5 RDQM DR/HA Deployment on Azure

This example shows how the integrated DR and HA support in [IBM MQ Advanced 9.1.5](https://www-01.ibm.com/common/ssi/ShowDoc.wss?docURL=/common/ssi/rep_ca/4/897/ENUS220-094/index.html) can be deployed to the Azure Public Cloud.

The approaches to HA described in the HA directory were steps in the development of this example and may be removed in the future.

The template included here is slightly simpler than the previous one in that only one network interface is used in each virtual machine.

The template can be used once to deploy a single RDQM HA Group if a full DR/HA configuration is not required.

A custom image for RDQM is used and for a full DR/HA deployment to two Azure regions, an image is required in each region.

An Azure load balancer is used in each region to direct MQ traffic to the appropriate virtual machine that is running an RDQM HA queue manager.

This example uses Azure [global virtual network peering](https://docs.microsoft.com/en-gb/azure/virtual-network/virtual-network-peering-overview) to connect an RDQM HA Group running in one Azure region with another RDQM HA Group running in a different Azure region.

The process I used to create the custom images for RDQM is described [here](Image.md)

The process I used to create a deployment of RDQM DR/HA using that image is described [here](Deployment.md)

The process I used to test the deployment is described [here](Test.md)
