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
#
# This script removes the Azure configuration specific to one queue manager:
# the load balancer configuration (probe and rule)
# the network security group rule

Using Module Az

param (
    [Parameter(Mandatory = $true)] [string] $LoadBalancerName,
    [Parameter(Mandatory = $true)] [string] $ResourceGroupName,
    [Parameter(Mandatory = $true)] [string] $NetworkSecurityGroupName,
    [Parameter(Mandatory = $true)] [string] $QueueManagerName
)

. ./functions.ps1

try {
    $initial_lb = Get-LoadBalancer -LoadBalancerName $LoadBalancerName -ResourceGroupName $ResourceGroupName
    Remove-LoadBalancerConfig -LoadBalancer $initial_lb -QueueManagerName $QueueManagerName
    Write-Information "Load Balancer Configuration removed"
}
catch {
    Write-Error $_
}
try {
    $initial_nsg = Get-NetworkSecurityGroup -NetworkSecurityGroupName $NetworkSecurityGroupName -ResourceGroupName $ResourceGroupName
    Remove-NetworkSecurityGroupConfig -NetworkSecurityGroup $initial_nsg -QueueManagerName $QueueManagerName
    Write-Information "Network Security Group Configuration removed"
}
catch {
    Write-Error $_
}