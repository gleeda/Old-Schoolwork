
help:
	cat README

all:	permeate

permeate:	permeate.c
	gcc -g -o permeate permeate.c -lipq

tests:
	cd testing; \
	gcc -g -o myserver myserver.c; \
	gcc -g -o myclient myclient.c

perf:
	cd testing; \
	gcc -g -o perf perf.c -lipq

clean:
	rm -f permeate

tidy:
	rm -f core.* *~ #*
