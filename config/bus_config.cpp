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
REGIST_CONFIG_SAFE(CONFIG_BUS, CBusConfig)

//初始化配置
int32_t CBusConfig::Init()
{
	return 0;
}

//卸载配置
int32_t CBusConfig::Uninit()
{
	map<string, BusLine *>::iterator it = m_stCountry.m_stCityBusInfo.begin();
	for(; it != m_stCountry.m_stCityBusInfo.end();)
	{
		BusLine *pBusLine = it->second;
		map<string, Station *>::iterator jt = pBusLine->m_stBusLineMap.begin();
		for(; jt != pBusLine->m_stBusLineMap.end();)
		{
			delete jt->second;
			pBusLine->m_stBusLineMap.erase(jt++);
		}

		delete it->second;
		m_stCountry.m_stCityBusInfo.erase(it++);
	}

	return 0;
}

int32_t CBusConfig::Parser(char *pXMLString)
{
	TiXmlDocument doc;
	doc.Parse(pXMLString, 0, TIXML_ENCODING_UTF8);

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
			pBusLine->m_strCityName = strCityName;
			pBusLine->m_strCityID = strCityID;
			pBusLine->m_strBusLineName = strBusLineName;
			pBusLine->m_strBusLineID = strBusLineID;
			pBusLine->m_strStartStation = strBusLineStart;
			pBusLine->m_strEndStation = strBusLineEnd;

			m_stCountry.m_stCityBusInfo[strBusLineID] = pBusLine;

			uint16_t nStationIndex = 0;
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
				pStation->m_nStationIndex = nStationIndex;

				++nStationIndex;
				m_stBusLineInfo[strBusLineID].push_back(pStation);

				pBusLine->m_stBusLineMap[strStationName] = pStation;

				pStationNode = pStationNode->NextSiblingElement();
			}
			pBusLineNode = pBusLineNode->NextSiblingElement();
		}
		pAreaNode = pAreaNode->NextSiblingElement();
	}

	return 0;
}

void CBusConfig::GetStationPosition(string strBusLineID, string strStation, uint64_t &nLongtitude, uint64_t &nLatitude)
{
	nLongtitude = 0;
	nLatitude = 0;

	const Station *pStation = GetStation(strBusLineID, strStation);
	if(pStation != NULL)
	{
		nLongtitude = pStation->m_nLongitude;
		nLatitude = pStation->m_nLatitude;
	}
}

const Station *CBusConfig::GetStation(string strBusLineID, string strStation)
{
	map<string, BusLine *>::iterator it = m_stCountry.m_stCityBusInfo.find(strBusLineID);
	if(it != m_stCountry.m_stCityBusInfo.end())
	{
		BusLine *pBusLine = it->second;
		map<string, Station *>::iterator jt = pBusLine->m_stBusLineMap.find(strStation);
		if(jt != pBusLine->m_stBusLineMap.end())
		{
			return jt->second;
		}
	}

	return NULL;
}

const Station *CBusConfig::GetPrevStation(string strBusLineID, string strStation)
{
	const Station *pStation = GetStation(strBusLineID, strStation);
	if(pStation == NULL)
	{
		return NULL;
	}

	map<string, StationList>::iterator it = m_stBusLineInfo.find(strBusLineID);
	if(it != m_stBusLineInfo.end())
	{
		uint16_t nStationIndex = pStation->m_nStationIndex;
		if(pStation->m_nStationIndex <= 0)
		{
			nStationIndex = 0;
		}
		else
		{
			--nStationIndex;
		}

		return it->second.at(nStationIndex);
	}

	return NULL;
}

const Station *CBusConfig::GetNextStation(string strBusLineID, string strStation)
{
	const Station *pStation = GetStation(strBusLineID, strStation);
	if(pStation == NULL)
	{
		return NULL;
	}

	StationList *pStationList = NULL;
	map<string, StationList>::iterator it = m_stBusLineInfo.find(strBusLineID);
	if(it != m_stBusLineInfo.end())
	{
		uint16_t nStationIndex = pStation->m_nStationIndex;
		pStationList = &(it->second);
		if(pStation->m_nStationIndex >= (pStationList->size() - 1))
		{
			nStationIndex = pStationList->size() - 1;
		}
		else
		{
			++nStationIndex;
		}

		return it->second.at(nStationIndex);
	}

	return NULL;
}

