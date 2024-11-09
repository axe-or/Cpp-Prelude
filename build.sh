CXX='g++ -std=c++20'
CFLAGS='-O0 -fPIC -Wall -Wextra -fno-strict-aliasing -g'
LDFLAGS=''

set -e
BuildMode="$1"

Run(){ echo "$@"; $@ ; }

case BuildMode in
	"sanitize") Run $CXX $CFLAGS -o main.bin main.cpp $LDFLAGS -fsanitize=address -lasan ;;
	*) Run $CXX $CFLAGS -o main.bin main.cpp $LDFLAGS ;;
esac

./main.bin


