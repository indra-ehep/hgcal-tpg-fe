#!/bin/bash

#./emul_Jul24.exe $1 $2 $3 $4 $5 $6 > out_${1}.log &

# #relaylist=runlist_sep24-testbeam.txt
# relaylist=dpglist.csv
# while read relaynum
# do
#     #echo Relay number : $relaynum
#     #relay=`echo $relaynum | cut -f 1 -d r`
#     prevrun=0
#     for ifile in `ls dat/Relay${relaynum}/*Link0*`
#     do
# 	#echo ifile : $ifile
# 	rname=`echo $ifile | cut -f 3 -d '/' | cut -d 'n' -f 2 | cut -d '_' -f 1`
# 	if [ "$rname" -ne "$prevrun" ] ; then
# 	    #echo -e "\n\n ============== Relay : $relaynum Run : $rname ================= "
# 	    nofevents=`./dump_event.exe $relaynum $rname 0 10 2>/dev/null | tail -n 1 | cut -d ':' -f 2`
# 	    nofevents=$[$nofevents+1]
# 	    echo -e "$relaynum $rname $nofevents"
# 	    prevrun=$rname
# 	fi
#     done
# done < $relaylist

# procrun=procrun.txt
# while read relay run nevents
# do
#     echo -e "======== Relay:$relay, Run:$run, NofEvents:$nevents ========="
#     echo -e "./loop_emul_Sep24.exe $relay $run $nevents 1 1 >out_${relay}_${run}.log 2>error_${relay}_${run}.log"
#     ./loop_emul_Sep24.exe $relay $run $nevents 1 1 >out_${relay}_${run}.log 2>error_${relay}_${run}.log
#     echo -e "\n"
# done < $procrun

# maxproc=20
# procrun=procrun.txt
# while read relay run nevents
# do
#     echo -e "======== Relay:$relay, Run:$run, NofEvents:$nevents ========="
#     echo -e "./loop_emul_Sep24.exe $relay $run $nevents 1 1 >out_${relay}_${run}.log 2>error_${relay}_${run}.log"
#     nofproc=`ps -ef | grep loop_emul_Sep24.exe | wc -l`
#     while [ $nofproc -gt $maxproc ];
#     do
# 	sleep 60
# 	nofproc=`ps -ef | grep loop_emul_Sep24.exe | wc -l`
#     done
#     ./loop_emul_Sep24.exe $relay $run $nevents 1 1 >out_${relay}_${run}.log 2>error_${relay}_${run}.log &
#     echo -e "\n"
# done < $procrun

maxproc=10
procrun=filelist.txt
index=1
while read fname
do
    echo -e "======== filename:$fname ========="
    nofproc=`ps -ef | grep stage2HtoTauTauEnergyCorrelation.exe | wc -l`
    while [ $nofproc -gt $maxproc ];
    do
	sleep 60
	nofproc=`ps -ef | grep stage2HtoTauTauEnergyCorrelation.exe | wc -l`
    done
    echo -e "./stage2HtoTauTauEnergyCorrelation.exe $fname $index"
    ./stage2HtoTauTauEnergyCorrelation.exe $fname $index >out_${index}.log &
    index=$[$index+1]
    echo -e "\n"
done < $procrun
