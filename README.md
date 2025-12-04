# Targeted Fix (TF)
*A fork of PIFork with a different aim.*


[![GitHub release (latest by date)](https://img.shields.io/github/v/release/VisionR1/TargetedFix?label=Release&color=blue&style=flat)](https://github.com/VisionR1/TargetedFix/releases/latest)
[![GitHub Release Date](https://img.shields.io/github/release-date/VisionR1/TargetedFix?label=Release%20Date&color=brightgreen&style=flat)](https://github.com/VisionR1/TargetedFix/releases)
[![GitHub Releases](https://img.shields.io/github/downloads/VisionR1/TargetedFix/latest/total?label=Downloads%20%28Latest%20Release%29&color=blue&style=flat)](https://github.com/VisionR1/TargetedFix/releases/latest)
[![GitHub All Releases](https://img.shields.io/github/downloads/VisionR1/TargetedFix/total?label=Total%20Downloads%20%28All%20Releases%29&color=brightgreen&style=flat)](https://github.com/VisionR1/TargetedFix/releases)

## Purpose

A Zygisk module that features the ability to do multiple targeted spoofing of Build Field and System Property values. Multiple app processes can be targeted by configuring a target list and either a common prop/field value spoof list or prop/field value spoof lists that target particular processes.

This allows a user to spoof almost any prop/field values to app processes other than GMS DroidGuard or Vending, making it extremely versatile.

A common use is spoofing a different device when an app uses device identifiers, eg fingerprint, to limit use to devices on a whitelist.

## Original USNF spoofProvider function.

This principle USNF/PIF/PIFork function is retained to keep the ability to break key attestation for calls made by DroidGuard. Useful for 'fixing' legacy Play Integrity DEVICE response verdicts.

Detail: spoofProvider registers a fake Keystore provider to break these calls with an exception (error), causing DroidGuard (on-device) to fall back to collecting non-attestation-key software signals for inclusion in a PI API token (delivery payload) that is used by Google servers to validate integrity verdicts. This is required to allow legacy DEVICE verdicts in unlocked devices with working Android Platform Key Attestation (Keymaster 3.0+).

Note that spoofProvider is designed to target DroidGuard (com.google.android.gms.unstable) key attestation calls only, and can't (yet) be configured to target other processes with TF. 

## Original PIFork functions removed in TF

• Root hiding needed for a Play Integrity legacy BASIC or higher response. TF doesn't include a function to hide root traces from DroidGuard, so either hiding with the GMS processes in denylist or whitelist hiding must be handled by another module. Note that Denylist should only be used as a *list* for other hiding modules as enforcing (enabling) it will break Zygisk functions of this module.

• Sensitive prop handling for DEVICE+ has been removed, so must be handled separately by another module or script.

• Various script based fixes for some device-specific issues are removed.

• spoofVendingSdk is removed as PI's sdkVersion can simply be spoofed with TF by spoofing an SDK_INT value to vending. Note that the equivalent to spoofVendingSdk 1 is SDK_INT 32 (Android 12L).

• spoofVendingFinger is removed as TF can easily spoof any fingerprint to vending.

<p style="text-align:center;">------------</p>

To use this module you need one of the following combinations:

- [Magisk](https://github.com/topjohnwu/Magisk) with Zygisk enabled or provided by a module, and either DenyList enabled or disabled if using [Shamiko](https://github.com/LSPosed/LSPosed.github.io?tab=readme-ov-file#shamiko) or [Zygisk Assistant](https://github.com/snake-4/Zygisk-Assistant) or [NoHello](https://github.com/MhmRdd/NoHello) module.
- [KernelSU](https://github.com/tiann/KernelSU) with [Zygisk Next](https://github.com/Dr-TSNG/ZygiskNext) or [ReZygisk](https://github.com/PerformanC/ReZygisk) or [NeoZygisk](https://github.com/JingMatrix/NeoZygisk) module.
- [KernelSU Next](https://github.com/KernelSU-Next/KernelSU-Next) with [Zygisk Next](https://github.com/Dr-TSNG/ZygiskNext) or [ReZygisk](https://github.com/PerformanC/ReZygisk) or [NeoZygisk](https://github.com/JingMatrix/NeoZygisk) module.
- [APatch](https://github.com/bmax121/APatch) with [Zygisk Next](https://github.com/Dr-TSNG/ZygiskNext) or [ReZygisk](https://github.com/PerformanC/ReZygisk) or [NeoZygisk](https://github.com/JingMatrix/NeoZygisk) module.

## About module

It injects a classes.dex file to modify fields in the android.os.Build class. Also, it creates a hook in the native code to modify system properties. These are spoofed only in app processes marked in a target list.

It uses robust customised .json or.prop lists and/or a default json or.prop list that target specific app processes defined in a main target list ??


multi-targetng

configuring a target list and either a common prop/field value spoof list or prop/field value spoof lists that target particular processes.



A example, inside in the target.txt write the package name but then create a com.android.vending.json/prop or com.google.android.gms.unstable.json/prop with what FP you want inside, etc.

Also, read the original README.md from PIFork:
https://github.com/osm0sis/PlayIntegrityFork?tab=readme-ov-file#play-integrity-fork

## How use this module

For generally use, customize target packages in /data/adb/targetedfix/config/target.txt & the FP in the /data/adb/targetedfix/config/fix.json/prop to "fix" the FP.

For multi-target with different FPs, customize target packages in /data/adb/targetedfix/config/target.txt, but instead of fix.json/prop rename it to the package name or process of the apps (example com.android.vending.json/prop & com.google.android.gms.unstable.json/prop).
For the GMS, it's the same process with PIFork.
