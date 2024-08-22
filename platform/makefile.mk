# Select target platform
linux:
	cp linux.mk platform.mk
arm:
	cp arm.mk platform.mk
arm64:
	cp arm64.mk platform.mk

fast:
	cp fast.mk optimize.mk
debug:
	cp debug.mk optimize.mk
