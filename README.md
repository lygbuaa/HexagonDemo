# HexagonDemo
1) download & install hexagon_sdk
2) source setup_sdk_env.source
3) run build_skel.sh to build libcalculator_skel.so
4) adb push libcalculator_skel.so /data/app/hexagon
5) build & run HexagonDemo
6) input num & CalcSum

run halide in APK
1) adb push ${HALIDE_ROOT}/lib/cdsp/arm-64-android/libhalide_hexagon_host.so  /vendor/lib64
2) modify /vendor/etc/public.libraries.txt， add <libhalide_hexagon_host.so> to tail # to make this library public, it should be in /vendor/lib64 and not under /system/
lib64
3) adb push ${HALIDE_ROOT}/lib/v60/libhalide_hexagon_remote_skel.so /vendor/lib64  # make this library public, it should be in /vendor/lib64 and not under /system/lib/
rfsa/adsp/
4) adb reboot & adb root