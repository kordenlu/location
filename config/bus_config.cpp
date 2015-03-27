/*
 * buf_config.cpp
 *
 *  Created on: Mar 19, 2015
 *      Author: jimm
 */

#include "bus_config.h"
#include "../../common/common_api.h"
#include "../../logger/logger.h"
#include "../../tinyxml/tinyxml.h"
#include "../server_typedef.h"

using namespace LOGGER;

//注册到配置管理器
REGIST_CONFIG(CONFIG_BUS, CBusConfig)

//初始化配置
int32_t CBusConfig::Init()
{
	TiXmlDocument doc(m_szConfigFile);
	if (!doc.LoadFile(TIXML_ENCODING_UTF8))
	{
		WRITE_WARN_LOG(SERVER_NAME, "%s is not format utf8!\n", m_szConfigFile);
		return 1;
	}

	//获取根节点
	TiXmlElement *pRoot = doc.RootElement();
	if (NULL == pRoot)
	{
		WRITE_WARN_LOG(SERVER_NAME, "%s is not found server node!\n", m_szConfigFile);
		return 1;
	}

	const char* pszValue = NULL;

	TiXmlElement *pAreaNode = pRoot->FirstChildElement("area");
	if(NULL == pAreaNode)
	{
		WRITE_WARN_LOG(SERVER_NAME, "%s is not found area!\n", m_szConfigFile);
		return 1;
	}

	while(pAreaNode != NULL)
	{
		pszValue = pAreaNode->Attribute("name");
		if(NULL == pszValue)
		{
			WRITE_WARN_LOG(SERVER_NAME, "%s is not found name node!\n", m_szConfigFile);
			return 1;
		}

		string strCityName = string(pszValue);

		pszValue = pAreaNode->Attribute("id");
		if(NULL == pszValue)
		{
			WRITE_WARN_LOG(SERVER_NAME, "%s is not found id node!{area=%s}\n", m_szConfigFile, strCityName.c_str());
			return 1;
		}

		string strCityID = string(pszValue);

		CityInfo *pCityInfo = new CityInfo();
		pCityInfo->m_strCityName = strCityName;
		pCityInfo->m_strCityID = strCityID;

		m_stCountry.m_stCityInfo[strCityName] = pCityInfo;

		TiXmlElement *pBusLineNode = pAreaNode->FirstChildElement("busline");
		while(pBusLineNode != NULL)
		{
			pszValue = pBusLineNode->Attribute("name");
			if(NULL == pszValue)
			{
				WRITE_WARN_LOG(SERVER_NAME, "%s is not found name node!{area=%s}\n", m_szConfigFile, strCityName.c_str());
				return 1;
			}

			string strBusLineName = string(pszValue);

			pszValue = pBusLineNode->Attribute("id");
			if(NULL == pszValue)
			{
				WRITE_WARN_LOG(SERVER_NAME, "%s is not found id node!{area=%s, busline=%s}\n", m_szConfigFile, strCityName.c_str(),
						strBusLineName.c_str());
				return 1;
			}

			string strBusLineID = string(pszValue);

			pszValue = pBusLineNode->Attribute("start");
			if(NULL == pszValue)
			{
				WRITE_WARN_LOG(SERVER_NAME, "%s is not found start node!{area=%s, busline=%s}\n", m_szConfigFile, strCityName.c_str(),
						strBusLineName.c_str());
				return 1;
			}

			string strBusLineStart = string(pszValue);

			pszValue = pBusLineNode->Attribute("end");
			if(NULL == pszValue)
			{
				WRITE_WARN_LOG(SERVER_NAME, "%s is not found end node!{area=%s, busline=%s}\n", m_szConfigFile, strCityName.c_str(),
						strBusLineName.c_str());
				return 1;
			}

			string strBusLineEnd = string(pszValue);

			BusLine *pBusLine = new BusLine();
			pBusLine->m_strBusLineName = strBusLineName;
			pBusLine->m_strBusLineID = strBusLineID;
			pBusLine->m_strStartStation = strBusLineStart;
			pBusLine->m_strEndStation = strBusLineEnd;

			pCityInfo->m_stCityBusInfo[strBusLineID] = pBusLine;

			TiXmlElement *pStationNode = pBusLineNode->FirstChildElement("station");
			while(pStationNode != NULL)
			{
				pszValue = pStationNode->Attribute("name");
				if(NULL == pszValue)
				{
					WRITE_WARN_LOG(SERVER_NAME, "%s is not found name node!{area=%s, busline=%s}\n", m_szConfigFile,
							strCityName.c_str(), strBusLineName.c_str());
					return 1;
				}

				string strStationName = string(pszValue);

				pszValue = pStationNode->Attribute("id");
				if(NULL == pszValue)
				{
					WRITE_WARN_LOG(SERVER_NAME, "%s is not found id node!{area=%s, busline=%s, station=%s}\n", m_szConfigFile,
							strCityName.c_str(), strBusLineName.c_str(), strStationName.c_str());
					return 1;
				}

				string strStationID = string(pszValue);

				pszValue = pStationNode->Attribute("longitude");
				if(NULL == pszValue)
				{
					WRITE_WARN_LOG(SERVER_NAME, "%s is not found longitude node!{area=%s, busline=%s, station=%s}\n", m_szConfigFile,
							strCityName.c_str(), strBusLineName.c_str(), strStationName.c_str());
					return 1;
				}

				string strLongitude = string(pszValue);

				pszValue = pStationNode->Attribute("latitude");
				if(NULL == pszValue)
				{
					WRITE_WARN_LOG(SERVER_NAME, "%s is not found latitude node!{area=%s, busline=%s, station=%s}\n", m_szConfigFile,
							strCityName.c_str(), strBusLineName.c_str(), strStationName.c_str());
					return 1;
				}

				string strLatitude = string(pszValue);

				Station *pStation = new Station();
				pStation->m_strStationName = strStationName;
				pStation->m_strStationID = strStationID;
				pStation->m_nLongitude = atoi64(strLongitude.c_str());
				pStation->m_nLatitude = atoi64(strLatitude.c_str());

				pBusLine->m_stBusLineMap[strStationName] = pStation;

				pStationNode = pStationNode->NextSiblingElement();
			}
			pBusLineNode = pBusLineNode->NextSiblingElement();
		}
		pAreaNode = pAreaNode->NextSiblingElement();
	}

	return 0;
}

//卸载配置
int32_t CBusConfig::Uninit()
{
	map<string, CityInfo *>::iterator it = m_stCountry.m_stCityInfo.begin();
	for(; it != m_stCountry.m_stCityInfo.end();)
	{
		CityInfo *pCityInfo = it->second;
		map<string, BusLine *>::iterator et = pCityInfo->m_stCityBusInfo.begin();
		for(; et != pCityInfo->m_stCityBusInfo.end();)
		{
			BusLine *pBusLine = et->second;
			map<string, Station *>::iterator jt = pBusLine->m_stBusLineMap.begin();
			for(; jt != pBusLine->m_stBusLineMap.end();)
			{
				delete jt->second;
				pBusLine->m_stBusLineMap.erase(jt++);
			}

			delete et->second;
			pCityInfo->m_stCityBusInfo.erase(et++);
		}

		delete it->second;
		m_stCountry.m_stCityInfo.erase(it++);
	}

	return 0;
}

void CBusConfig::GetStationPosition(string strCityName, string strBusLineID, string strStation, uint64_t &nLongtitude, uint64_t &nLatitude)
{
	nLongtitude = 0;
	nLatitude = 0;

	map<string, CityInfo *>::iterator it = m_stCountry.m_stCityInfo.find(strCityName);
	if(it != m_stCountry.m_stCityInfo.end())
	{
		CityInfo *pCityInfo = it->second;
		map<string, BusLine *>::iterator et = pCityInfo->m_stCityBusInfo.find(strBusLineID);
		if(et != pCityInfo->m_stCityBusInfo.end())
		{
			BusLine *pBusLine = et->second;
			map<string, Station *>::iterator jt = pBusLine->m_stBusLineMap.find(strStation);
			if(jt != pBusLine->m_stBusLineMap.end())
			{
				nLongtitude = jt->second->m_nLongitude;
				nLatitude = jt->second->m_nLatitude;
			}
		}
	}
}



