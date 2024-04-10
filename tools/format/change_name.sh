#! /bin/bash
echo "This script will rename in [../../src] directory"

if [[ $# < 2 ]];then
	echo "Usage: ./change_name.sh old_string new_string"
fi

dir=../../src

sed -i "s/$1/$2/g" `grep $1 -rl $dir`
