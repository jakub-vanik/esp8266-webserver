PROJECT_NAME := snap

include $(IDF_PATH)/make/project.mk

upload:
	spiffy/spiffy files spiffy/spiff_rom.bin 2097152
	$(IDF_PATH)/components/esptool_py/esptool/esptool.py --port /dev/ttyUSB0 write_flash 0x100000 spiffy/spiff_rom.bin
