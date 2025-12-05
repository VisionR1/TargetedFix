# Targeted Fix (TF)
*A fork of PIFork with a different aim.*


[![GitHub release (latest by date)](https://img.shields.io/github/v/release/VisionR1/TargetedFix?label=Release&color=blue&style=flat)](https://github.com/VisionR1/TargetedFix/releases/latest)
[![GitHub Release Date](https://img.shields.io/github/release-date/VisionR1/TargetedFix?label=Release%20Date&color=brightgreen&style=flat)](https://github.com/VisionR1/TargetedFix/releases)
[![GitHub Releases](https://img.shields.io/github/downloads/VisionR1/TargetedFix/latest/total?label=Downloads%20%28Latest%20Release%29&color=blue&style=flat)](https://github.com/VisionR1/TargetedFix/releases/latest)
[![GitHub All Releases](https://img.shields.io/github/downloads/VisionR1/TargetedFix/total?label=Total%20Downloads%20%28All%20Releases%29&color=brightgreen&style=flat)](https://github.com/VisionR1/TargetedFix/releases)

## Purpose

A Zygisk module that features the ability to do **multi-targeted spoofing of different lists of System Property and Build Field values**. Multiple app processes can be targeted by configuring a target list and either a common prop/field value spoof list or prop/field value spoof lists that target particular processes.

This allows a user to spoof almost any prop/field values to app processes other than GMS DroidGuard or Vending, making it extremely versatile.

A common use is spoofing a different device when an app uses device identifiers, eg fingerprint, to limit use to devices on a whitelist.

## spoofProvider retained

The original USNF mechanism, this has been the principal USNF/PIF/PIFork function. It is retained to keep the ability to break key attestation for calls made by DroidGuard. This is essential for 'fixing' legacy Play Integrity DEVICE response verdicts without spoofing leaked OEM HW keys (eg with TrickyStore).

Detail: spoofProvider registers a fake Keystore provider to break these calls with an exception (error), causing DroidGuard (on-device) to fall back to collecting non-attestation-key software signals for inclusion in a PI API token (delivery payload) that is used by Google servers to validate integrity verdicts. This is required to allow legacy DEVICE verdicts in unlocked devices with working Android Platform Key Attestation (Keymaster 3.0+).

Note that spoofProvider is designed to target DroidGuard (com.google.android.gms.unstable) key attestation calls only, and can't (yet) be configured to target other processes with TF. 

## Original PIFork functions *removed* in TF

• Root hiding needed for a Play Integrity legacy BASIC or higher response. TF doesn't include a function to hide root traces from DroidGuard, so either hiding with the GMS processes in denylist or whitelist hiding must be handled by another module. Note that Denylist should only be used as a *list* for other hiding modules as enforcing (enabling) it will break Zygisk functions of this module.

• Sensitive prop handling for DEVICE+ has been removed, so must be handled separately by another module or script.

• Various script based fixes for some device-specific issues are removed.

• spoofVendingSdk is removed as PI's sdkVersion can simply be spoofed with TF by spoofing an SDK_INT value to vending. Note that the equivalent to spoofVendingSdk 1 is SDK_INT 32 (Android 12L).

• spoofVendingFinger is removed as TF can easily spoof any fingerprint to vending.

## Technical Detail

This module injects a classes.dex file to modify fields in the android.os.Build class. Also, it creates a hook in the native code to modify system properties. These are spoofed only in app processes marked in a target list.

It uses a default .prop (or .json) list and/or custom .prop (or .json) lists that target specific app processes defined in the target.txt file for robust multi-targetng of multiple spoof list values.

Please read the original PIFork README.md for other (original) implementation details:
https://github.com/osm0sis/PlayIntegrityFork

## Usage

To use this module you need one of the following combinations:

- [Magisk](https://github.com/topjohnwu/Magisk) with Zygisk enabled or provided by a module, and either DenyList enabled or disabled if using [Shamiko](https://github.com/LSPosed/LSPosed.github.io?tab=readme-ov-file#shamiko) or [Zygisk Assistant](https://github.com/snake-4/Zygisk-Assistant) or [NoHello](https://github.com/MhmRdd/NoHello) module.
- [KernelSU](https://github.com/tiann/KernelSU) with [Zygisk Next](https://github.com/Dr-TSNG/ZygiskNext) or [ReZygisk](https://github.com/PerformanC/ReZygisk) or [NeoZygisk](https://github.com/JingMatrix/NeoZygisk) module.
- [KernelSU Next](https://github.com/KernelSU-Next/KernelSU-Next) with [Zygisk Next](https://github.com/Dr-TSNG/ZygiskNext) or [ReZygisk](https://github.com/PerformanC/ReZygisk) or [NeoZygisk](https://github.com/JingMatrix/NeoZygisk) module.
- [APatch](https://github.com/bmax121/APatch) with [Zygisk Next](https://github.com/Dr-TSNG/ZygiskNext) or [ReZygisk](https://github.com/PerformanC/ReZygisk) or [NeoZygisk](https://github.com/JingMatrix/NeoZygisk) module.

TF configuration files are all created or configured in the /data/adb/modules/targetedfix/config/ directory.

For the purpose of this guide, spoof list files with a .prop format and file extension are indicated, but .json formatted files with the .json extension can be used instead and are compatible too. Note that a .json spoof list file (eg fix.json) will be bypassed if a .prop file (eg fix.prop) with the same name exists. Even if empty, the .prop file will be used by default, so delete or rename .prop files if using the .json file format in their stead.

For basic spoofing of a single list of prop/field values, simply add props and/or fields to **fix.prop**, formatted as required by PIFork, and list app processes in **target.txt**, similar to list in target.txt for Tricky Store. (Note that while Tricky Store treats target items as complete packages, TF treats them as individual processes. Currently, the only way to target all processes in a package is to list them all. Unlike TS, if the main process only is listed, sub processes won't be targeted.)

To spoof unique lists of prop/field values to particular target processes, start simply by adding a confirmation file like fix.prop but named **<process>.prop*** (exactly as the process with a .prop extension). The same process must also be added to the **target.txt** list, but if the correspondingly named .prop file exists, it will be used instead of fix.prop. Multiple spoof lists targeting different processes can be defined this way.

An example:

**com.android.vending.prop** and **com.google.android.gms.unstable.prop** spoof list files could be used (with corresponding processes listed in **target.txt**) to spoof differing prop/field values (eg different fingerprints) to the GMS DroidGuard and vending services.
