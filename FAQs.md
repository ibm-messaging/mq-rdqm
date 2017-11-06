This page contains Frequently Asked Questions about RDQM.

# Can I control when an RDQM moves back to its preferred node following a failure?

If an RDQM fails over to a node other than its preferred node, by default it will move back to its preferred node when it can. If you would prefer to initiate the move back explicitly you should either

1. remove the preferred location from the RDQM once it has been created, or once it has failed over to another node
2. once it has failed over to another node, set the preferred location of the RDQM to that node