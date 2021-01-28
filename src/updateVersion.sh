#!/bin/bash

version_h="../Version.h"
echo "=========================== UPDATE INFO ================================"
lineNumberCommit=`grep -n COMMIT_NUMBER $version_h | cut -d: -f1`
if [ -z $lineNumberCommit ]; then
	echo "COMMIT_NUMBER not found in $version_h"
else
	lineNumberDate=`grep -n BUILD_DATE $version_h | cut -d: -f1`

	if [ -z $lineNumberDate ]; then
		echo "BUILD_DATE not found in $version_h"
	else
		lineNumberTime=`grep -n BUILD_TIME $version_h | cut -d: -f1`

		if [ -z $lineNumberTime ]; then
			echo "BUILD_TIME not found in $version_h"
		else
			dateStr=`date '+%Y-%m-%d'`
			timeStr=`date '+%H:%M:%S'`
			commitHash=`git rev-parse --short HEAD`
			
			echo "Build $commitHash, $dateStr, $timeStr"
			
			cat $version_h | sed -e "${lineNumberTime}s/.*/#define BUILD_TIME   \"$timeStr\"/" | sed -e "${lineNumberDate}s/.*/#define BUILD_DATE   \"$dateStr\"/" | sed -e "${lineNumberCommit}s/.*/#define COMMIT_NUMBER    \"$commitHash\"/" > Version.tmp
			
			rm $version_h
			mv Version.tmp $version_h
		fi
	fi
fi
echo "========================================================================"

