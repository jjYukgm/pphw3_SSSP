
@echo "Generate the job file...."
@echo off
rem 輸出檔名, 數量
rem %1 = core數量

setlocal enabledelayedexpansion

rem job_Str_p4 job_Str_ms1 job_Str_ma1
@set  fn1=job_Str_ma3
@set  fn2=.sh
rem q001_20 q002_24 q003_36 q004_48 q005_60
@set  fn3=q003_36
@set  fn4=esma3
@set  fn5=out
@set  fn6=36


for /L %%i in (1 1 %1) do (

echo #PBS -N %fn1%_%%i		            	    				>>%fn1%_%%i%fn2%
echo #PBS -r n                                  				>>%fn1%_%%i%fn2%
echo #PBS -l nodes=1:ppn=1                      				>>%fn1%_%%i%fn2%
echo #PBS -l walltime=00:05:00                  				>>%fn1%_%%i%fn2%
echo #PBS -e %fn4%%%i	                        				>>%fn1%_%%i%fn2%
echo #PBS -o %fn5%                              				>>%fn1%_%%i%fn2%
echo cd $PBS_O_WORKDIR                          				>>%fn1%_%%i%fn2%
echo OMP_NUM_THREADS=%%i	                    				>>%fn1%_%%i%fn2%
echo export MV2_ENABLE_AFFINITY=0								>>%fn1%_%%i%fn2%
echo time mpiexec -np %fn6% ./$exe $OMP_NUM_THREADS %fn3% o51 1	>>%fn1%_%%i%fn2%
rem echo time ./$exe $OMP_NUM_THREADS %fn3% o51 1	>>%fn1%_%%i%fn2%


)


pause
goto exit

:exit
