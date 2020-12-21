#!/bin/sh
#######################################################
# Author	: Fanqi Lu
# Date		: 2017/8/1
# Brief     : Split the large file into smaller bits, that
#             can easily be transferred over network.
#             At dest_file, you can join them back to the original archive.
#
# SSHPASS   : https://gist.github.com/arunoda/7790979
#######################################################

password="nicai"
username="Fanqi"
ip="192.168.1.1"
src_file="/path/of/file"
dest_file="/path/of/file"
split_length="4M"
prefix_tmp="aabbccddee"

function finish {
    if [$local_work_dir != "" ]; then
        echo "Removing local tmp dir: $local_work_dir"
        rm -rf $local_work_dir
    fi
}
trap finish EXIT

send(){
    count=0
    files="$(ls aabbcc*)"
    for element in $files
    do
        now=$(date +"%T")
        echo "$count, Sending $element, Start time: $now"
        sshpass -p "$password" scp $element $username@$ip:$remote_work_dir &
        sleep 0.30
        count=$(( count+1 ))
        if [ $counter -eq 10 ]; then
            sleep 1.00
            count=0
        fi
    done
}

while getopts "d:i:p:s:l:u:" opt
do
    case "$opt" in
      d)
          dest_file="$OPTARG"
          ;;
      i)
          ip="$OPTARG"
          ;;
      p)
          password="$OPTARG"
          ;;
      s)
          src_file="$OPTARG"
          ;;
      u)
          username="$OPTARG"
          ;;
      l)
          split_length="$OPTARG"
          ;;
      *)		# unknown flag
      	  echo >&2 "usage: $0 [-d dest] [-s src] [-i ip] [-u username] [-p password] [-l split_length]"
          exit 1
          ;;
    esac
done
shift `expr $OPTIND - 1`

echo "dest_file = $dest_file"
echo "ip        = $ip"
echo "username  = $username"
echo "src_file  = $src_file"

local_work_dir=`mktemp -d`
remote_work_dir=`sshpass -p "$password" ssh  $username@$ip "mktemp -d"`

#remove the remote image
sshpass -p "$password" ssh  $username@$ip "rm -rf $dest_file"

#Split image into multiple chunks.
cp $src_file $local_work_dir
md5sum $src_file
md5sum $local_work_dir/*
cd  $local_work_dir
/usr/bin/split --bytes $split_length --numeric-suffixes --suffix-length=3  $local_work_dir/* ${prefix_tmp}

#send image
send
md5sum_old='md5sum $src_file | cut -d ' ' -f 1"'
wait

#merge them
sshpass -p "$password" ssh  $username@$ip "cat $remote_work_dir/${prefix_tmp}* >> $dest_file"
sshpass -p "$password" ssh  $username@$ip "sync"
md5sum_new=`sshpass -p "$password" ssh  $username@$ip "md5sum $dest_file | cut -d ' ' -f 1"`

#remove the tmp dir
rm -rf $local_work_dir
sshpass -p "$password" ssh  $username@$ip "rm -rf $remote_work_dir"

if [ "$md5sum_old" != "$md5sum_new" ]; then
    echo "Original md5sum value: $md5sum_old"
    echo "New md5sum value:      $md5sum_new"
    echo "Send file failed"
    exit 1
fi

now=$(date +"%T")
echo "Finish time: $now"

