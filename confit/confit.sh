#!/bin/bash

CONFIT_PATH="/home/ahutsikau/work/github/tmp"

if [ -z $CONFIT_PATH ]
then
	CONFIT_PATH="/usr/share/confit"
fi

CFIT_WORK=$CONFIT_PATH/work
CFIT_GIT=$CONFIT_PATH/git

gitc="git --git-dir=${CFIT_GIT} --work-tree=${CFIT_WORK}"

TRACE="eval echo \$FUNCNAME\(\) in \\\"\$(basename \$BASH_SOURCE)\\\" at \$LINENO"

init()
{
	mkdir -pv $CFIT_WORK
	mkdir -pv $CFIT_GIT
	$gitc init
}

status()
{
	echo "confit work : ${CFIT_WORK}"
	echo "confit git  : ${CFIT_GIT}"
	tree ${CFIT_WORK}
	$gitc status
}

cp_in()
{
	for f in $*
	do
		F=$(realpath -s $f)
		FP=$(dirname $F)
		mkdir -pv $CFIT_WORK/$FP
		cp -rvsui $f $CFIT_WORK/$FP/
	done
}


cp_out()
{
	echo $TRACE
	$TRACE

}


add()
{
	for f in $*
	do
		F=$(realpath -s $f)
		if [ ! -e $CFIT_WORK/$F ]
		then
			cp_in $f
		fi
		$gitc add $CFIT_WORK/$F
	done
}


remove()
{
	$TRACE

}


commit()
{
	$gitc commit
}

if [ -n "${1}" ]
then
	_cmd=${1/-/_}
	_args=( $* )
	$_cmd ${_args[@]:1}
else
	USAGE="${0} [cmd] [args] \n
	cmd: \n
	\t	init \n
	\t	status \n
	\t	cp-in \n
	\t	cp-out \n
	\t	add \n
	\t	remove \n
	"
	echo -e $USAGE
fi



