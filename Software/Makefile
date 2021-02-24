CC=vc +kick13
CFLAGS=-c99 -Ireqtools -DNDEBUG -lamiga -lauto

all: FK FlashKickstart

FK: FlashKickstart.c FlashDeviceSST39.c Helpers.c RomInfo.c
	$(CC) $(CFLAGS) $^ -o FK

FlashKickstart: gui.c RomInfo.c Helpers.c FlashDeviceSST39.c reqtoolsnb.lib
	$(CC) $(CFLAGS) $^ -o FlashKickstart
