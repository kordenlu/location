/*
 * getstationuserlist_handler.cpp
 *
 *  Created on: 2015年3月26日
 *      Author: jimm
 */

#include "getstationuserlist_handler.h"
#include "../../common/common_datetime.h"
#include "../../common/common_api.h"
#include "../../frame/frame.h"
#include "../../frame/server_helper.h"
#include "../../frame/redissession_bank.h"
#include "../../logger/logger.h"
#include "../../include/cachekey_define.h"
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

	CGetStationUserListReq *pGetStationUserListReq = dynamic_cast<CGetStationUserListReq *>(pMsgBody);
	if(pGetStationUserListReq == NULL)
	{
		return 0;
	}

	uint64_t nLongtitude;
	uint64_t nLatitude;
	CBusConfig *pBusConfig = (CBusConfig *)g_Frame.GetConfig(CONFIG_BUS);
	pBusConfig->GetStationPosition(pGetStationUserListReq->m_strCityName, pGetStationUserListReq->m_strBusLineID, pGetStationUserListReq->m_strStationName,
			nLongtitude, nLatitude);

	CMongoBank *pMongoBank = (CMongoBank *)g_Frame.GetBank(BANK_MONGO);

	mongoc_client_t *pMongoClient = pMongoBank->GetMongoChannel("location");
	mongoc_collection_t *pCollection = mongoc_client_get_collection(pMongoClient, "location", "user_coord");

	bson_t *query = BCON_NEW("position", "{", "$near", "[", BCON_DOUBLE(double(nLongtitude / 1000000)),
			BCON_DOUBLE(double(nLatitude / 1000000)), "]", "$maxDistance", BCON_DOUBLE(0.1), "}");

	mongoc_cursor_t *cursor = mongoc_collection_find(pCollection, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL);
	const bson_t *doc;
	while(mongoc_cursor_next(cursor, &doc))
	{
		char *str = bson_as_json(doc, NULL);
		bson_free(str);
	}

	return 0;
}


