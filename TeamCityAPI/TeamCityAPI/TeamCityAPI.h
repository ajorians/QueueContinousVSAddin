#ifndef TEAMCITYAPI_TEAMCITYAPI_H
#define TEAMCITYAPI_TEAMCITYAPI_H

#ifdef WIN32
#define TEAMCITY_EXTERN	extern "C" __declspec(dllexport)
#else
#define TEAMCITY_EXTERN extern "C"
#endif

typedef void*	TeamCityAPI;

typedef int (*TeamCityCreateFunc)( TeamCityAPI* api, int nVerbose);
typedef int (*TeamCityFreeFunc)( TeamCityAPI* api);
typedef int (*TeamCityQueueBuildFunc)( TeamCityAPI api, const char* pstrBranchName, const char* pstrUsername, const char* pstrPassword );

TEAMCITY_EXTERN int TeamCityCreate( TeamCityAPI* api, int nVerbose);
TEAMCITY_EXTERN int TeamCityFree( TeamCityAPI* api);
TEAMCITY_EXTERN int TeamCityQueueBuild( TeamCityAPI api, const char* pstrBranchName, const char* pstrUsername, const char* pstrPassword );

#endif
