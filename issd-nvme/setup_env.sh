#!/bin/bash

# add ramdisk on /mnt/ramdisk
sudo mount -t tmpfs -o size=1024m tmpfs /mnt/ram_disk
