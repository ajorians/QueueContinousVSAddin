#include <iostream>
#include <string>
#include <fstream>
#include <stdlib.h>//For atoi
#include "TeamCityAPI.h"
#include "Library.h"

#ifdef WIN32
#include <windows.h>
#endif

#ifndef WIN32
#include <unistd.h>//For usleep
#include <limits.h>//For PATH_MAX
#endif

using namespace std;

int main(int argc, char *argv[])
{
   std::string strPath;
#ifdef WIN32
   strPath = "..\\..\\TeamCityAPI\\Debug\\TeamCityAPI.dll";
#else
   char path[PATH_MAX] = "/proc/self/exe";
   char dest[PATH_MAX];
   readlink(path, dest, PATH_MAX);
   std::string strEXE(dest);
   strEXE = strEXE.substr(0, strEXE.rfind('/'));//EXE folder
   strEXE = strEXE.substr(0, strEXE.rfind('/'));//Build folder

   strPath = strEXE + "/TeamCityAPI/libTeamCityAPI.so";

#endif
   RLibrary library(strPath);

   if( !library.Load() )
      return 0;

   TeamCityCreateFunc CreateAPI = (TeamCityCreateFunc)library.Resolve("TeamCityCreate");
   if( !CreateAPI )
      return 0;

   TeamCityAPI pTeamCity = NULL;
   CreateAPI(&pTeamCity, 1);

   ///

   TeamCityQueueBuildFunc QueueBuild = (TeamCityQueueBuildFunc)library.Resolve( "TeamCityQueueBuild" );
   if( !QueueBuild )
      return 0;

   QueueBuild( pTeamCity, "feature/JumpLists");

   ///

   TeamCityFreeFunc FreeAPI = (TeamCityFreeFunc)library.Resolve("TeamCityFree");
   if( !FreeAPI )
      return 0;

   FreeAPI(&pTeamCity );

   return 0;
}

