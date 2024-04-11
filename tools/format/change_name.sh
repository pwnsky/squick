#! /bin/bash
dir=../../src/

echo "This script will rename in [$dir] directory"

if [[ $# < 2 ]];then
	echo "Usage: ./change_name.sh old_string new_string"
fi

sed -i "s/$1/$2/g" `grep $1 -rl $dir`
