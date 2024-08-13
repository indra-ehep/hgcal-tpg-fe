rm *.exe *.d *.pcm *.so
curdir=$PWD
for dir in `ls -ltr|grep "^d"|awk '{print $9}'`
do
    echo dir : $dir
    cd $dir
    rm *#
    rm *~
    #clear
    cd $curdir
done

