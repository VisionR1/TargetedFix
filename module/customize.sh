#!/system/bin/sh

MODPATH=${0%/*}
MODULE_NAME=targetedfix
MODULE_PATH=/data/adb/modules/$MODULE_NAME
UPDATE_PATH=/data/adb/modules_update/$MODULE_NAME

ui_print "-> Installing $MODULE_NAME..."

# If old install exists = update/reinstall
if [ -d "$MODULE_PATH/config" ]; then
    ui_print "-> Backing up user configs..."
    mkdir -p "$UPDATE_PATH/config"

    for f in "$MODULE_PATH/config/"*; do
        [ -s "$f" ] && cp -af "$f" "$UPDATE_PATH/config/"
    done 2>/dev/null
fi

ui_print "-> $MODULE_NAME installation done!"
ui_print "-> Thanks for using this module.🙂"

exit 0