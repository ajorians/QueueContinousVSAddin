#ifndef TEAMCITYAPI_TEAMCITY_H
#define TEAMCITYAPI_TEAMCITY_H

#include <vector>
#include <string>
#include <curl/curl.h>
#include "TeamCityAPI.h"


class TeamCity
{
public:
   TeamCity(bool bVerbose);
   ~TeamCity();

   bool QueueBuild(const std::string& strBranchName, const std::string& strUsername, const std::string& strPassword);

protected:
   

protected:

   bool m_bVerbose;
};


#endif
