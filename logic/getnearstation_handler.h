/*
 * getnearstation_handler.h
 *
 *  Created on: Mar 19, 2015
 *      Author: jimm
 */

#ifndef GETNEARSTATION_HANDLER_H_
#define GETNEARSTATION_HANDLER_H_

#include "../../common/common_object.h"
#include "../../frame/frame_impl.h"
#include "../../frame/redis_session.h"
#include "../../include/control_head.h"
#include "../../include/location_msg.h"
#include "../../include/msg_head.h"
#include <string>

using namespace std;
using namespace FRAME;

class CGetNearStationHandler : public CBaseObject
{
	struct UserSession
	{
		UserSession()
		{
		}
		ControlHead			m_stCtlHead;
		MsgHeadCS			m_stMsgHeadCS;
		CGetNearStationReq	m_stGetNearStationReq;
	};

public:

	virtual int32_t Init()
	{
		return 0;
	}
	virtual int32_t Uninit()
	{
		return 0;
	}
	virtual int32_t GetSize()
	{
		return 0;
	}

	int32_t GetNearStation(ICtlHead *pCtlHead, IMsgHead *pMsgHead, IMsgBody *pMsgBody, uint8_t *pBuf, int32_t nBufSize);
};


#endif /* GETNEARSTATION_HANDLER_H_ */
