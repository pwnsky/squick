#! /bin/bash

cd $(dirname $0)

if [ -z $SQUICK_RUN_ENV ];then
    SQUICK_RUN_ENV="dev"
fi
echo "Run env: $SQUICK_RUN_ENV"

out_path="../config/node"
tmpl_path="../config/tmpl"
conf_file="$tmpl_path/env/$SQUICK_RUN_ENV.conf"

echo "Clean old env config"
rm -rf $out_path

files=()
while IFS= read -r file; do
    if [[ $file != "env" ]];then
        files+=("$file")
    fi
done < <(ls $tmpl_path)

echo "Gen env config..."
mkdir -p $out_path

echo "replacing ..."

sed_cmd="sed"
while IFS= read -r line
do
    key={`echo $line | awk '{split($0, a, "="); print a[1]}'`}
    value=`echo $line | awk '{split($0, a, "="); print a[2]}'`
    sed_cmd+=" -e 's/$key/$value/g'"
done < "$conf_file"

# search
for file in "${files[@]}"; do
    tmpl_file="$tmpl_path/$file"
    bash -c "cat $tmpl_file | $sed_cmd $tmpl_file" > $out_path/$file
done
echo "Gen cfg ok"
