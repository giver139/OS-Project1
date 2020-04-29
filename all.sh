for f in OS_PJ1_Test/*.txt;
do
    fn=`echo "$f" | cut -d / -f 2 | cut -d . -f 1`
    dmesg -c > /dev/null
    ./main < "$f" > output/"$fn"_stdout.txt
    dmesg | grep Project1 > output/"$fn"_dmesg.txt
done
