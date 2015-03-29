/*
 * main.cpp
 *
 *  Created on: Mar 9, 2015
 *      Author: jimm
 */

#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include "../netevent/net_namespace.h"
#include "../netevent/net_impl.h"
#include "../netevent/net_typedef.h"
#include "../netevent/net_reactor.h"
#include "../netevent/net_handler.h"
#include "../netevent/net_acceptor.h"
#include "../netevent/net_connector.h"
#include "../logger/logger.h"
#include "../frame/frame.h"
#include "../include/cachekey_define.h"
#include "../include/dbkey_define.h"
#include "regist_message.h"
#include "dispatch/msgparser_factory.h"
#include "dispatch/msg_handler.h"
#include "dispatch/checkconn_handler.h"
#include "config/server_config.h"
#include "config/redis_config.h"
#include "server_typedef.h"

using namespace LOGGER;
using namespace NETEVENT;
using namespace FRAME;

//注册到配置管理器
REGIST_CONFIG(USER_BASE_INFO, UserBaseInfo)
REGIST_CONFIG(ACCOUNT_INFO, AccountInfo)
REGIST_CONFIG(DB_LOCATION, DB_Location);

int32_t InitNetAndTimer(CNetHandler *pNetHandler)
{
	CServerConfig *pServerConfig = (CServerConfig *)g_Frame.GetConfig(CONFIG_SERVER);
	if(pServerConfig == NULL)
	{
		return 1;
	}

	//创建包解析器工厂
	IPacketParserFactory *pMsgParserFactory = new CMsgParserFactory();
	CMsgHandler *pMsgHandler = new CMsgHandler();

	CAcceptor *pMsgAcceptor = new CAcceptor(pNetHandler, pMsgParserFactory, pMsgHandler);
	pMsgAcceptor->Bind(pServerConfig->GetOutsideListenPort());

	CCheckConnHandler *pCheckConnHandler = new CCheckConnHandler();
	int32_t nTimerIndex = 0;
	g_Frame.CreateTimer(static_cast<TimerProc>(&CCheckConnHandler::CheckConnStatus),
			pCheckConnHandler, NULL, 10 * MS_PER_SECOND, true, nTimerIndex);

	return 0;
}

#include <bson.h>
#include <mongoc.h>
#include "config/bus_config.h"
#include "bank/mongo_bank.h"
int32_t main()
{
	//启动日志线程
	CLogger::Start();

	//创建网络事件处理器
	CNetHandler *pNetHandler = new CNetHandler();
	pNetHandler->CreateReactor();

	g_Frame.AddRunner(pNetHandler);

	if(g_Frame.Init(SERVER_NAME) != 0)
	{
		return 0;
	}

	InitNetAndTimer(pNetHandler);

	uint64_t nLongtitude;
	uint64_t nLatitude;
	CBusConfig *pBusConfig = (CBusConfig *)g_Frame.GetConfig(CONFIG_BUS);
	pBusConfig->GetStationPosition("北京", "110100014471", "天竺卫生院",
			nLongtitude, nLatitude);

	CMongoBank *pMongoBank = (CMongoBank *)g_Frame.GetBank(BANK_MONGO);

	mongoc_client_t *pMongoClient = pMongoBank->GetMongoChannel("location");
	mongoc_collection_t *pCollection = mongoc_client_get_collection(pMongoClient, "location", "user_coord");

	bson_t *query = BCON_NEW("position", "{", "$near", "[", BCON_DOUBLE(nLongtitude / 1000000),
			BCON_DOUBLE(nLatitude / 1000000), "]", "$maxDistance", BCON_DOUBLE(10000), "}");

	mongoc_cursor_t *cursor = mongoc_collection_find(pCollection, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL);
	const bson_t *doc;
	while(mongoc_cursor_next(cursor, &doc))
	{
		char *str = bson_as_json(doc, NULL);
		bson_free(str);
	}

	while(true)
	{
		g_Frame.Run();
	}

	g_Frame.Uninit();

	return 0;
}

