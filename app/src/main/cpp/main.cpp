#include <android/log.h>
#include <sys/system_properties.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <sstream>
#include <cctype>

#include "zygisk.hpp"
#include "json.hpp"
#include "dobby.h"

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, "TFIX/Native", __VA_ARGS__)

#define DEX_FILE_PATH "/data/adb/modules/targetedfix/classes.dex"

#define JSON_FILE_PATH "/data/adb/modules/targetedfix/config/fix.json"
#define TARGET_LIST_PATH "/data/adb/modules/targetedfix/config/target.txt"

static int verboseLogs = 0;
static int spoofBuild = 1;
static int spoofProps = 1;
static int spoofProvider = 1;
static int spoofSignature = 0;

static std::map<std::string, std::string> jsonProps;

typedef void (*T_Callback)(void *, const char *, const char *, uint32_t);

static std::map<void *, T_Callback> callbacks;

static void modify_callback(void *cookie, const char *name, const char *value, uint32_t serial) {
    if (cookie == nullptr || name == nullptr || value == nullptr || !callbacks.contains(cookie)) return;

    const char *oldValue = value;
	
    std::string prop(name);

    if (jsonProps.count(prop)) {
        // Exact property match
        value = jsonProps[prop].c_str();
    } else {
        // Leading * wildcard property match
        for (const auto &p: jsonProps) {
            if (p.first.starts_with("*") && prop.ends_with(p.first.substr(1))) {
                value = p.second.c_str();
                break;
            }
        }
    }

    if (oldValue == value) {
        if (verboseLogs > 99) LOGD("[%s]: %s (unchanged)", name, oldValue);
    } else {
        LOGD("[%s]: %s -> %s", name, oldValue, value);
    }

    return callbacks[cookie](cookie, name, value, serial);
}

static void (*o_system_property_read_callback)(const prop_info *, T_Callback, void *);

static void my_system_property_read_callback(const prop_info *pi, T_Callback callback, void *cookie) {
    if (pi == nullptr || callback == nullptr || cookie == nullptr) {
        return o_system_property_read_callback(pi, callback, cookie);
    }
    callbacks[cookie] = callback;
    return o_system_property_read_callback(pi, modify_callback, cookie);
}

static void doHook() {
    void *handle = DobbySymbolResolver(nullptr, "__system_property_read_callback");
    if (handle == nullptr) {
        LOGD("Couldn't find '__system_property_read_callback' handle");
        return;
    }
    LOGD("Found '__system_property_read_callback' handle at %p", handle);
    DobbyHook(handle, reinterpret_cast<dobby_dummy_func_t>(my_system_property_read_callback),
        reinterpret_cast<dobby_dummy_func_t *>(&o_system_property_read_callback));
}

class TargetedFix : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api *api, JNIEnv *env) override {
        this->api = api;
        this->env = env;
    }

    void preAppSpecialize(zygisk::AppSpecializeArgs *args) override {
        bool shouldSpoof = false;
		
        auto rawProcess = env->GetStringUTFChars(args->nice_name, nullptr);
        auto rawDir = env->GetStringUTFChars(args->app_data_dir, nullptr);
        
        // Prevent crash on apps with no data dir
        if (rawDir == nullptr) {
            env->ReleaseStringUTFChars(args->nice_name, rawProcess);
            api->setOption(zygisk::DLCLOSE_MODULE_LIBRARY);
            return;
        }

        pkgName = rawProcess;
        std::string_view process(rawProcess);

        long dexSize = 0, jsonSize = 0, targetSize = 0;
		
        int fd = api->connectCompanion();

        // Send package name to companion
        std::string processStr(process);
        long processSize = processStr.size();
        write(fd, &processSize, sizeof(long));
        write(fd, processStr.data(), processSize);
        
        // Read file sizes from companion
        read(fd, &dexSize, sizeof(long));
        read(fd, &jsonSize, sizeof(long));
        read(fd, &targetSize, sizeof(long));

        if (dexSize < 1 || jsonSize < 1) {
            close(fd);
            LOGD("Couldn't read required files");
            api->setOption(zygisk::DLCLOSE_MODULE_LIBRARY);
            return;
        }

        dexVector.resize(dexSize);
        read(fd, dexVector.data(), dexSize);

        jsonVector.resize(jsonSize);
        read(fd, jsonVector.data(), jsonSize);

        targetVector.resize(targetSize);
        read(fd, targetVector.data(), targetSize);
        
        close(fd);

        parseTargetVector();

        if (isTargetPackage(process)) {
            shouldSpoof = true;
        }

        env->ReleaseStringUTFChars(args->nice_name, rawProcess);
        env->ReleaseStringUTFChars(args->app_data_dir, rawDir);

        if (!shouldSpoof) {
            api->setOption(zygisk::DLCLOSE_MODULE_LIBRARY);
            return;
        }
        
        // We are in TargetPackage now, force unmount
        api->setOption(zygisk::FORCE_DENYLIST_UNMOUNT);
        
        std::string jsonString(jsonVector.cbegin(), jsonVector.cend());
        json = nlohmann::json::parse(jsonString, nullptr, false, true);

        jsonVector.clear();
    }

    void postAppSpecialize(const zygisk::AppSpecializeArgs *args) override {
        if (dexVector.empty() || json.empty()) return;

        readJson();

        if (spoofProps > 0) doHook();
        if (spoofBuild + spoofProvider + spoofSignature > 0) inject();

        dexVector.clear();
        json.clear();
        targetVector.clear();
        targetPackages.clear();
    }

    void preServerSpecialize(zygisk::ServerSpecializeArgs *args) override {
        api->setOption(zygisk::DLCLOSE_MODULE_LIBRARY);
    }

private:
    zygisk::Api *api = nullptr;
    JNIEnv *env = nullptr;
    std::vector<char> dexVector;
    std::vector<char> jsonVector;
    std::vector<char> targetVector;
    nlohmann::json json;
    std::string pkgName;
	std::vector<std::string> targetPackages;

    void parseTargetVector() {
        if (targetVector.empty()) {
            LOGD("Received empty target list from companion.");
            return;
        }
        std::string content(targetVector.begin(), targetVector.end());
        std::stringstream ss(content);
        std::string line;
        while (std::getline(ss, line)) {
            std::string trimmedLine = "";
            for (char c : line) {
                if (!std::isspace(c)) {
                    trimmedLine += c;
                }
            }
            if (!trimmedLine.empty() && trimmedLine[0] != '#') {
                std::string finalTrimmedLine = "";
                int lastNonSpace = trimmedLine.length() - 1;
                while (lastNonSpace >= 0 && std::isspace(trimmedLine[lastNonSpace])) {
                    lastNonSpace--;
                }
                if (lastNonSpace >= 0) {
                    finalTrimmedLine = trimmedLine.substr(0, lastNonSpace + 1);
                } else {
                    finalTrimmedLine = trimmedLine;
                }
                targetPackages.push_back(finalTrimmedLine);
            }
        }
    }
    
    bool isTargetPackage(std::string_view process) {
        std::string processStr(process);
        for (const auto &pkg : targetPackages) {
            if (pkg == processStr) {
                return true;
            }
        }
        return false;
    }

    void readJson() {
        LOGD("JSON contains %d keys!", static_cast<int>(json.size()));
        
        // Verbose logging level
        if (json.contains("verboseLogs")) {
            if (!json["verboseLogs"].is_null() && json["verboseLogs"].is_string() && json["verboseLogs"] != "") {
                verboseLogs = stoi(json["verboseLogs"].get<std::string>());
                if (verboseLogs > 0) LOGD("Verbose logging (level %d) enabled!", verboseLogs);
            } else {
                LOGD("Error parsing verboseLogs!");
            }
            json.erase("verboseLogs");
        }
        
        // Advanced spoofing settings
        if (json.contains("spoofBuild")) {
            if (!json["spoofBuild"].is_null() && json["spoofBuild"].is_string() && json["spoofBuild"] != "") {
                spoofBuild = stoi(json["spoofBuild"].get<std::string>());
                if (verboseLogs > 0) LOGD("Spoofing Build Fields %s!", (spoofBuild > 0) ? "enabled" : "disabled");
            } else {
                LOGD("Error parsing spoofBuild!");
            }
            json.erase("spoofBuild");
        }
        if (json.contains("spoofProps")) {
            if (!json["spoofProps"].is_null() && json["spoofProps"].is_string() && json["spoofProps"] != "") {
                spoofProps = stoi(json["spoofProps"].get<std::string>());
                if (verboseLogs > 0) LOGD("Spoofing System Properties %s!", (spoofProps > 0) ? "enabled" : "disabled");
            } else {
                LOGD("Error parsing spoofProps!");
            }
            json.erase("spoofProps");
        }
        if (json.contains("spoofProvider")) {
            if (!json["spoofProvider"].is_null() && json["spoofProvider"].is_string() && json["spoofProvider"] != "") {
                spoofProvider = stoi(json["spoofProvider"].get<std::string>());
                if (verboseLogs > 0) LOGD("Spoofing Keystore Provider %s!", (spoofProvider > 0) ? "enabled" : "disabled");
            } else {
                LOGD("Error parsing spoofProvider!");
            }
            json.erase("spoofProvider");
        }
        if (json.contains("spoofSignature")) {
            if (!json["spoofSignature"].is_null() && json["spoofSignature"].is_string() && json["spoofSignature"] != "") {
                spoofSignature = stoi(json["spoofSignature"].get<std::string>());
                if (verboseLogs > 0) LOGD("Spoofing ROM Signature %s!", (spoofSignature > 0) ? "enabled" : "disabled");
            } else {
                LOGD("Error parsing spoofSignature!");
            }
            json.erase("spoofSignature");
        }

        std::vector<std::string> eraseKeys;
        for (auto &jsonList: json.items()) {
            if (verboseLogs > 1) LOGD("Parsing %s", jsonList.key().c_str());
            if (jsonList.key().find_first_of("*.") != std::string::npos) {
                // Name contains . or * (wildcard) so assume real property name
                if (!jsonList.value().is_null() && jsonList.value().is_string()) {
                    if (jsonList.value() == "") {
                        LOGD("%s is empty, skipping", jsonList.key().c_str());
                    } else {
                        if (verboseLogs > 0) LOGD("Adding '%s' to properties list", jsonList.key().c_str());
                        jsonProps[jsonList.key()] = jsonList.value();
                    }
                } else {
                    LOGD("Error parsing %s!", jsonList.key().c_str());
                }
                eraseKeys.push_back(jsonList.key());
            }
        }
        // Remove properties from parsed JSON
        for (auto key: eraseKeys) {
            if (json.contains(key)) json.erase(key);
        }
    }

    void inject() {
        const char* niceName = "APP";

        LOGD("JNI %s: Getting system classloader", niceName);
        auto clClass = env->FindClass("java/lang/ClassLoader");
        auto getSystemClassLoader = env->GetStaticMethodID(clClass, "getSystemClassLoader", "()Ljava/lang/ClassLoader;");
        auto systemClassLoader = env->CallStaticObjectMethod(clClass, getSystemClassLoader);

        LOGD("JNI %s: Creating module classloader", niceName);
        auto dexClClass = env->FindClass("dalvik/system/InMemoryDexClassLoader");
        auto dexClInit = env->GetMethodID(dexClClass, "<init>", "(Ljava/nio/ByteBuffer;Ljava/lang/ClassLoader;)V");
        auto buffer = env->NewDirectByteBuffer(dexVector.data(), static_cast<jlong>(dexVector.size()));
        auto dexCl = env->NewObject(dexClClass, dexClInit, buffer, systemClassLoader);

        LOGD("JNI %s: Loading module class", niceName);
        auto loadClass = env->GetMethodID(clClass, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");        
        const char* className = "es.chiteroman.playintegrityfix.EntryPoint";
        auto entryClassName = env->NewStringUTF(className);
        auto entryClassObj = env->CallObjectMethod(dexCl, loadClass, entryClassName);
		
        auto entryClass = (jclass) entryClassObj;

        LOGD("JNI %s: Sending JSON", niceName);
        auto receiveJson = env->GetStaticMethodID(entryClass, "receiveJson", "(Ljava/lang/String;)V");
        auto javaStr = env->NewStringUTF(json.dump().c_str());
        env->CallStaticVoidMethod(entryClass, receiveJson, javaStr);

        LOGD("JNI %s: Calling EntryPoint.init", niceName);
        auto entryInit = env->GetStaticMethodID(entryClass, "init", "(IIII)V");
        env->CallStaticVoidMethod(entryClass, entryInit, verboseLogs, spoofBuild, spoofProvider, spoofSignature);
    }
};

static void companion(int fd) {
    long dexSize = 0, jsonSize = 0, targetSize = 0;
    std::vector<char> dexVector, jsonVector, targetVector;
    
    // Receive package name to enable per-app JSON
    std::string processName;
    long processSize = 0;
    read(fd, &processSize, sizeof(long));
    processName.resize(processSize);
    read(fd, processName.data(), processSize);

    // Replace ':' with '.' for subprocess filename consistency
    std::replace(processName.begin(), processName.end(), ':', '.');

    // --- File Reading Logic ---
    FILE *dex = fopen(DEX_FILE_PATH, "rb");
	
    if (dex) {
        fseek(dex, 0, SEEK_END);
        dexSize = ftell(dex);
        fseek(dex, 0, SEEK_SET);
		
        dexVector.resize(dexSize);
        fread(dexVector.data(), 1, dexSize, dex);
		
        fclose(dex);
    }

    FILE *json = nullptr;
    std::string customJsonPath = "/data/adb/modules/targetedfix/config/" + processName + ".json";
    json = fopen(customJsonPath.c_str(), "r");

    if (!json) {
        json = fopen(JSON_FILE_PATH, "r");
    }

    if (json) {
        fseek(json, 0, SEEK_END);
        jsonSize = ftell(json);
        fseek(json, 0, SEEK_SET);
		
        jsonVector.resize(jsonSize);
        fread(jsonVector.data(), 1, jsonSize, json);
		
        fclose(json);
    }

    FILE *target = fopen(TARGET_LIST_PATH, "r");
    if (target) {
        fseek(target, 0, SEEK_END);
        targetSize = ftell(target);
        fseek(target, 0, SEEK_SET);
		
        targetVector.resize(targetSize);
        fread(targetVector.data(), 1, targetSize, target);
		
        fclose(target);
    }

    write(fd, &dexSize, sizeof(long));
    write(fd, &jsonSize, sizeof(long));
    write(fd, &targetSize, sizeof(long));

    write(fd, dexVector.data(), dexSize);
    write(fd, jsonVector.data(), jsonSize);
    write(fd, targetVector.data(), targetSize);
}

REGISTER_ZYGISK_MODULE(TargetedFix)

REGISTER_ZYGISK_COMPANION(companion)
