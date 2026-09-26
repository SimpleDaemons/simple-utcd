# Changelog

All notable changes to simple-utcd are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/) (see [VERSIONING.md](VERSIONING.md)).

## [1.0.0] — 2026-09-26

### Added
- RFC 868 time service on TCP and UDP port 37, using seconds since 1900-01-01 UTC
- IPv4 listeners, and IPv6 listeners when `enable_ipv6` is enabled
- Allow/deny lists with IPv4 CIDR matching, and per-client rate limits
- Privilege drop via `run_as_user` / `run_as_group` after the listen sockets are bound
- CLI: `-c`/`--config`, `--config-test`, `--help`, `--version`
- Log lines substitute `{}` arguments

### Changed
- The daemon serves the host clock. It does not speak NTP
- systemd unit is `Type=simple` and starts `/usr/bin/simple-utcd -c …` as `simple-utcd` with `CAP_NET_BIND_SERVICE`
- One product only. Enterprise and datacenter entry points are removed

## [0.1.0]

### Notes
- Initial tree: config loader, packaging, and a TCP-only listener that sent Unix-epoch timestamps
