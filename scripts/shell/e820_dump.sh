#!/bin/bash
#/********************************************************************
# *
# * File:  e820_dump.sh
# * Name:  Fanqi Lu
# *
# * Description: 
#              Display that memory map in a human-readable format.
# *
# * Copyright (c) 1985-2017 by Fanqi Lu.
# * All rights reserved.
# *
# **********************************************************************/

#/sys/firmware/memmap is an interface that provides 
#the raw memory map to userspace.
#Following shell snippet can be used to display 
#that memory map in a human-readable format.
cd /sys/firmware/memmap
for dir in * ; do
    start=$(cat $dir/start)
    end=$(cat $dir/end)
    type=$(cat $dir/type)
    printf "%016x-%016x (%s)\n" $start $end "$type"
done

