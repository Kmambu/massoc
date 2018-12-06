SUBDIRS = a_minimips_simple b_minimips_simple soft

.PHONY:	all clean

all:
	for d in $(SUBDIRS); do $(MAKE) --directory $$d; done

clean:
	for d in $(SUBDIRS); do $(MAKE) --directory $$d clean; done
