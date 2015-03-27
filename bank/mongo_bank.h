/*
 * mongo_bank.h
 *
 *  Created on: 2015年3月25日
 *      Author: jimm
 */

#ifndef BANK_MONGO_BANK_H_
#define BANK_MONGO_BANK_H_

#include "../../frame/frame_impl.h"
#include <mongoc.h>
#include <bson.h>
#include <string>
#include <map>
using namespace std;
using namespace FRAME;

#define MAX_REDISHASH_COUNT		1024

class CMongoBank : public IBank
{
	typedef map<string, mongoc_client_t *>	MongoServerMap;
public:
	CMongoBank()
	{
		m_nHashTableSize = 0;
	}

	//初始化
	virtual int32_t Init();
	//卸载
	virtual int32_t Uninit();
	//获取一个redis对象，目前采取对目标uin hash到方案
	mongoc_client_t *GetMongoChannel(uint32_t key);
	//根据cache key获取redis对象
	mongoc_client_t *GetMongoChannel(string strKey);
protected:
	MongoServerMap		m_stMongoServerMap;
	int32_t				m_nHashTableSize;
	mongoc_client_t	*m_arrMongoHashTable[MAX_REDISHASH_COUNT];
};


#endif /* BANK_MONGO_BANK_H_ */
