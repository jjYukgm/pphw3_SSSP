#/bin/bash

echo "Generate the job file...."
# 輸出檔名, 數量
# %1 = core數量


# job_Str_p4 job_Str_ms1 job_Str_ma1
fn1=job_Str_p9
fn2=.sh
# q001_20 q002_24 q003_36 q004_48 q005_60 q006_100 q007_200 q008_400 q900_1000
fn3=q900_1000
# esp2 esms2 esma2
fn4=esp9
fn5=out
fn6=1000
#cd tm/

for i in $(seq 1 $1) 
do
total=$((${i}*12))

echo "#PBS -N ${fn1}_$i													"	>>${fn1}_$i${fn2}
echo "#PBS -r n															"	>>${fn1}_$i${fn2}
echo "#PBS -l nodes=$i:ppn=12   										"	>>${fn1}_$i${fn2}
echo "#PBS -l walltime=00:05:00											"	>>${fn1}_$i${fn2}
echo "#PBS -e ${fn4}$i         											"	>>${fn1}_$i${fn2}
echo "#PBS -o ${fn5}           											"	>>${fn1}_$i${fn2}
echo 'cd $PBS_O_WORKDIR                          				  		'	>>${fn1}_$i${fn2}
echo "OMP_NUM_THREADS=${total}	                   				  		"	>>${fn1}_$i${fn2}
echo "export MV2_ENABLE_AFFINITY=0								  	    "	>>${fn1}_$i${fn2}
#echo "time mpiexec -np ${fn6}"' ./$exe $OMP_NUM_THREADS '"${fn3} o51 1  " >>${fn1}_$i${fn2}
echo 'time ./$exe $OMP_NUM_THREADS '"${fn3} o51 1						"	>>${fn1}_$i${fn2}


done

exit 0
