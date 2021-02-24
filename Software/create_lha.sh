# Requires patool (in Debian / Ubuntu repos) and lha from https://github.com/jca02266/lha
tar -cvf fk.tar --transform 's,^,FK/,' FK FlashKickstart FlashKickstart.info libs13 libs13.info libs20 libs20.info
patool repack fk.tar fk.lha
rm fk.tar
