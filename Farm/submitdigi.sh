#!/bin/tcsh

# expects a command line argument
if ($#argv == 1 ) then
    set RunList = $1
endif

#get the no of runs from the list
set Runs=`cat $RunList`

# set environment variable for directory to run digi from
setenv BATCH_DIR /home/rachel/digi
cd ${BATCH_DIR}

set tempdir = "/scratch/rachel/tdis"
setenv TEMPDIR $tempdir

# execute FarmDigi each run in run list
foreach RunStart ($Runs)
    setenv RUNNUMBER $RunStart

    echo "Submitting run # $RunStart"
    qsub -V FarmDigi
    #FarmDigi
    echo "Waiting 20 seconds for next submit"
    sleep 10

 end

echo "ALL JOBS LOADED"

