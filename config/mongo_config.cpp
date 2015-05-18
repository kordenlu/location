/*
 * mongo_config.cpp
 *
 *  Created on: 2015年3月25日
 *      Author: jimm
 */

#include "mongo_config.h"
#include "logger/logger.h"
#include "tinyxml/tinyxml.h"
#include "server_typedef.h"

using namespace LOGGER;

//注册到配置管理器
REGIST_CONFIG_SAFE(CONFIG_MONGO, CMongoConfig)

//初始化配置
int32_t CMongoConfig::Init()
{
	return 0;
}

//卸载配置
int32_t CMongoConfig::Uninit()
{
	return 0;
}

int32_t CMongoConfig::Parser(char *pXMLString)
{
	TiXmlDocument doc;
	doc.Parse(pXMLString, 0, TIXML_ENCODING_UTF8);

	//获取根节点
	TiXmlElement *pRoot = doc.RootElement();
	if (NULL == pRoot)
	{
		WRITE_WARN_LOG(SERVER_NAME, "%s is not found mongo node!\n", m_szConfigFile);
		return 1;
	}

	const char* pszValue = NULL;

	TiXmlElement *pNode = pRoot->FirstChildElement("node");
	if(NULL == pNode)
	{
		WRITE_WARN_LOG(SERVER_NAME, "%s is not found node!\n", m_szConfigFile);
		return 1;
	}

	while(pNode != NULL)
	{
		int32_t nServerID = 0;
		pszValue = pNode->Attribute("server_id", &nServerID);
		if(NULL == pszValue)
		{
			WRITE_WARN_LOG(SERVER_NAME, "%s is not found server_id node!\n", m_szConfigFile);
			return 1;
		}

		char arrServerURI[128];
		memset(arrServerURI, 0, sizeof(arrServerURI));

		pszValue = pNode->Attribute("server_uri");
		if(NULL == pszValue)
		{
			WRITE_WARN_LOG(SERVER_NAME, "%s is not found server_uri node!\n", m_szConfigFile);
			return 1;
		}

		strcpy(arrServerURI, pszValue);

		char arrDBName[256];
		memset(arrDBName, 0, sizeof(arrDBName));

		pszValue = pNode->Attribute("dbname");
		if(NULL == pszValue)
		{
			WRITE_WARN_LOG(SERVER_NAME, "%s is not found dbname node!\n", m_szConfigFile);
			return 1;
		}

		strcpy(arrDBName, pszValue);

		m_arrMongoServerInfo[m_nMongoCount].nServerID = nServerID;
		strcpy(m_arrMongoServerInfo[m_nMongoCount].arrServerURI, arrServerURI);
		strcpy(m_arrMongoServerInfo[m_nMongoCount].arrDBName, arrDBName);
		++m_nMongoCount;

		pNode = pNode->NextSiblingElement();
	}

	return 0;
}

int32_t CMongoConfig::GetMongoServerInfo(MongoServerInfo arrMongoServerInfo[], int32_t nMaxCount)
{
	int32_t nCount = 0;
	for(int32_t i = 0; i < m_nMongoCount; ++i)
	{
		arrMongoServerInfo[nCount++] = m_arrMongoServerInfo[i];
		if(nCount >= nMaxCount - 1)
		{
			break;
		}
	}

	return nCount;
}

