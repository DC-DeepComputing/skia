#!/usr/bin/env bash



OUTPUTFILENAME="ganeshvk_dm_output.txt"
ANDROIDDIR="/data/local/tmp/"
ANDROIDFILEPATH=${ANDROIDDIR}${OUTPUTFILENAME}

# Define cmd as variable so we can cleanly use more lines for clarity
CMDSTR="
./data/local/tmp/dm -v
--config vk
--resourcePath /data/local/tmp/resources

--src gm
--match hdr-pip-blur

>> ${ANDROIDFILEPATH}"

rm ${OUTPUTFILENAME}
adb shell rm ${ANDROIDFILEPATH}
adb shell touch ${ANDROIDFILEPATH}
adb shell ${CMDSTR}
adb pull ${ANDROIDFILEPATH} .

