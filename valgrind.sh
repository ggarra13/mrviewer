#!/bin/bash --norc

#valgrind -v --trace-children=yes --gen-suppressions=yes $*

valgrind -v --trace-children=yes --suppressions=valgrind.sup $*

# valgrind -v --demangle=yes --track-fds=yes --trace-children=yes --track-origins=yes --show-reachable=yes --leak-check=full $*


