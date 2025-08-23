# Targeted Fix
*Forked to be more flexible, with other apps in a target list, except only GMS.*

[![GitHub release (latest by date)](https://img.shields.io/github/v/release/VisionR1/TargetedFix?label=Release&color=blue&style=flat)](https://github.com/VisionR1/TargetedFix/releases/latest)
[![GitHub Release Date](https://img.shields.io/github/release-date/VisionR1/TargetedFix?label=Release%20Date&color=brightgreen&style=flat)](https://github.com/VisionR1/TargetedFix/releases)
[![GitHub Releases](https://img.shields.io/github/downloads/VisionR1/TargetedFix/latest/total?label=Downloads%20%28Latest%20Release%29&color=blue&style=flat)](https://github.com/VisionR1/TargetedFix/releases/latest)
[![GitHub All Releases](https://img.shields.io/github/downloads/VisionR1/TargetedFix/total?label=Total%20Downloads%20%28All%20Releases%29&color=brightgreen&style=flat)](https://github.com/VisionR1/TargetedFix/releases)

A Zygisk module which "fixes" the FP to the apps in the target list.

To use this module you must have one of the following (latest versions):

- [Magisk](https://github.com/topjohnwu/Magisk) with Zygisk enabled (and Enforce DenyList enabled if NOT also using [Shamiko](https://github.com/LSPosed/LSPosed.github.io?tab=readme-ov-file#shamiko) or [Zygisk Assistant](https://github.com/snake-4/Zygisk-Assistant) or [NoHello](https://github.com/MhmRdd/NoHello), for best results)
- [KernelSU](https://github.com/tiann/KernelSU) with [Zygisk Next](https://github.com/Dr-TSNG/ZygiskNext) or [ReZygisk](https://github.com/PerformanC/ReZygisk) or [NeoZygisk](https://github.com/JingMatrix/NeoZygisk) module installed
- [KernelSU Next](https://github.com/KernelSU-Next/KernelSU-Next) with [Zygisk Next](https://github.com/Dr-TSNG/ZygiskNext) or [ReZygisk](https://github.com/PerformanC/ReZygisk) or [NeoZygisk](https://github.com/JingMatrix/NeoZygisk) module installed
- [APatch](https://github.com/bmax121/APatch) with [Zygisk Next](https://github.com/Dr-TSNG/ZygiskNext) or [ReZygisk](https://github.com/PerformanC/ReZygisk) or [NeoZygisk](https://github.com/JingMatrix/NeoZygisk) module installed

## About module

It injects a classes.dex file to modify fields in the android.os.Build class. Also, it creates a hook in the native code to modify system properties. These are spoofed only in the apps inside in the target list.

Also, there have a robust multi-target .json spoofer, which only targeted specific apps with a differned FP than the default .json. 

A example, inside in the target.txt write the package name but then create a com.android.vending.json or com.google.android.gms.unstable.json with what FP you want inside, etc.

Also, read the original README.md from PIFork:
https://github.com/osm0sis/PlayIntegrityFork?tab=readme-ov-file#play-integrity-fork

## How use this module

For generally use, customize target packages in /data/adb/targetedfix/config/target.txt & the FP in the /data/adb/targetedfix/config/fix.json to "fix" the FP.

For multi-target with different FPs, customize target packages in /data/adb/targetedfix/config/target.txt, but instead of fix.json rename it to the package name or process of the apps (example com.android.vending.json & com.google.android.gms.unstable.json).
For the GMS, it's the same process with PIFork.