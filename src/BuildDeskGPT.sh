#!/bin/bash

SOURCE_PATH=$1
EXE_NAME=$2
pushd $SOURCE_PATH > /dev/null
rm -f $SOURCE_PATH/obj/Release/*
rm -f $SOURCE_PATH/bin/Release/*
for f in *.cpp; do
	file_name=$(echo $f | cut -d "." -f 1)
	x86_64-w64-mingw32-c++.exe  -Wall -O2 -std=c++14 -I"$SOURCE_PATH/include" -c "$SOURCE_PATH/$f" -o $SOURCE_PATH/obj/Release/$file_name.o
	if [[ ! $? -eq 0 ]]; then
		echo "Failed to compile $f exiting build"
		exit 1
	fi
	echo "Succesfully compiled $f."
done

pushd $SOURCE_PATH/obj/Release/ > /dev/null
for f in *.o; do
	object_file+=( $f )
done
echo "Linking object files."
x86_64-w64-mingw32-g++.exe -o $SOURCE_PATH/bin/Release/$EXE_NAME "${object_file[@]}"
if [[ ! $? -eq 0 ]]; then
	echo "Failed to link objects, build failed."
	exit 1
fi

echo "Succesfully built $EXE_NAME"

echo "Copying DLL's"

if [[ ! -f $SOURCE_PATH/bin/Release/libstdc++-6.dll ]]; then
        cp /cygdrive/c/cygwin64/usr/x86_64-w64-mingw32/sys-root/mingw/bin/libstdc++-6.dll $SOURCE_PATH/bin/Release/
        if [[ ! $? -eq 0 ]]; then
                echo "Failed to copy libstdc++-6.dll from MINGW/bin/"
        fi
fi

if [[ ! -f $SOURCE_PATH/bin/Release/libgcc_s_seh-1.dll ]]; then
        cp /cygdrive/c/cygwin64/usr/x86_64-w64-mingw32/sys-root/mingw/bin/libgcc_s_seh-1.dll $SOURCE_PATH/bin/Release/
        if [[ ! $? -eq 0 ]]; then
                echo "Failed to copy libgcc_s_seh-1.dll from cygwin64/usr/x86_64-w64-mingw32/sys-root/mingw/bin/libgcc_s_seh-1.dll"
        fi

fi

if [[ ! -f $SOURCE_PATH/bin/Release/libwinpthread-1.dll ]]; then
         cp /cygdrive/c/cygwin64/usr/x86_64-w64-mingw32/sys-root/mingw/bin/libwinpthread-1.dll $SOURCE_PATH/bin/Release/
        if [[ ! $? -eq 0 ]]; then
                echo "Failed to copy linbwinpthread-1.dll from cygwin64/usr/x86_64-w64-mingw32/sys-root/mingw/bin/libwinpthread-1.dll"
        fi

fi


