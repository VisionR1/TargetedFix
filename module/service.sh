#!/system/bin/sh
# Late boot restore fallback

MODULE_NAME=targetedfix
MODULE_PATH=/data/adb/modules/$MODULE_NAME
UPDATE_PATH=/data/adb/modules_update/$MODULE_NAME

sleep 5
mkdir -p "$MODULE_PATH/config"

for f in "$UPDATE_PATH/config/"*; do
    [ -s "$f" ] && cp -af "$f" "$MODULE_PATH/config/"
done 2>/dev/null