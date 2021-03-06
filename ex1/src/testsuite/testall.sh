#!/bin/bash
set -e


genpathpath=../generator/generate.exec
diffpath=../diffpy/diff.py
#diffpath=echo
serialpath=../serial/main.exec
testfilesSizes=(10 100 1000)
#testfilesSizes=(5 15 42 100 500 1000 2000 5000)
#MPItestfolders=(../mpi/ptp/continuous-single ../mpi/ptp/hybrid ../mpi/collective/hybrid ../mpi/collective/continuous-single)
MPItestfolders=(../mpi/continuous ../mpi/cyclic)
OPENMPtestfolders=(../openmp)
NTHREADS=4
for i in ${testfilesSizes[@]}
do
    testfiles[${i}]="mat_${i}.in"
    if [[ 'x'$1 != 'xgen' && -f "mat_${i}.in" ]];
    then
        echo "Not generating mat_${i}.in, file exists."
    else
        echo "Generating mat_${i}.in"
        ${genpathpath} ${i} "mat_${i}.in"
    fi
done

# Serial execution
echo "============ SERIAL EXECUTION =============="
for i in ${testfiles[@]}
do
    echo "Running testfile :" ${i}
    out="${j//\.\.\//}"
    serialfile="serial_${i%in}out"
    ${serialpath} ${i} ${serialfile}
done


# Parallel execution using MPI
echo "============ MPI EXECUTION =============="
for j in ${MPItestfolders[@]}
do
    echo $j
    echo "Point-To-Point runs"
    for i in ${testfiles[@]}
    do
        echo "Running testfile:" ${i}
        out="${j//\.\.\//}"
        outfile="${out//\//_}_ptp_${i%in}out"
        serialfile="serial_${i%in}out"
        mpirun -np ${NTHREADS} ${j}/main.exec ${i} ${outfile}
        ${diffpath} ${serialfile} ${outfile}
    done

    echo "Collective runs"
    for i in ${testfiles[@]}
    do
        echo "Running testfile:" ${i}
        out="${j//\.\.\//}"
        outfile="${out//\//_}_collective_${i%in}out"
        serialfile="serial_${i%in}out"
        mpirun -np ${NTHREADS} ${j}/main.exec ${i} ${outfile} 1
        ${diffpath} ${serialfile} ${outfile}
    done
done

# Parallel execution using OpenMP
echo "============ OPENMP EXECUTION =============="
for j in ${OPENMPtestfolders[@]}
do
    echo $j
    for i in ${testfiles[@]}
    do
        echo $i
        out="${j//\.\.\//}"
        outfile="${out//\//_}_${i%in}out"
        serialfile="serial_${i%in}out"
        ${j}/main.exec ${i} ${outfile}
        ${diffpath} ${serialfile} ${outfile}
    done
done
