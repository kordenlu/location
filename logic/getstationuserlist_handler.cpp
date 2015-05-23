/*
 * getstationuserlist_handler.cpp
 *
 *  Created on: 2015年3月26日
 *      Author: jimm
 */

#include "getstationuserlist_handler.h"
#include "common/common_datetime.h"
#include "common/common_api.h"
#include "frame/frame.h"
#include "frame/server_helper.h"
#include "frame/redissession_bank.h"
#include "frame/cachekey_define.h"
#include "logger/logger.h"
#include "include/control_head.h"
#include "include/typedef.h"
#include "include/location_msg.h"
#include "config/string_config.h"
#include "config/bus_config.h"
#include "server_typedef.h"
#include "bank/redis_bank.h"
#include "bank/mongo_bank.h"
#include "rapidjson/document.h"
using namespace rapidjson;

using namespace LOGGER;
using namespace FRAME;

int32_t CGetStationUserListHandler::GetStationUserList(ICtlHead *pCtlHead, IMsgHead *pMsgHead, IMsgBody *pMsgBody, uint8_t *pBuf, int32_t nBufSize)
{
	ControlHead *pControlHead = dynamic_cast<ControlHead *>(pCtlHead);
	if(pControlHead == NULL)
	{
		return 0;
	}

	MsgHeadCS *pMsgHeadCS = dynamic_cast<MsgHeadCS *>(pMsgHead);
	if(pMsgHeadCS == NULL)
	{
		return 0;
	}

	if(pControlHead->m_nUin != pMsgHeadCS->m_nSrcUin)
	{
		CRedisBank *pRedisBank = (CRedisBank *)g_Frame.GetBank(BANK_REDIS);
		CRedisChannel *pClientRespChannel = pRedisBank->GetRedisChannel(pControlHead->m_nGateRedisAddress, pControlHead->m_nGateRedisPort);

		return CServerHelper::KickUser(pControlHead, pMsgHeadCS, pClientRespChannel, KickReason_NotLogined);
	}

	CGetStationUserListReq *pGetStationUserListReq = dynamic_cast<CGetStationUserListReq *>(pMsgBody);
	if(pGetStationUserListReq == NULL)
	{
		return 0;
	}

	uint64_t nLongtitude;
	uint64_t nLatitude;
	CBusConfig *pBusConfig = (CBusConfig *)g_Frame.GetConfig(CONFIG_BUS);
	pBusConfig->GetStationPosition(pGetStationUserListReq->m_strBusLineID, pGetStationUserListReq->m_strStationName,
			nLongtitude, nLatitude);

	CMongoBank *pMongoBank = (CMongoBank *)g_Frame.GetBank(BANK_MONGO);

	mongoc_client_t *pMongoClient = pMongoBank->GetMongoChannel("location");
	mongoc_collection_t *pCollection = mongoc_client_get_collection(pMongoClient, "location", "user_coord");

	double nMinDistance = (pGetStationUserListReq->m_nMinDistance + 10) / 1000.0;
	bson_t *query = BCON_NEW("geoNear", "user_coord", "near", "[", BCON_DOUBLE(nLongtitude / 1000000.0),
			BCON_DOUBLE(nLatitude / 1000000.0), "]", "minDistance", BCON_DOUBLE(nMinDistance / 6371.0), "maxDistance",
			BCON_DOUBLE(10.0 / 6371.0), "query", "{", "updatetime", "{", "$gt", BCON_INT32(0), "}", "}", "spherical", BCON_BOOL(true),
			"limit", BCON_INT32(pGetStationUserListReq->m_nReqCount));

	mongoc_cursor_t *cursor = mongoc_collection_command(pCollection, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL);

	const Station *pPrevStation = pBusConfig->GetPrevStation(pGetStationUserListReq->m_strBusLineID, pGetStationUserListReq->m_strStationName);
	const Station *pNextStation = pBusConfig->GetNextStation(pGetStationUserListReq->m_strBusLineID, pGetStationUserListReq->m_strStationName);

	CGetStationUserListResp stGetStationUserListResp;
	stGetStationUserListResp.m_nResult = CGetStationUserListResp::enmResult_OK;
	stGetStationUserListResp.m_strPrevStation = (pPrevStation == NULL ? pGetStationUserListReq->m_strStationName : pPrevStation->m_strStationName);
	stGetStationUserListResp.m_strNextStation = (pNextStation == NULL ? pGetStationUserListReq->m_strStationName : pNextStation->m_strStationName);
	stGetStationUserListResp.m_nUserCount = 0;
	stGetStationUserListResp.m_nNearUserCount = 0;

	int32_t nUserCount = 0;
	int32_t nNearUserCount = 0;
	const bson_t *doc;
	while(mongoc_cursor_next(cursor, &doc))
	{
		char *str = bson_as_json(doc, NULL);
		Document document;
		if(document.Parse<0>(str).HasParseError())
		{
			bson_free(str);
			continue;
		}

		if(!document.IsObject())
		{
			bson_free(str);
			continue;
		}

		Value &stResult = document["results"];
		if(!stResult.IsArray())
		{
			bson_free(str);
			continue;
		}

		for(SizeType i = 0; i < stResult.Size(); ++i)
		{
			if(!stResult[i].IsObject())
			{
				break;
			}

			Value &stElement = stResult[i];
			double nDistance = stElement["dis"].GetDouble() * 6371.0;

			Value &stObj = stElement["obj"];
			if(!stObj.IsObject())
			{
				break;
			}

			//大于0.5km
//			if(nDistance * 10 > 5)
//			{
//				stGetStationUserListResp.m_arrNearUserSimpleInfo[nNearUserCount].m_nUin = stObj["uin"].GetUint();
//				stGetStationUserListResp.m_arrNearUserSimpleInfo[nNearUserCount].m_strNickName = stObj["nickname"].GetString();
//				stGetStationUserListResp.m_arrNearUserSimpleInfo[nNearUserCount].m_strHeadImageAddr = stObj["headimageaddr"].GetString();
//				stGetStationUserListResp.m_arrNearUserSimpleInfo[nNearUserCount].m_nAge = stObj["age"].GetInt();
//				stGetStationUserListResp.m_arrNearUserSimpleInfo[nNearUserCount].m_nGender = stObj["gender"].GetInt();
//				stGetStationUserListResp.m_arrNearUserSimpleInfo[nNearUserCount].m_strOneselfWords = stObj["oneselfwords"].GetString();
//
//				++nNearUserCount;
//			}
//			else
			{
				stGetStationUserListResp.m_arrUserSimpleInfo[nUserCount].m_nUin = stObj["uin"].GetUint();
				stGetStationUserListResp.m_arrUserSimpleInfo[nUserCount].m_strNickName = stObj["nickname"].GetString();
				stGetStationUserListResp.m_arrUserSimpleInfo[nUserCount].m_strHeadImageAddr = stObj["headimageaddr"].GetString();
				stGetStationUserListResp.m_arrUserSimpleInfo[nUserCount].m_nAge = stObj["age"].GetInt();
				stGetStationUserListResp.m_arrUserSimpleInfo[nUserCount].m_nGender = stObj["gender"].GetInt();
				stGetStationUserListResp.m_arrUserSimpleInfo[nUserCount].m_strOneselfWords = stObj["oneselfwords"].GetString();
				stGetStationUserListResp.m_arrUserSimpleInfo[nUserCount].m_nDistance = (nDistance * 1000000) / 1000;

				++nUserCount;
			}
		}

		bson_free(str);
	}

	stGetStationUserListResp.m_nUserCount = nUserCount;
	stGetStationUserListResp.m_nNearUserCount = nNearUserCount;

	CRedisBank *pRedisBank = (CRedisBank *)g_Frame.GetBank(BANK_REDIS);
	CRedisChannel *pRespChannel = pRedisBank->GetRedisChannel(pControlHead->m_nGateRedisAddress, pControlHead->m_nGateRedisPort);

	uint8_t arrRespBuf[MAX_MSG_SIZE];

	MsgHeadCS stMsgHeadCS;
	stMsgHeadCS.m_nMsgID = MSGID_GETSTATIONUSERLIST_RESP;
	stMsgHeadCS.m_nSeq = pMsgHeadCS->m_nSeq;
	stMsgHeadCS.m_nSrcUin = pMsgHeadCS->m_nSrcUin;
	stMsgHeadCS.m_nDstUin = pMsgHeadCS->m_nDstUin;

	uint16_t nTotalSize = CServerHelper::MakeMsg(pCtlHead, &stMsgHeadCS, &stGetStationUserListResp, arrRespBuf, sizeof(arrRespBuf));
	pRespChannel->RPush(NULL, CServerHelper::MakeRedisKey(ClientResp::keyname, pControlHead->m_nGateID), (char *)arrRespBuf, nTotalSize);

	g_Frame.Dump(pCtlHead, &stMsgHeadCS, &stGetStationUserListResp, "send ");

	mongoc_collection_destroy(pCollection);
	bson_destroy(query);
	mongoc_cursor_destroy(cursor);

	return 0;
}


