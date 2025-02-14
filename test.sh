#!/bin/bash
export GTEST_COLOR=yes
cd build
ctest --verbose | sed \
  -e "s/^\([0-9]\+\/[0-9]\+ Test #[0-9]\+: .*\)/\\x1b[36m\1\\x1b[0m/g" \
  -e "s/\(Passed\)/\\x1b[32m\1\\x1b[0m/g" \
  -e "s/\(\*\*\*Failed\)/\\x1b[31m\1\\x1b[0m/g" \
  -e "s/\([0-9]\+\.[0-9]\+ sec\)/\\x1b[35m\1\\x1b[0m/g" \
  | while IFS= read -r line; do echo -e "$line"; done
cd ..
