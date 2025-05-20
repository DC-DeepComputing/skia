#!/usr/bin/env bash



OUTPUTFILENAME="grvk_nanobench_csv_results.txt"
ANDROIDDIR="/data/local/tmp/"
ANDROIDFILEPATH=${ANDROIDDIR}${OUTPUTFILENAME}

# Define cmd as variable so we can cleanly use more lines for clarity
CMDSTR="
./data/local/tmp/nanobench -v
--config grvk
--resourcePath /data/local/tmp/resources

--sourceType gm
--match hdr-pip-blur

--csv
--samples 1
>> ${ANDROIDFILEPATH}
--gpuStats"

rm ${OUTPUTFILENAME}
adb shell rm ${ANDROIDFILEPATH}
adb shell touch ${ANDROIDFILEPATH}
adb shell ${CMDSTR}
adb pull ${ANDROIDFILEPATH} .

