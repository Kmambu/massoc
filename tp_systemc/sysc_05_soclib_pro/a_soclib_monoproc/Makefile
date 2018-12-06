SIMULATOR_BINARY=run.exe
ARCH=mips32el
SIMULATION_ARGS=100000
SOCLIB?=$(shell soclib-cc --getpath)
export SOCLIB
include $(SOCLIB)/soclib/platform/topcells/all.mk

.PHONY:	myclean

myclean:
	rm -f vcitty0 *~

clean:	myclean
