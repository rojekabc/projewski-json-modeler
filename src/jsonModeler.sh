#!/bin/bash

JSON_MODEL_PATH=/usr/local/share/projewski/jsonModeler

if test -z "$1"; then
	echo "Error: No model filename";
	echo "Usage: $0 <model filename>";
	exit;
fi

CHECKPARAMS="true";
while "$CHECKPARAMS" = "true"; do
	case "$1" in
		-resource)
			shift;
			JSON_MODEL_PATH=$1;
			shift;
			;;
		*)
			break;
			;;
	esac;
done;


fileNoExt=${1%%.*};
filePath=`dirname $1`;

# Check resources
echo "#define MODEL_FILENAME \"$1\"" > $filePath/jsonModeler.h
echo "#define OUT_CODE_FILENAME \"$fileNoExt.c\"" >> $filePath/jsonModeler.h
echo "#define OUT_HEADER_FILENAME \"$fileNoExt.h\"" >> $filePath/jsonModeler.h

if test -d $JSON_MODEL_PATH; then
	if ! test -f $JSON_MODEL_PATH/jsonModel.c; then
		echo "Error: Cannot find resource file [$JSON_MODEL_PATH/jsonModel.c]";
		exit;
	fi
else
	echo "Error: Cannot find resuorces folder [$JSON_MODEL_PATH]";
	exit;
fi;

if test -f $filePath/jsonModel.c; then
	echo "Warn: jsonModel.c file already exists in this location";
else
	cp $JSON_MODEL_PATH/jsonModel.c $filePath;
fi;

echo -n "Building model ... ";
cd $filePath;
rm -rf ERRORS;
gcc -o $fileNoExt $filePath/jsonModel.c -I/usr/local/include/projewski -lprojewski-tools 2> ERRORS;
if test -s ERRORS; then
	echo "fail";
	cat ERRORS;
	exit;
fi
rm $filePath/jsonModel.c $filePath/jsonModeler.h ERRORS;
./$fileNoExt;
rm $fileNoExt;
echo "done";
