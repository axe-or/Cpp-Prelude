CXX='g++ -std=c++20'
CFLAGS='-O0 -fPIC -Wall -Wextra -fno-strict-aliasing -g'
LDFLAGS=''

set -e
$CXX $CFLAGS -o main.bin main.cpp $LDFLAGS
[ "$1" = 'valgrind' ] \
    && valgrind ./main.bin \
    || ./main.bin
