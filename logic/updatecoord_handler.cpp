/*
 * updatecoord_handler.cpp
 *
 *  Created on: 2015年3月26日
 *      Author: jimm
 */

#include "updatecoord_handler.h"
#include "../../common/common_datetime.h"
#include "../../common/common_api.h"
#include "../../frame/frame.h"
#include "../../frame/server_helper.h"
#include "../../frame/redissession_bank.h"
#include "../../logger/logger.h"
#include "../../include/cachekey_define.h"
#include "../../include/dbkey_define.h"
#include "../../include/control_head.h"
#include "../../include/typedef.h"
#include "../../include/location_msg.h"
#include "../config/msgdispatch_config.h"
#include "../config/string_config.h"
#include "../config/bus_config.h"
#include "../server_typedef.h"
#include "../bank/redis_bank.h"
#include "../bank/mongo_bank.h"

using namespace LOGGER;
using namespace FRAME;

int32_t CUpdateCoordHandler::UpdateCoord(ICtlHead *pCtlHead, IMsgHead *pMsgHead, IMsgBody *pMsgBody, uint8_t *pBuf, int32_t nBufSize)
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

	CUpdateCoordReq *pUpdateCoordReq = dynamic_cast<CUpdateCoordReq *>(pMsgBody);
	if(pUpdateCoordReq == NULL)
	{
		return 0;
	}

	if(pMsgHeadCS->m_nSrcUin == 0)
	{
		WRITE_WARN_LOG(SERVER_NAME, "uin is illegal!\n");
		return 0;
	}

	CRedisSessionBank *pRedisSessionBank = (CRedisSessionBank *)g_Frame.GetBank(BANK_REDIS_SESSION);
	RedisSession *pSession = pRedisSessionBank->CreateSession(this, static_cast<RedisReply>(&CUpdateCoordHandler::OnSessionGetUserSimpleInfo),
			static_cast<TimerProc>(&CUpdateCoordHandler::OnRedisSessionTimeout));
	UserSession *pSessionData = new(pSession->GetSessionData()) UserSession();
	pSessionData->m_stCtlHead = *pControlHead;
	pSessionData->m_stMsgHeadCS = *pMsgHeadCS;
	pSessionData->m_stUpdateCoordReq = *pUpdateCoordReq;

	UserBaseInfo *pConfigUserBaseInfo = (UserBaseInfo *)g_Frame.GetConfig(USER_BASEINFO);
	CRedisBank *pRedisBank = (CRedisBank *)g_Frame.GetBank(BANK_REDIS);

	CRedisChannel *pUserBaseInfoChannel = pRedisBank->GetRedisChannel(pConfigUserBaseInfo->string);
	pUserBaseInfoChannel->HMGet(pSession, itoa(pMsgHeadCS->m_nSrcUin), "%s %s %s %s %s", pConfigUserBaseInfo->nickname,
			pConfigUserBaseInfo->headimage, pConfigUserBaseInfo->gender, pConfigUserBaseInfo->age, pConfigUserBaseInfo->oneselfwords);

	return 0;
}

int32_t CUpdateCoordHandler::OnSessionGetUserSimpleInfo(int32_t nResult, void *pReply, void *pSession)
{
	redisReply *pRedisReply = (redisReply *)pReply;
	RedisSession *pRedisSession = (RedisSession *)pSession;
	UserSession *pUserSession = (UserSession *)pRedisSession->GetSessionData();

	CRedisSessionBank *pRedisSessionBank = (CRedisSessionBank *)g_Frame.GetBank(BANK_REDIS_SESSION);
	CStringConfig *pStringConfig = (CStringConfig *)g_Frame.GetConfig(CONFIG_STRING);

	CMsgDispatchConfig *pMsgDispatchConfig = (CMsgDispatchConfig *)g_Frame.GetConfig(CONFIG_MSGDISPATCH);
	CRedisBank *pRedisBank = (CRedisBank *)g_Frame.GetBank(BANK_REDIS);
	pRedisSessionBank->DestroySession(pRedisSession);

	string strNickName;
	string strHeadImage;
	string strOneselfWords;
	int32_t nGender = 0;
	int32_t nAge = 0;
	bool bIsReturn = false;
	do
	{
		if(pRedisReply->type == REDIS_REPLY_ERROR)
		{
			bIsReturn = true;
			break;
		}

		if(pRedisReply->type == REDIS_REPLY_ARRAY)
		{
			redisReply *pReplyElement = pRedisReply->element[0];
			if(pReplyElement->type != REDIS_REPLY_NIL)
			{
				strNickName = string(pReplyElement->str);
			}

			pReplyElement = pRedisReply->element[1];
			if(pReplyElement->type != REDIS_REPLY_NIL)
			{
				strHeadImage = string(pReplyElement->str);
			}

			pReplyElement = pRedisReply->element[2];
			if(pReplyElement->type != REDIS_REPLY_NIL)
			{
				nGender = atoi(pReplyElement->str);
			}

			pReplyElement = pRedisReply->element[3];
			if(pReplyElement->type != REDIS_REPLY_NIL)
			{
				nAge = atoi(pReplyElement->str);
			}

			pReplyElement = pRedisReply->element[4];
			if(pReplyElement->type != REDIS_REPLY_NIL)
			{
				strOneselfWords = string(pReplyElement->str);
			}
		}
	}while(0);

	DB_Location *pDBLocation = (DB_Location *)g_Frame.GetConfig(DB_LOCATION);
	CMongoBank *pMongoBank = (CMongoBank *)g_Frame.GetBank(BANK_MONGO);

	mongoc_client_t *pMongoClient = pMongoBank->GetMongoChannel(pDBLocation->string);
	mongoc_collection_t *pCollection = mongoc_client_get_collection(pMongoClient, pDBLocation->string, pDBLocation->user_coord.string);

	bson_t *query = BCON_NEW(pDBLocation->user_coord.uin, BCON_INT32(pUserSession->m_stMsgHeadCS.m_nSrcUin));
	bson_t *update = BCON_NEW("$set", "{", pDBLocation->user_coord.position, "[", BCON_DOUBLE(pUserSession->m_stUpdateCoordReq.m_nLatitude / 1000000.0),
			BCON_DOUBLE(pUserSession->m_stUpdateCoordReq.m_nLongtitude / 1000000.0), "]", pDBLocation->user_coord.updatetime,
			BCON_INT32(CDateTime::CurrentDateTime().Seconds()), pDBLocation->user_coord.nickname, BCON_UTF8(strNickName.c_str()),
			pDBLocation->user_coord.headimageaddr, BCON_UTF8(strHeadImage.c_str()), pDBLocation->user_coord.age, BCON_INT32(nAge),
			pDBLocation->user_coord.gender, BCON_INT32(nGender), pDBLocation->user_coord.oneselfwords, BCON_UTF8(strOneselfWords.c_str()), "}");

	bson_error_t error;
	if(!mongoc_collection_update(pCollection, MONGOC_UPDATE_UPSERT, query, update, NULL, &error))
	{
		WRITE_WARN_LOG(SERVER_NAME, "user[%u] update coord failed!{error=%s}\n", pUserSession->m_stMsgHeadCS.m_nSrcUin, error.message);
	}

	bson_destroy(query);
	bson_destroy(update);
	mongoc_collection_destroy(pCollection);

	return 0;
}

int32_t CUpdateCoordHandler::OnRedisSessionTimeout(void *pTimerData)
{
	CRedisSessionBank *pRedisSessionBank = (CRedisSessionBank *)g_Frame.GetBank(BANK_REDIS_SESSION);
	RedisSession *pRedisSession = (RedisSession *)pTimerData;
	pRedisSessionBank->DestroySession(pRedisSession);
	return 0;
}


