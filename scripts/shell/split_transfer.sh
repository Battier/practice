#!/bin/sh
#######################################################
# Author	: Fanqi Lu
# Date		: 2017/8/1
# Brief     : Split the large file into smaller bits, that 
#             can easily be transferred over network. 
#             At destination, you can join them back to the original archive.
#######################################################

password="nicai"
username="Fanqi"
ip="192.168.1.1"
source="/path/of/file"
destination="/path/of/file"
local_work_dir=`mktemp -d`
remote_work_dir=`sshpass -p "$password" ssh  $username@$ip "mktemp -d"`
split_length="2M"

send(){
    count=0
    files="$(ls aabbcc*)"
    for element in $files
    do
        now=$(date +"%T")
        echo "$count, Sending $element, Start time: $now"
        sshpass -p "$password" scp $element $username@$ip:$remote_work_dir &
        count=$(( count+1 ))
    done
}

usage(){
   echo Usage: $0 []
   echo first argument: $1
   echo second argument: $2
   echo third argument: $3
   echo here they are again: $@    
}

while getopts dipslu: opt
do
    case "$opt" in
      d)  destination="$OPTARG";;
      i)  ip="$OPTARG";;
      p)  password="$OPTARG";;
      s)  source="$OPTARG";;
      u)  username="$OPTARG";;
      l)  split_length="$OPTARG";;
      \?)		# unknown flag
      	  echo >&2 "usage: $0 [-d dest] [-s src] [-i ip] [-u username] [-p password] [-l split_length]" 
          exit 1;;
    esac
done
shift `expr $OPTIND - 1`

#remove the remote image
sshpass -p "$password" ssh  $username@$ip "rm -rf $destination"

#Split image into multiple chunks.
cp $source $local_work_dir
md5sum $source 
md5sum $local_work_dir/*
cd  $local_work_dir
/usr/bin/split --bytes $split_length --numeric-suffixes --suffix-length=3  $local_work_dir/* aabbcc 

#send image
send
wait

#merge them
sshpass -p "$password" ssh  $username@$ip "cat $remote_work_dir/aabbcc* >> $destination"
sshpass -p "$password" ssh  $username@$ip "md5sum $destination"

#remove the tmp dir
rm -rf $local_work_dir
sshpass -p "$password" ssh  $username@$ip "rm -rf $remote_work_dir"

now=$(date +"%T")
echo "Finish time: $now"

