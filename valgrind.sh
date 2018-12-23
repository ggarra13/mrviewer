#!/bin/bash --norc

#valgrind -v --trace-children=yes --gen-suppressions=yes $*

valgrind -v --leak-check=full --trace-children=yes --leak-resolution=high --num-callers=40 --show-reachable=yes --read-var-info=yes --suppressions=valgrind.sup --track-origins=yes $*

# valgrind -v --demangle=yes --track-fds=yes --trace-children=yes --track-origins=yes --show-reachable=yes --leak-check=full $*


