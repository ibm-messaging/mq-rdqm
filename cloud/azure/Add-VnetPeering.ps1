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

param (
    [Parameter(Mandatory = $true)] [string] $ResourceGroupName,
    [Parameter(Mandatory = $true)] [string] $Vnet1Name,
    [Parameter(Mandatory = $true)] [string] $Vnet2Name
)

# PeerVnets.ps1 - peer a pair of vnets to allow RDQM DR communication between regions

try {
    $vnet1 = Get-AzVirtualNetwork -Name $Vnet1Name -ResourceGroupName $ResourceGroupName
    $vnet2 = Get-AzVirtualNetwork -Name $Vnet2Name -ResourceGroupName $ResourceGroupName
    $peer1 = Add-AzVirtualNetworkPeering -Name Vnet1ToVnet2 -VirtualNetwork $vnet1 -RemoteVirtualNetworkId $vnet2.Id
    $peer2 = Add-AzVirtualNetworkPeering -Name Vnet2ToVnet1 -VirtualNetwork $vnet2 -RemoteVirtualNetworkId $vnet1.Id
    Write-Information "Vnet peering established"
}
catch {
    Write-Error $_
}
