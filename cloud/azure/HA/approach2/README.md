# Introduction

This second approach is closer to an approach that might be used in production to deploy the RDQM HA feature of IBM MQ Advanced to the Microsoft Azure public cloud.

The main differences of this approach compared to the first approach are:
1. A custom image is created so that most of the RHEL configuration and the installation of IBM MQ Advanced is only done once
2. A load balancer is used so that a queue manager can be accessed via a single IP address
3. IBM MQ Advanced 9.1.5 is used

# Terminology

* node
A system from the perspective of RDQM.
* VM
A system from the perspective of Azure.

The process I used to create a custom image for RDQM is described [here](Image.md)

The process I used to create a deployment of RDQM using that image is described [here](Deployment.md)

The process I used to test the deployment is described [here](Test.md)