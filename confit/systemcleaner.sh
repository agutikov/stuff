#!/bin/bash

. local_bash_utils.sh

TMP_DIR="/home/$USER/tmp"

INSTALLED_FILES="$TMP_DIR/installed_files"
ALL_FILES="$TMP_DIR/all_files"
SORTED_INSTALLED_FILES="$TMP_DIR/sorted_installed_files"
PACKETS="$TMP_DIR/packets"
SORTED_ALL_FILES="$TMP_DIR/sorted_all_files"


get_all_installed_files()
{
	packets=$(eix '-I*' --format '<installedversions:NAMEVERSION>')

	echo $packets >> $PACKETS
	
	for p in $packets 
	do
		equery f $p >> $INSTALLED_FILES
	done

	sort $INSTALLED_FILES > $SORTED_INSTALLED_FILES 
}


get_all_files_w_exceptions()
{
	find / \
		! -path "/dev*" \
		! -path "/proc*" \
		! -path "/sys*" \
		! -path "/home*" \
		! -path "/tmp*" \
		! -path "/var*" \
		! -path "/mnt*" \
	> $ALL_FILES


	sort $ALL_FILES > $SORTED_ALL_FILES


}



get_all_files_w_exceptions

get_all_installed_files



