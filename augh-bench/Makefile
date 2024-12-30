
SUBDIRS = $(shell ls -pA | grep /)

.PHONY: all git-archive vectors

all:
	echo "No target selected."

git-archive:
	git archive -o augh-bench.tar.gz master

vectors:
	for i in $(SUBDIRS); do \
		$(MAKE) -C $$i vectors; \
	done;

