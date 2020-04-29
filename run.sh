dmesg -c > /dev/null
cat $1
./main < $1
dmesg | grep Project1
