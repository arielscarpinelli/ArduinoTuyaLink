#!/bin/sh

SDK=tuya-connect-kit-for-mqtt-embedded-c

rm -rf src
mkdir src

# Hack until https://github.com/arduino/arduino-cli/pull/502
for file in \
	$SDK/include/*.h \
	$SDK/interface/*.h \
	$SDK/libraries/coreJSON/source/include/*.h \
	$SDK/utils/*.h \
	source/*.h
#	$SDK/libraries/coreMQTT/source/include/*.h \
#	$SDK/libraries/coreHTTP/source/include/*.h \
#	$SDK/libraries/coreHTTP/source/dependency/3rdparty/http_parser/*.h \
#	$SDK/middleware/*.h \
do
	ln -s ../$file src/$(basename $file)
done

ln -s ../$SDK/src src/sdk
# ln -s ../$SDK/middleware src/middleware
ln -s ../$SDK/utils src/utils
ln -s ../$SDK/libraries/coreJSON src/coreJSON
# ln -s ../$SDK/libraries/coreMQTT src/coreMQTT
# ln -s ../$SDK/libraries/coreHTTP src/coreHTTP
ln -s ../source src/src

# rename base64.h link to mbedtlsbase64.h
mv src/base64.h src/mbedtlsbase64.h
mv src/queue.h src/tuya_queue.h

# Apply diff
cd $SDK
git apply ../sdk_patch.txt




