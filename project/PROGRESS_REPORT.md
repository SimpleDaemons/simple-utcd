# simple-utcd — Progress Report

**Version:** 1.0.0  
**Date:** September 2026

1.0.0 is an RFC 868 server. Earlier status write-ups that called 0.3.2 complete, or that described separate Enterprise and Datacenter products, do not match the code and are withdrawn.

Shipped on the request path: TCP and UDP listeners, RFC 868 epoch, allow/deny (IPv4 CIDR), rate limiting, privilege drop, config reload, and a CLI with `--config-test`.

Not on the request path: the authentication, TLS, DDoS, and upstream-manager classes. They are library code with unit tests. RFC 868 has no authentication or TLS, and this daemon does not sync from upstream servers.
