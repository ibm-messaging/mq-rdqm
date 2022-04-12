# (C) Copyright IBM Corporation 2020
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitions under the License.

# It would be nice to allow only the values to be changed to be specified,
# but for now require everything to be specified, as with Create.

Using Module Az

param (
    [Parameter(Mandatory = $true)] [string] $LoadBalancerName,
    [Parameter(Mandatory = $true)] [string] $ResourceGroupName,
    [Parameter(Mandatory = $true)] [string] $QueueManagerName,
    [Parameter(Mandatory = $true)] [int] $ProbePort,
    [Parameter(Mandatory = $true)] [int] $FrontendPort,
    [Parameter(Mandatory = $true)] [int] $BackendPort,
    [Parameter(Mandatory = $true)] [string] $NetworkSecurityGroupName
)

. ./functions.ps1

# Add-QueueManagerConfig - adds the necessary Azure configurations for an RDQM HA queue manager. The configurations are:
# a load balancer health probe
# a load balancer rule
# a network security group inbound rule
# None of the existing configurations must exist, if any do then this script fails.
# The script Remove-QueueManagerConfig should be run to remove any existing configurations for the queue manager.

# If any of the three aspects of the configuration for a queue manager
# already exists then this command will fail and the user will have
# to run DeleteQueueManagerConfig

try {
    $initial_lb = Get-LoadBalancer -LoadBalancerName $LoadBalancerName -ResourceGroupName $ResourceGroupName
    $initial_nsg = Get-NetworkSecurityGroup -NetworkSecurityGroupName $NetworkSecurityGroupName -ResourceGroupName $ResourceGroupName
    Set-LoadBalancerConfig -LoadBalancer $initial_lb -QueueManagerName $QueueManagerName -ProbePort $ProbePort -FrontendPort $FrontendPort -BackendPort $BackendPort    
    Set-NetworkSecurityGroupConfig -NetworkSecurityGroup $initial_nsg -QueueManagerName $QueueManagerName -BackendPort $BackendPort    
    Write-Information "All configuration for queue manager $QueueManagerName updated"
}
catch {
    Write-Error $_
}