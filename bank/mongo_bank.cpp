/*
 * mongo_bank.cpp
 *
 *  Created on: 2015年3月25日
 *      Author: jimm
 */

#include "mongo_bank.h"
#include "../../frame/frame.h"
#include "../config/mongo_config.h"
#include "../server_typedef.h"
#include "../dispatch/subscribe_channel.h"

using namespace FRAME;

REGIST_BANK(BANK_MONGO, CMongoBank)

//初始化
int32_t CMongoBank::Init()
{
	CMongoConfig *pMongoConfig = (CMongoConfig *)g_Frame.GetConfig(CONFIG_MONGO);

	int32_t nMongoCount = 0;
	MongoServerInfo arrMongoServerInfo[MAX_MONGOSERVER_COUNT];
	nMongoCount = pMongoConfig->GetMongoServerInfo(arrMongoServerInfo, MAX_MONGOSERVER_COUNT);

	mongoc_init();

	for(int32_t i = 0; i < nMongoCount; ++i)
	{
		mongoc_client_t *client = mongoc_client_new (arrMongoServerInfo[i].arrServerURI);
		m_stMongoServerMap[arrMongoServerInfo[i].arrDBName] = client;
		m_arrMongoHashTable[m_nHashTableSize++] = client;
	}

	return 0;
}

//卸载
int32_t CMongoBank::Uninit()
{
	map<string, mongoc_client_t *>::iterator it = m_stMongoServerMap.begin();
	for(; it != m_stMongoServerMap.end(); ++it)
	{
		mongoc_client_destroy(it->second);
	}

	mongoc_cleanup();

	m_stMongoServerMap.clear();
	return 0;
}

//获取一个redis对象，目前采取对目标uin hash到方案
mongoc_client_t *CMongoBank::GetMongoChannel(uint32_t key)
{
	if(m_nHashTableSize <= 0)
	{
		return NULL;
	}

	return m_arrMongoHashTable[key % m_nHashTableSize];
}

//根据cache key获取redis对象
mongoc_client_t *CMongoBank::GetMongoChannel(string strKey)
{
	mongoc_client_t *pMongoChannel = NULL;
	map<string, mongoc_client_t *>::iterator it = m_stMongoServerMap.find(strKey);
	if(it != m_stMongoServerMap.end())
	{
		pMongoChannel = it->second;
	}

	return pMongoChannel;
}

