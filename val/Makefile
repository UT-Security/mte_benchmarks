CC := clang
CFLAGS := -v -target aarch64-linux-gnu -march=armv8.5-a+memtag -static -O0 -I../util/ 
SRC := validation.c
UTIL_SRC := ../util/util.c ../util/sha256.c

validation: $(SRC) $(UTIL_SRC)
	$(CC) $(CFLAGS) -o validation $(SRC) $(UTIL_SRC) 

.PHONY: clean
clean:
	rm -f validation
