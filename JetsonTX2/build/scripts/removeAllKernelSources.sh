#!/bin/bash
# Remove the kernel source and compressed files
cd /usr/src
rm -r kernel
rm -r hardware
rm -r nvidia
rm public_sources.tbz2
