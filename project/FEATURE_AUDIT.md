# simple-utcd feature audit

**Version:** 1.0.0  
**Date:** September 2026

| On the request path | Status |
|---------------------|--------|
| RFC 868 4-byte timestamp (seconds since 1900) | Served |
| TCP accept, send, close | Served |
| UDP recvfrom / sendto | Served |
| IPv4 bind | Served |
| IPv6 bind when enabled | Served; IPv4 still comes up if IPv6 does not |
| Allow/deny, including IPv4 CIDR | Served |
| Per-client rate limit | Served |
| Drop root after bind | Served when `run_as_user` is set |
| Config reload on SIGHUP | Served (listen sockets stay as they were) |

Authentication, TLS, DDoS scoring, and upstream failover exist as separate classes and are not called by the server. RFC 868 does not use them.
