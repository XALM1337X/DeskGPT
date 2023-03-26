#!/bin/bash
SOURCE_PATH=$1
BP_BUILD_FLAG=$2
BP_TOOLS_PATH=$3
rm -f $SOURCE_PATH/obj/Release/*
rm -f $SOURCE_PATH/bin/Release/*
rm -f $SOURCE_PATH/include/BP*.h
rm -f $SOURCE_PATH/lib/libBP*

if [[ $2 == "true" ]]; then
	pushd $BP_TOOLS_PATH/src > /dev/null
	./BuildBPTools.sh $(pwd)
	popd > /dev/null
fi


pushd $SOURCE_PATH > /dev/null



#Copy BPTools include file and library to GPTMobileServer/src/include and /lib respectively

cp $BP_TOOLS_PATH/src/include/*.h $SOURCE_PATH/include/
cp $BP_TOOLS_PATH/src/lib/libBPTools.so $SOURCE_PATH/lib/



for f in *.cpp; do
	file_name=$(echo $f | cut -d "." -f 1)
	gcc  -Wall -O2 -std=c++17 -I"$SOURCE_PATH/include" -c "$SOURCE_PATH/$f" -o $SOURCE_PATH/obj/Release/$file_name.o
	if [[ ! $? -eq 0 ]]; then
		echo "Failed to compile $f exiting build"
		exit 1
	fi
	echo "Succesfully compiled $f."
done


#Push object files for GPTMobileServer and not the GPTMobileClientTest
pushd $SOURCE_PATH/obj/Release/ > /dev/null
for f in *.o; do
	if [[ $f != "GPTMobileClientTest.o" ]]; then
		object_file+=( $f )
	fi
done

#Redhat Linux linking
#/usr/lib/gcc/x86_64-redhad-linux/8 This is the path to gcc/c++ ".so" library files
echo "Linking GPTMobileServer."
gcc -L/usr/lib/gcc/x86_64-redhat-linux/8 -L$SOURCE_PATH/lib -o $SOURCE_PATH/bin/Release/GPTMobileServer.exe "${object_file[@]}" -lstdc++ -pthread -lBPTools
if [[ ! $? -eq 0 ]]; then
	echo "Failed to link objects, build failed for GPTMobileServer."
	exit 1
fi
echo "Succesfully built $EXE_NAME"


echo "Linking GPTMobileClientTest."
gcc -L/usr/lib/gcc/x86_64-redhat-linux/8 -o $SOURCE_PATH/bin/Release/GPTMobileClientTest.exe GPTMobileClientTest.o -lstdc++
if [[ ! $? -eq 0 ]]; then
	echo "Failed to link objects, build failed for GPTMobileClientTest."
	exit 1
fi
################################################################################################################################
#These DLL paths are for windows builds with cygwin
#if [[ ! -f $SOURCE_PATH/bin/Release/libstdc++-6.dll ]]; then
#        cp /cygdrive/c/cygwin64/usr/x86_64-w64-mingw32/sys-root/mingw/bin/libstdc++-6.dll $SOURCE_PATH/bin/Release/
#        if [[ ! $? -eq 0 ]]; then
#                echo "Failed to copy libstdc++-6.dll from MINGW/bin/"
#        fi
#fi

#if [[ ! -f $SOURCE_PATH/bin/Release/libgcc_s_seh-1.dll ]]; then
#        cp /cygdrive/c/cygwin64/usr/x86_64-w64-mingw32/sys-root/mingw/bin/libgcc_s_seh-1.dll $SOURCE_PATH/bin/Release/
#        if [[ ! $? -eq 0 ]]; then
#                echo "Failed to copy libgcc_s_seh-1.dll from cygwin64/usr/x86_64-w64-mingw32/sys-root/mingw/bin/libgcc_s_seh-1.dll"
#        fi

#fi

#if [[ ! -f $SOURCE_PATH/bin/Release/libwinpthread-1.dll ]]; then
#         cp /cygdrive/c/cygwin64/usr/x86_64-w64-mingw32/sys-root/mingw/bin/libwinpthread-1.dll $SOURCE_PATH/bin/Release/
#        if [[ ! $? -eq 0 ]]; then
#                echo "Failed to copy linbwinpthread-1.dll from cygwin64/usr/x86_64-w64-mingw32/sys-root/mingw/bin/libwinpthread-1.dll"
#        fi

#fi


