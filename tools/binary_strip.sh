#! /bin/bash

cd ../bin
find . -type f -executable -exec strip {} \;
