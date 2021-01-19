#!/usr/bin/bash
#######################################################
# Author	: Fanqi Lu
# Date		: 2017/8/1
# Brief     : Split the large file into smaller bits, that
#             can easily be transferred over network.
#             At dest_file, you can join them back to the original archive.
#
# How to run SCP/SSH without password prompt:
#   ssh-keygen -t rsa -b 4096
#   cat ~/.ssh/id_rsa.pub | ssh server_ip 'cat >> ~/.ssh/authorized_keys'
#
#######################################################

password="nicai"
username="Fanqi"
ip="192.168.1.1"
src_file="/path/of/file"
dest_file="/path/of/file"
split_length="20M"
prefix_tmp="aabbccddee"

finish() {
    if [ $local_work_dir != "" ];then
        echo "Removing local tmp dir: $local_work_dir"
        rm -rf $local_work_dir
    fi
    if [ $remote_work_dir != "" ];then
        echo "Removing remote tmp dir: $remote_work_dir"
        ssh  $username@$ip "rm -rf $remote_work_dir"
    fi
}
trap finish EXIT

send(){
    count=0
    files=`ssh $username@$ip "cd $remote_work_dir;ls aabbcc*"`
    for element in $files
    do
        now=$(date +"%T")
        echo "Sending $element, Start time: $now"
        scp $username@$ip:$remote_work_dir/$element ./ &
        sleep 5
        count=$(( count+1 ))
        if [ $count -eq 10 ]; then
            sleep 5.00
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
          echo >&2 "          Example( Server IP is  66.42.101.155, The source file is /home/falu/sonic-buildimage/target/sonic-vs.bin at server ):"
      	  echo >&2 "                  ./split_transfer.sh -d /home/falu/ws/sonic-buildimage/target/sonic-vs.bin -s /home/falu/sonic-buildimage/target/sonic-vs.bin -i 66.42.101.155 -u falu"
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
remote_work_dir=`ssh  $username@$ip "mktemp -d"`

#remove the remote image
rm -rf $dest_file

#Split image into multiple chunks.
ssh $username@$ip "cp $src_file $remote_work_dir"
ret=$?
ssh $username@$ip "cd $remote_work_dir;split --bytes $split_length --numeric-suffixes --suffix-length=3  $remote_work_dir/* ${prefix_tmp}"
ret=$?

#send image
cd $local_work_dir
send
wait
md5sum_old=`ssh  $username@$ip "md5sum $src_file | cut -d ' ' -f 1"`
echo "Original md5sum value: $md5sum_old"

#merge them
cat $local_work_dir/${prefix_tmp}* >> $dest_file
ret=$?
md5sum_new=`md5sum $dest_file | cut -d ' ' -f 1`
echo "New md5sum value:      $md5sum_new"
ls -l $dest_file

#remove the tmp dir
rm -rf $local_work_dir
ssh $username@$ip "rm -rf $remote_work_dir"
ret=$?

if [ "$md5sum_old" != "$md5sum_new" ]; then
    echo "Original md5sum value: $md5sum_old"
    echo "New md5sum value:      $md5sum_new"
    echo "Send file failed"
    exit 1
fi

now=$(date +"%T")
echo "Finish time: $now"
