#! /bin/sh

pid=$1
perf_path=/tmp/squick_flame.perf
script_path=/tmp/squick_flame.script
svg_path=/tmp/squick_flame.svg

if [[ $# < 1 ]];then
	echo "Useage: ./captrue.sh pid"
	exit 1
fi

rm $perf_path $script_path $svg_path

cancel_handler() {
	echo "Exporting to svg"
	perf script -i $perf_path > $script_path
	cat $script_path | ./stackcollapse-perf.pl | ./flamegraph.pl > $svg_path
	echo "Exported the flamegraph at $svg_path"
}

trap cancel_handler SIGINT
perf record -a -p $pid -g -o $perf_path
