/*
 * mongo_config.h
 *
 *  Created on: 2015年3月25日
 *      Author: jimm
 */

#ifndef CONFIG_MONGO_CONFIG_H_
#define CONFIG_MONGO_CONFIG_H_

#include "frame/frame_impl.h"
#include "frame/frame.h"
#include "include/typedef.h"
#include "server_typedef.h"
#include <string.h>

using namespace FRAME;

#define MAX_MONGOSERVER_COUNT		64

struct MongoServerInfo
{
	MongoServerInfo()
	{
		nServerID = 0;
		memset(arrServerURI, 0, sizeof(arrServerURI));
		memset(arrDBName, 0, sizeof(arrDBName));
	}
	int32_t 		nServerID;
	char			arrServerURI[enmMaxInternetAddrLen];
	char			arrDBName[256];
};

class CMongoConfig : public IConfig
{
public:
	CMongoConfig(const char *pConfigName)
	{
		strcpy(m_szConfigFile, pConfigName);
		m_nMongoCount = 0;
	}

	virtual ~CMongoConfig(){};

	//初始化配置
	virtual int32_t Init();
	//卸载配置
	virtual int32_t Uninit();

	virtual int32_t Parser(char *pXMLString);

	int32_t GetMongoServerInfo(MongoServerInfo arrMongoServerInfo[], int32_t nMaxCount);

protected:
	char					m_szConfigFile[enmMaxConfigFileNameSize];
	int32_t				m_nMongoCount;
	MongoServerInfo	m_arrMongoServerInfo[MAX_MONGOSERVER_COUNT];
};


#endif /* CONFIG_MONGO_CONFIG_H_ */
