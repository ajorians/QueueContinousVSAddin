#include "TeamCity.h"
#include "TeamCityAPI.h"

#include <cassert>
#include <iostream>

using namespace std;

#ifndef SAFE_DELETE
#define SAFE_DELETE(x) { delete (x); (x) = NULL; }
#endif

#ifndef ARR_SIZE
#define ARR_SIZE(x) sizeof(x)/sizeof(x[0])
#endif

//#ifdef _MSC_VER 
////not #if defined(_WIN32) || defined(_WIN64) because we have strncasecmp in mingw
//#define strncasecmp _strnicmp
//#define strcasecmp _stricmp
//#endif

TEAMCITY_EXTERN int TeamCityCreate(TeamCityAPI* api, int nVerbose)
{
   *api = new TeamCity(nVerbose==1);

   return 0;
}

TEAMCITY_EXTERN int FlowdockFree( TeamCityAPI *api)
{
   TeamCity* pTeamCity = (TeamCity*)*api;
   delete pTeamCity;
   return 0;
}

TEAMCITY_EXTERN int TeamCityQueueBuild( TeamCityAPI api, const char* pstrBranchName, const char* pstrUsername, const char* pstrPassword )
{
   std::string strBranchname( pstrBranchName ), strUsername( pstrUsername ), strPassword( pstrPassword );
   TeamCity* pTeamCity = (TeamCity*)api;
   return pTeamCity->QueueBuild( strBranchname, strUsername, strPassword );
}

TeamCity::TeamCity(bool bVerbose)
{
   curl_global_init(CURL_GLOBAL_ALL);
}

TeamCity::~TeamCity()
{
   curl_global_cleanup();
}

int TeamCity::QueueBuild( const std::string& strBranchName, const std::string& strUsername, const std::string& strPassword )
{
   CURL *curl;
   CURLcode res;

   std::string strURL = "http://teamcity.techsmith.com/app/rest/buildQueue";

   curl = curl_easy_init();
   if( !curl )
      return -1;

   curl_easy_setopt(curl, CURLOPT_URL, strURL.c_str());

   std::string ctype_header = "Content-Type:application/xml";

   std::string data = "<build personal=\"true\" branchName=\"";
   data += strBranchName;
   data += "\">\
    <buildType id=\"Camtasia_Continuous\"/>\
    <comment><text>Making a build</text></comment>\
</build>";

   curl_easy_setopt(curl, CURLOPT_USERAGENT, "ajclient/0.0.1");
   curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
   std::string strUserPass = strUsername + std::string( ":") + strPassword;
   curl_easy_setopt(curl, CURLOPT_USERPWD, strUserPass.c_str());
   curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

   curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
   curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

   curl_easy_setopt(curl, CURLOPT_POST, 1L);
   curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
   curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, data.size());

   curl_slist* header = NULL;
   header = curl_slist_append(header, ctype_header.c_str());
   curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);

   res = curl_easy_perform(curl);

   long http_code = 0;
   curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

   curl_easy_cleanup(curl);

   return static_cast<int>( http_code );
}






