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

	CMongoBank *pMongoBank = (CMongoBank *)g_Frame.GetBank(BANK_MONGO);

	mongoc_client_t *pMongoClient = pMongoBank->GetMongoChannel("location");
	mongoc_collection_t *pCollection = mongoc_client_get_collection(pMongoClient, "location", "user_coord");

	bson_t *query = BCON_NEW("uin", BCON_INT32(pMsgHeadCS->m_nSrcUin));
	bson_t *update = BCON_NEW("$set", "{", "position", "[", BCON_DOUBLE(pUpdateCoordReq->m_nLongtitude / 1000000),
			BCON_DOUBLE(pUpdateCoordReq->m_nLatitude / 1000000), "]", "updatetime", BCON_INT64(CDateTime::CurrentDateTime().Seconds()),"}");

	bson_error_t error;
	if(!mongoc_collection_update(pCollection, MONGOC_UPDATE_UPSERT, query, update, NULL, &error))
	{
		WRITE_WARN_LOG(SERVER_NAME, "user[%u] update coord failed!{error=%s}\n", pMsgHeadCS->m_nSrcUin, error.message);
	}

	bson_destroy(query);
	bson_destroy(update);
	mongoc_collection_destroy(pCollection);

	return 0;
}




