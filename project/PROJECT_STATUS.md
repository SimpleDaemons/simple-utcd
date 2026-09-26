# simple-utcd — Project Status

**Version:** 1.0.0  
**License:** Apache-2.0  
**Role:** RFC 868 time server (TCP and UDP port 37)

There is one program. Enterprise and datacenter editions are not built.

## What 1.0.0 does

- Answers RFC 868 with a 32-bit big-endian count of seconds since 1900-01-01 00:00:00 UTC
- Listens on TCP and UDP. IPv4 is required; IPv6 is used when `enable_ipv6` is true and the address is unspecified or IPv6
- Serves the host clock. Keep the host synced with chrony, systemd-timesyncd, or simple-ntpd
- Allow and deny lists, including IPv4 CIDR, plus a per-client token-bucket rate limit
- Drops root to `run_as_user` after bind when started as root
- CLI: `-c` / `--config`, `--config-test`, `--help`, `--version`, and SIGHUP reload

## Out of scope

NTP, NTS, PTP, leap-second indicators, upstream time sync inside this daemon, a web UI, and SNMP. Those are not part of RFC 868. simple-ntpd is the NTP daemon.

## Ops

systemd unit: `deployment/systemd/simple-utcd.service` (`Type=simple`, `User=simple-utcd`, `CAP_NET_BIND_SERVICE`). The daemon binds the sockets itself.
