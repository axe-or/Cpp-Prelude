CXX='g++ -std=c++20'
CFLAGS='-fPIC -Wall -Wextra -fno-strict-aliasing'
LDFLAGS='-static'

set -e
BuildMode="$1"

Run(){ echo "$@"; $@ ; }

case $BuildMode in
	"sanitize") Run $CXX $CFLAGS -o main.bin main.cpp $LDFLAGS -fsanitize=address -lasan ;;
	"dist") Run $CXX $CFLAGS -O2 -o main.bin main.cpp $LDFLAGS ;;
	*) Run $CXX $CFLAGS -O0 -g -o main.bin main.cpp $LDFLAGS ;;
esac

./main.bin


