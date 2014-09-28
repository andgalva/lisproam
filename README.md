lisproam
========
LISP (Locator/ID Separation Protocol) is an experimental protocol developed by Cisco Systems (http://lisp.cisco.com/).
LISP decouples the identity and the location of a host, assigning a different address for each.
In this way, as the host changes its location (e.g. attaches to another WiFi network) it will maintain the same EID (Endpoint Identificator), changing only its RLOC (Routing locator).

LISP is being used for multiple case scenarios. In this work, I'm leveraging the EID/RLOC decouplement to provide host mobility.

LISP-ROAM is the result of my Master's Thesis "Support for Network-based User Mobility with LISP" (http://bit.ly/thesis-galvani).

LISP-ROAM is a network-based implementation of LISP-MN (https://datatracker.ietf.org/doc/draft-meyer-lisp-mn/).
LISP-MN is a software that modifies the TCP/IP of the host, allowing it to change attaching point (e.g. switch WiFi to 4G) without dropping its active connections. LISP-ROAM is a solution meant to be implemented on the edge network, leaving the host untouched and its network stack unmodified.
Hosts can establish and maintain connections with LISP-ROAM-enabled sites, while moving across LISP-ROAM-enabled networks with little impact on the network experience.

Further info can be found in my ACM publication: "LISP-ROAM: Network-based Host Mobility with LISP" (http://dl.acm.org/citation.cfm?doid=2645892.2645898)

The software implementation of LISP-ROAM is a modified version of LISPmob 0.3.2 (http://lispmob.org/).

References
==========
* LISP (Locator/ID Separation Protocol) http://lisp.cisco.com/
* LISP-ROAM: network-based host mobility with LISP (Publication) http://dl.acm.org/citation.cfm?doid=2645892.2645898
* Support for Network-based User Mobility with LISP (Master's Thesis) http://bit.ly/thesis-galvani
* LISPmob http://lispmob.org/
* LISP-MN https://datatracker.ietf.org/doc/draft-meyer-lisp-mn/


