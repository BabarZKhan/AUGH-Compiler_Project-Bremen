
SUBDIRS = $(shell ls -pA | grep /)

.PHONY: all vectors

all:
	echo "No target selected."

vectors:
	for i in $(SUBDIRS); do \
		$(MAKE) -C $$i vectors; \
	done;

