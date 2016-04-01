测试发布包
cd rel/jni
ndk-build
adb push wqy-zenhei.ttc /data/
adb push ../libs/armeabi-v7a/sjk_test /data
adb shell
cd /data
./sjk_test

wqy-zenhei.ttc需要和sjk_test在同一个目录
# skwork
