# FreeBSD make reads this file instead of Makefile.
# The project Makefile is GNU make syntax; hand every target to gmake.

GMAKE!= command -v gmake || true

.if empty(GMAKE)
.error This project requires GNU make. Install the gmake package, then run make again.
.endif

.MAIN: all

all:
	${GMAKE} ${.TARGET}

.DEFAULT:
	${GMAKE} ${.TARGET}
