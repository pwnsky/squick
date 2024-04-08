# FlameGraph

## Step 1: Compile progame as debug version
Before you start you shold compile your progame as debug version. then run it.
your can modify {project_path}/tools/common.sh
set build_version="Debug"

## Step 2: Record process

```
perf record -a -p 28537 -g -o /tmp/test.perf
```
The number 28537 is your target PID

You can cancel it with Ctrl + C when you think it enough.


## Step 3: Change it to gragh

## Change Specimen to script

perf script -i /tmp/test.perf > /tmp/test.script

### Export to svg
cat /tmp/test.script | ./stackcollapse-perf.pl | ./flamegraph.pl > /tmp/test.svg

## Step 4: Use browser to analyze it

Now use your browser to open /tmp/test.svg
