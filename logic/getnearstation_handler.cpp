/*
 * getnearstation_handler.cpp
 *
 *  Created on: Mar 19, 2015
 *      Author: jimm
 */

#include "getnearstation_handler.h"
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

using namespace LOGGER;
using namespace FRAME;

int32_t CGetNearStationHandler::GetNearStation(ICtlHead *pCtlHead, IMsgHead *pMsgHead, IMsgBody *pMsgBody, uint8_t *pBuf, int32_t nBufSize)
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

	CGetNearStationReq *pGetNearStationReq = dynamic_cast<CGetNearStationReq *>(pMsgBody);
	if(pGetNearStationReq == NULL)
	{
		return 0;
	}

	CBusConfig *pBusConfig = (CBusConfig *)g_Frame.GetConfig(CONFIG_BUS);

	return 0;
}

