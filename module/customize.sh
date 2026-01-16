#!/system/bin/sh
# customize.sh - Safe Installer

# 1. Setup Variables
# CRITICAL: Do NOT define MODPATH manually. Trust the Manager.
MODID=targetedfix
LIVE_PATH="/data/adb/modules/$MODID"

ui_print "- Installing TargetedFix..."

# 2. CONFIG RESTORATION
# Check if config folder exists AND is not empty
if [ -d "$LIVE_PATH/config" ] && [ "$(ls -A "$LIVE_PATH/config")" ]; then
    ui_print "- Preserving user config..."
    
    # A. Delete the default 'config' folder
    rm -rf "$MODPATH/config"
    
    # B. Copy the user's clean config folder
    cp -af "$LIVE_PATH/config" "$MODPATH/"
fi

ui_print "- Installation Done!"
ui_print "- Thanks for using this module.🙂"
