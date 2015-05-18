/*
 * getstationuserlist_handler.h
 *
 *  Created on: 2015年3月26日
 *      Author: jimm
 */

#ifndef LOGIC_GETSTATIONUSERLIST_HANDLER_H_
#define LOGIC_GETSTATIONUSERLIST_HANDLER_H_

#include "common/common_object.h"
#include "frame/frame_impl.h"
#include "frame/redis_session.h"
#include "include/control_head.h"
#include "include/location_msg.h"
#include "include/msg_head.h"
#include <string>

using namespace std;
using namespace FRAME;

class CGetStationUserListHandler : public CBaseObject
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

	int32_t GetStationUserList(ICtlHead *pCtlHead, IMsgHead *pMsgHead, IMsgBody *pMsgBody, uint8_t *pBuf, int32_t nBufSize);
};

#endif /* LOGIC_GETSTATIONUSERLIST_HANDLER_H_ */
