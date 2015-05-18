/*
 * regist_message.h
 *
 *  Created on: Mar 10, 2015
 *      Author: jimm
 */

#ifndef REGIST_MESSAGE_H_
#define REGIST_MESSAGE_H_

#include "frame/frame.h"
#include "include/msg_head.h"
#include "include/control_head.h"
#include "include/location_msg.h"
#include "logic/getstationuserlist_handler.h"
#include "logic/updatecoord_handler.h"

using namespace FRAME;

MSGMAP_BEGIN(msgmap)
ON_PROC_PCH_PMH_PMB_PU8_I32(MSGID_GETSTATIONUSERLIST_REQ, ControlHead, MsgHeadCS, CGetStationUserListReq, CGetStationUserListHandler, CGetStationUserListHandler::GetStationUserList);
ON_PROC_PCH_PMH_PMB_PU8_I32(MSGID_UPDATECOORD_REQ, ControlHead, MsgHeadCS, CUpdateCoordReq, CUpdateCoordHandler, CUpdateCoordHandler::UpdateCoord);
MSGMAP_END(msgmap)

#endif /* REGIST_MESSAGE_H_ */
