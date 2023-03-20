#! /bin/bash
# Author: i0gan
# Email : l418894113@gmail.com
# Date  : 2022-03-18
# Github: https://github.com/pwnsky/squick
# Description: Format all source files

echo -n "Running clang-format "
find ../../src -name "*\.h" -o -name "*\.cc" -o -name "*\.cc"|grep -v bundled|xargs -I {} sh -c "clang-format -i -style=file {}; echo -n '.'"
echo
