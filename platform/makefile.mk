# Select target platform
linux:
	cp linux.mk platform.mk
other:
	echo other

fast:
	cp fast.mk optimize.mk
debug:
	cp debug.mk optimize.mk
