# Analysis on babbling CAN node

Within different constraints and delays and starts of our two CAN nodes,
this varies widely. The most consistent arrangement we configured was a
ms delay of ~400 on the sleeping, lower priority node, and a busy loop
counting to 1000 on the other node.
The sleeping node consistently receives about 8 CAN messages and then hangs
due to the backed up CAN bus.

# Renode setup
The Raspberry Pico needs configuration files for Renode to work properly.

* On MacOS, the installation location is `/Applications/Renode.app/Contents/MacOs`
* On Linux, the location for Debian, Fedora, and Arch is `/opt/renode`
* On Windows, the location is `C://Program Files/Renode`

To add the Pico configuration files:
1. Copy `rp2040_spinlock.py` and `rp2040_divider.py` to the `scripts/pydev` directory of your Renode installation.
1. Copy `rpi_pico_rp2040_w.repl` to the `platforms/cpus` directory.
