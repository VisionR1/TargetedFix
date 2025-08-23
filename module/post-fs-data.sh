#!/system/bin/sh
# Early boot restore

MODULE_NAME=targetedfix
MODULE_PATH=/data/adb/modules/$MODULE_NAME
UPDATE_PATH=/data/adb/modules_update/$MODULE_NAME

mkdir -p "$MODULE_PATH/config"

# Restore from backup if available
for f in "$UPDATE_PATH/config/"*; do
    [ -s "$f" ] && cp -af "$f" "$MODULE_PATH/config/"
done 2>/dev/null