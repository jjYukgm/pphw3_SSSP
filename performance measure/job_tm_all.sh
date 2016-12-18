#/bin/bash
stuID=`echo $USER`

function judge() {
	local index=$1
	local ind2=$2
	#SSSP_Pthread SSSP_MPI_sync SSSP_MPI_async
	#job_Str_p job_Str_ms job_Str_ma
	job_id=$(echo $(qsub -v exe="SSSP_Pthread" "tm3/job_Str_p${ind2}_$index.sh") | awk -F '[. ]' '{print $1}')

	#esp esms esma
	while [ ! -f "esp${ind2}$index" ]; do
		sleep 0.2
	done
	sleep 0.2

	index_=$(printf '%2d' $index)

	
	echo -e "TimeMeasure $index_ \E[0;32;40maccepted\E[0m"
}

#for ((j=5; j<=8; j=j+1))
#do
#	for ((i=1; i<=4; i=i+1))
#	do
#		#if 	[ ${i} -eq 13 ] ; then
#		#	continue
#		#elif [ ${i} -eq 17 ] || [ ${i} -eq 19 ]; then
#		#	continue
#		#elif [ ${i} -eq 22 ] || [ ${i} -eq 23 ]; then
#		#	continue
#		#fi
#		judge ${i} ${j}
#	done
#	judge 36 ${j}
#	judge 48 ${j}
#
#done

qsub tm2/job_p_p.sh
while [ ! -f "epp" ]; do
	sleep 0.2
done
sleep 0.2
echo -e "TimeMeasure Pthread \E[0;32;40maccepted\E[0m"

qsub tm2/job_p_ms.sh
while [ ! -f "epms" ]; do
	sleep 0.2
done
sleep 0.2
echo -e "TimeMeasure p_ms \E[0;32;40maccepted\E[0m"

qsub tm2/job_p_ma.sh
while [ ! -f "epma" ]; do
	sleep 0.2
done
sleep 0.2
echo -e "TimeMeasure p_ma \E[0;32;40maccepted\E[0m"
