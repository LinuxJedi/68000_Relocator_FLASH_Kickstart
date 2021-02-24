# Requires xdftool from amitools (https://github.com/cnvogelg/amitools/)
xdftool fk.adf create
xdftool fk.adf format "FK"
xdftool fk.adf write FK
xdftool fk.adf write FlashKickstart
xdftool fk.adf write FlashKickstart.info
xdftool fk.adf write libs13
xdftool fk.adf write libs20
xdftool fk.adf write libs13.info
xdftool fk.adf write libs20.info

