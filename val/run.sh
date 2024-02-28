#!/system/bin/sh

# testmte > 1 for test mte functionality, = 0 for benchmarking mte, = -1 for benchmark no mte behavior
# buffer_size = size of the buffer
# setup_teardown = 0: set up, iteration (apply MTE (testmte==0), access, madvice); = 1 iteration (set up, apply MTE (testmte==0), madvice) 
# workload = 0, access benchmark is pointer chasing, = 1 access benchmark is memset+sha256
# iteration 

# Exp1: MTE and noMTE (set up, apply MTE (testmte==0), madvice) vs buffer size

for size in 16 160 1600 16000 160000 1600000 16000000 16000000 160000000
do
    ./validation 0 $size 1 0 100 >> exp1.txt
    sleep 5
done

for size in 16 160 1600 16000 160000 1600000 16000000 16000000 160000000
do
    ./validation -1 $size 1 0 100 >> exp1.txt
    sleep 5
done

# Exp2: MTE and noMTE (apply MTE (testmte==0), access = pointer_chasing, madvice) vs buffer size

for size in 16 160 1600 16000 160000 1600000 16000000 16000000 160000000
do
    ./validation 0 $size 0 0 100 >> exp2.txt
    sleep 5
done

for size in 16 160 1600 16000 160000 1600000 16000000 16000000 160000000
do
    ./validation -1 $size 0 0 100 >> exp2.txt
    sleep 5
done

# Exp3: MTE and noMTE (apply MTE (testmte==0), access = memset+sha256, madvice) vs buffer size

for size in 16 160 1600 16000 160000 1600000 16000000 16000000 160000000
do
    ./validation 0 $size 0 1 100 >> exp3.txt
    sleep 5
done

for size in 16 160 1600 16000 160000 1600000 16000000 16000000 160000000
do
    ./validation -1 $size 0 1 100 >> exp3.txt
    sleep 5
done