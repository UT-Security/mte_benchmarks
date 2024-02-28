CC := aarch64-linux-gnu-gcc
CFLAGS := -static -march=armv8.5-a+memtag -O3 -I../util/ 
SRC := validation.c
UTIL_SRC := ../util/util.c ../util/sha256.c

validation: $(SRC) $(UTIL_SRC)
	$(CC) $(CFLAGS) -o validation $(SRC) $(UTIL_SRC) 

.PHONY: clean
clean:
	rm -f validation