#!/bin/bash

# Make sure LLVM is installed and the bin directory set in PATH.
# Run the following command from the root directory of the project within bash.
# .\contrib\run-clang-format.sh

find examples source -regex '.*\.\(cpp\|h\|hpp\|cc\|cxx\)' -exec clang-format -style=file -i {} \;
