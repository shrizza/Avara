#pragma once

#include "Types.h"
#include "PlayerConfig.h"
#include "SoundSystemDefines.h"

#include <string>
#include <vector>
#include <json.hpp>

// path separator
#if defined(_WIN32)
#define PATHSEP "\\"
#else
#define PATHSEP "/"
#endif

// files stuff
#define LEVELDIR "levels"
#define SETFILE "set.json"
#define ALFDIR "alf"
#define BSPSDIR "bsps"
#define BSPSEXT ".json"
#define DEFAULTSCRIPT "default.avarascript"
#define OGGDIR "ogg"
#define WAVDIR "wav"

void UseResFile(std::string filename);
void UseBaseFolder(std::string folder);
void AddExternalPackage(std::string folder);
void ClearExternalPackages();
void UseLevelFolder(std::string folder);
std::string OSTypeString(OSType t);
OSType StringOSType(std::string s);
Handle GetResource(OSType theType, short theID);
Handle GetNamedResource(OSType theType, std::string name);

void WriteResource(Handle theResource);
void ChangedResource(Handle theResource);
void ReleaseResource(Handle theResource);
void DetachResource(Handle theResource);

void BundlePath(const char *rel, char *dest);
void BundlePath(std::stringstream &buffa, char *dest);

nlohmann::json LoadLevelListFromJSON(std::string set);
nlohmann::json GetManifestJSON(std::string set);
nlohmann::json GetKeyFromSetJSON(std::string rsrc, std::string key, std::string default_id);

void LoadDefaultOggFiles();
void LoadLevelOggFiles(std::string set);
void LoadOggFile(short resId, const char* filename);
SampleHeaderHandle LoadSampleHeaderFromSetJSON(short resId, SampleHeaderHandle sampleList);
