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

function Get-LoadBalancer {
    [OutputType([Microsoft.Azure.Commands.Network.Models.PSLoadBalancer])]
    param (
        [Parameter(Mandatory = $true)] [string] $LoadBalancerName,
        [Parameter(Mandatory = $true)] [string] $ResourceGroupName
    )

    $lb = $null
    try {
        $lb = Get-AzLoadBalancer -Name $LoadBalancerName -ResourceGroupName $ResourceGroupName -ErrorAction Stop
        if ($?) {
            if ($lb) {
                return $lb
            }
            else {
                throw "Get-AzLoadBalancer returned null"
            }
        }
        else {
            throw "Get-AzLoadBalancer failed"
        }
    }
    catch {
        throw $_
    }
}

function Get-NetworkSecurityGroup {
    [OutputType([Microsoft.Azure.Commands.Network.Models.PSNetworkSecurityGroup])]
    param (
        [Parameter(Mandatory = $true)] [string] $NetworkSecurityGroupName,
        [Parameter(Mandatory = $true)] [string] $ResourceGroupName
    )

    $nsg = $null
    try {
        $nsg = Get-AzNetworkSecurityGroup -Name $NetworkSecurityGroupName -ResourceGroupName $ResourceGroupName -ErrorAction Stop
        if ($?) {
            if ($nsg) {
                return $nsg
            }
            else {
                throw "Get-AzNetworkSecurityGroup returned null"
            }
        }
        else {
            throw "Get-AzNetworkSecurityGroup failed"
        }
    }
    catch {
        throw $_
    }
}

function Add-LoadBalancerConfig {
    [OutputType([System.Void])]
    param (
        [Parameter(Mandatory = $true)] [Microsoft.Azure.Commands.Network.Models.PSLoadBalancer] $LoadBalancer,
        [Parameter(Mandatory = $true)] [string] $QueueManagerName,
        [Parameter(Mandatory = $true)] [int] $ProbePort,
        [Parameter(Mandatory = $true)] [int] $FrontendPort,
        [Parameter(Mandatory = $true)] [int] $BackendPort    
    )

    $probe_name = "probe-$QueueManagerName"
    $probe_path = "/$QueueManagerName"
    $lb_rule_name = "rule-$QueueManagerName"
    $new_lb = Add-AzLoadBalancerProbeConfig -LoadBalancer $LoadBalancer -Name $probe_name -Protocol 'Http' -Port $ProbePort -IntervalInSeconds 15 -ProbeCount 2 -RequestPath $probe_path
    $current_lb = $new_lb
    # The following is necessary to link the new probe and the corresponding rule
    $new_lb = Set-AzLoadBalancer -LoadBalancer $current_lb
    $current_lb = $new_lb
    $new_probe = Get-AzLoadBalancerProbeConfig -Name $probe_name -LoadBalancer $current_lb
    $new_lb = Add-AzLoadBalancerRuleConfig -LoadBalancer $current_lb -Name $lb_rule_name -Protocol "Tcp" -FrontendPort $FrontEndPort -BackendPort $BackendPort -FrontendIPConfiguration $current_lb.FrontendIpConfigurations[0] -BackendAddressPool $current_lb.BackendAddressPools[0] -Probe $new_probe
    $current_lb = $new_lb
    $new_lb = Set-AzLoadBalancer -LoadBalancer $current_lb
}

function Add-NetworkSecurityGroupConfig {
    [OutputType([System.Void])]
    param (
        [Parameter(Mandatory = $true)] [Microsoft.Azure.Commands.Network.Models.PSNetworkSecurityGroup] $NetworkSecurityGroup,
        [Parameter(Mandatory = $true)] [string] $QueueManagerName,
        [Parameter(Mandatory = $true)] [int] $BackendPort,
        [Parameter(Mandatory = $true)] [int] $SecurityRulePriority,
        [Parameter(Mandatory = $false)][int]$DrPort,      
        [Parameter(Mandatory = $false)][int]$DrRulePriority
    )

    $nsg_rule_name = "nsgrule-$QueueManagerName"
    $nsg_dr_rule_name = "nsgrule-dr-$QueueManagerName"
    $new_nsg = Add-AzNetworkSecurityRuleConfig -Name $nsg_rule_name -NetworkSecurityGroup $NetworkSecurityGroup -Access Allow -Protocol Tcp -Direction Inbound -Priority $SecurityRulePriority -SourceAddressPrefix "*" -SourcePortRange * -DestinationAddressPrefix * -DestinationPortRange $BackendPort
    $current_nsg = $new_nsg
    if ($PSBoundParameters.ContainsKey('DrPort')) {
        $new_nsg = Add-AzNetworkSecurityRuleConfig -Name $nsg_dr_rule_name -NetworkSecurityGroup $NetworkSecurityGroup -Access Allow -Protocol Tcp -Direction Inbound -Priority $DrRulePriority -SourceAddressPrefix "*" -SourcePortRange * -DestinationAddressPrefix * -DestinationPortRange $DrPort
    }
    $new_nsg = Set-AzNetworkSecurityGroup -NetworkSecurityGroup $current_nsg
}

function Remove-LoadBalancerConfig {
    [OutputType([System.Void])]
    param (
        [Parameter(Mandatory = $true)] [Microsoft.Azure.Commands.Network.Models.PSLoadBalancer] $LoadBalancer,
        [Parameter(Mandatory = $true)] [string] $QueueManagerName
    )
    $current_lb = $LoadBalancer
    $removed_probe = $false
    $removed_rule = $false
    try {
        $probe_name = "probe-$QueueManagerName"
        $new_lb = Remove-AzLoadBalancerProbeConfig -LoadBalancer $LoadBalancer -Name $probe_name
        $current_lb = $new_lb
        $removed_probe = $true
    }
    catch {
        Write-Error $_
    }
    try {
        $lb_rule_name = "rule-$QueueManagerName"
        $new_lb = Remove-AzLoadBalancerRuleConfig -LoadBalancer $current_lb -Name $lb_rule_name
        $current_lb = $new_lb
        $removed_rule = $true
    }
    catch {
        Write-Error $_
    }
    if ($removed_probe -or $removed_rule) {
        $new_lb = Set-AzLoadBalancer -LoadBalancer $current_lb
    }
}

function Remove-NetworkSecurityGroupConfig {
    [OutputType([System.Void])]
    param (
        [Parameter(Mandatory = $true)] [Microsoft.Azure.Commands.Network.Models.PSNetworkSecurityGroup] $NetworkSecurityGroup,
        [Parameter(Mandatory = $true)] [string] $QueueManagerName
    )
    $nsg_rule_name = "nsgrule-$QueueManagerName"
    $new_nsg = Remove-AzNetworkSecurityRuleConfig -NetworkSecurityGroup $NetworkSecurityGroup -Name $nsg_rule_name
    $current_nsg = $new_nsg
    $nsg_dr_rule_name = "nsgrule-dr-$QueueManagerName"
    $new_nsg = Remove-AzNetworkSecurityRuleConfig -NetworkSecurityGroup $NetworkSecurityGroup -Name $nsg_dr_rule_name
    $current_nsg = $new_nsg
    $new_nsg = Set-AzNetworkSecurityGroup -NetworkSecurityGroup $current_nsg
}

function Set-LoadBalancerConfig {
    [OutputType([System.Void])]
    param (
        [Parameter(Mandatory = $true)] [Microsoft.Azure.Commands.Network.Models.PSLoadBalancer] $LoadBalancer,
        [Parameter(Mandatory = $true)] [string] $QueueManagerName,
        [Parameter(Mandatory = $true)] [int] $ProbePort,
        [Parameter(Mandatory = $true)] [int] $FrontendPort,
        [Parameter(Mandatory = $true)] [int] $BackendPort    
    )

    $probe_name = "probe-$QueueManagerName"
    $probe_path = "/$QueueManagerName"
    $lb_rule_name = "rule-$QueueManagerName"
    $new_lb = Set-AzLoadBalancerProbeConfig -LoadBalancer $LoadBalancer -Name $probe_name -Protocol 'Http' -Port $ProbePort -IntervalInSeconds 15 -ProbeCount 2 -RequestPath $probe_path
    $current_lb = $new_lb
    # The following is necessary to link the new probe and the corresponding rule
    $new_lb = Set-AzLoadBalancer -LoadBalancer $current_lb
    $current_lb = $new_lb
    $new_probe = Get-AzLoadBalancerProbeConfig -Name $probe_name -LoadBalancer $current_lb
    $new_lb = Set-AzLoadBalancerRuleConfig -LoadBalancer $current_lb -Name $lb_rule_name -Protocol "Tcp" -FrontendPort $FrontEndPort -BackendPort $BackendPort -FrontendIPConfiguration $current_lb.FrontendIpConfigurations[0] -BackendAddressPool $current_lb.BackendAddressPools[0] -Probe $new_probe
    $current_lb = $new_lb
    $new_lb = Set-AzLoadBalancer -LoadBalancer $current_lb
}

function Set-NetworkSecurityGroupConfig {
    [OutputType([System.Void])]
    param (
        [Parameter(Mandatory = $true)] [Microsoft.Azure.Commands.Network.Models.PSNetworkSecurityGroup] $NetworkSecurityGroup,
        [Parameter(Mandatory = $true)] [string] $QueueManagerName,
        [Parameter(Mandatory = $true)] [int] $BackendPort    
    )

    $nsg_rule_name = "nsgrule-$QueueManagerName"
    $new_nsg = Set-AzNetworkSecurityRuleConfig -Name $nsg_rule_name -NetworkSecurityGroup $NetworkSecurityGroup -Access Allow -Protocol Tcp -Direction Inbound -Priority 1234 -SourceAddressPrefix "*" -SourcePortRange * -DestinationAddressPrefix * -DestinationPortRange $BackendPort
    $current_nsg = $new_nsg
    $new_nsg = Set-AzNetworkSecurityGroup -NetworkSecurityGroup $current_nsg
}