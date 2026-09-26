# Simple UTC Daemon — Project Overview

**Project:** `simple-utcd`  
**Version:** 1.0.0  
**Protocol / role:** RFC 868 time server (TCP and UDP port 37)  
**Status:** Production. Serves the host clock.

Keep the host clock synced with the operating system (chrony, systemd-timesyncd, or simple-ntpd). This daemon does not implement NTP.

## Where to look

| Document | Role |
|----------|------|
| [README.md](README.md) | How to build and run |
| [CHANGELOG.md](CHANGELOG.md) | Release history |
| [project/PROJECT_STATUS.md](project/PROJECT_STATUS.md) | What 1.0.0 includes |
| [RELEASING.md](RELEASING.md) | How to cut a release |

Portfolio-wide status lives in the monorepo `PROJECTS_OVERVIEW.md`.

*Last updated: September 2026*
