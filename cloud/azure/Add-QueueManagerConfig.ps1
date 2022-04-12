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

Using Module Az

[CmdletBinding(DefaultParameterSetName = 'HA')]

param (
    [Parameter(ParameterSetName = 'DR', Mandatory = $true)]
    [Parameter(ParameterSetName = 'HA', Mandatory = $true)]
    [string] $LoadBalancerName,
    [Parameter(ParameterSetName = 'DR', Mandatory = $true)]
    [Parameter(ParameterSetName = 'HA', Mandatory = $true)]
    [string] $ResourceGroupName,
    [Parameter(ParameterSetName = 'DR', Mandatory = $true)]
    [Parameter(ParameterSetName = 'HA', Mandatory = $true)]
    [string] $QueueManagerName,
    [Parameter(ParameterSetName = 'DR', Mandatory = $true)]
    [Parameter(ParameterSetName = 'HA', Mandatory = $true)]
    [int] $ProbePort,
    [Parameter(ParameterSetName = 'DR', Mandatory = $true)]
    [Parameter(ParameterSetName = 'HA', Mandatory = $true)]
    [int] $FrontendPort,
    [Parameter(ParameterSetName = 'DR', Mandatory = $true)]
    [Parameter(ParameterSetName = 'HA', Mandatory = $true)]
    [int] $BackendPort,
    [Parameter(ParameterSetName = 'DR', Mandatory = $true)]
    [Parameter(ParameterSetName = 'HA', Mandatory = $true)]
    [string] $NetworkSecurityGroupName,
    [Parameter(ParameterSetName = 'DR', Mandatory = $true)]
    [Parameter(ParameterSetName = 'HA', Mandatory = $true)]
    [string] $SecurityRulePriority,
    [Parameter(ParameterSetName = 'DR', Mandatory = $true)]
    [int] $DrPort,      
    [Parameter(ParameterSetName = 'DR', Mandatory = $true)]
    [int] $DrRulePriority
)

. ./functions.ps1

# Add-QueueManagerConfig - adds the necessary Azure configurations for an RDQM HA queue manager. The configurations are:
# a load balancer health probe
# a load balancer rule
# network security group inbound rule (s)
# None of the existing configurations must exist, if any do then this script fails.
# The script Remove-QueueManagerConfig should be run to remove any existing configurations for the queue manager.

# If any of the three aspects of the configuration for a queue manager
# already exists then this command will fail and the user will have
# to run DeleteQueueManagerConfig

try {
    $initial_lb = Get-LoadBalancer -LoadBalancerName $LoadBalancerName -ResourceGroupName $ResourceGroupName
    $initial_nsg = Get-NetworkSecurityGroup -NetworkSecurityGroupName $NetworkSecurityGroupName -ResourceGroupName $ResourceGroupName
    Add-LoadBalancerConfig -LoadBalancer $initial_lb -QueueManagerName $QueueManagerName -ProbePort $ProbePort -FrontendPort $FrontendPort -BackendPort $BackendPort    
    Write-Information "Load Balancer configuration for queue manager $QueueManagerName added"
    if ($PSBoundParameters.ContainsKey('DrPort')) {
        Add-NetworkSecurityGroupConfig -NetworkSecurityGroup $initial_nsg -SecurityRulePriority $SecurityRulePriority -QueueManagerName $QueueManagerName -BackendPort $BackendPort -DrPort $DrPort -DrRulePriority $DrRulePriority
    }
    else {
        Add-NetworkSecurityGroupConfig -NetworkSecurityGroup $initial_nsg -SecurityRulePriority $SecurityRulePriority -QueueManagerName $QueueManagerName -BackendPort $BackendPort 
    }
    Write-Information "Network Security Group configuration for queue manager $QueueManagerName added"
}
catch {
    Write-Error $_
}