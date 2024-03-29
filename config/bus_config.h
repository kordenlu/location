/*
 * bus_config.h
 *
 *  Created on: Mar 19, 2015
 *      Author: jimm
 */

#ifndef BUS_CONFIG_H_
#define BUS_CONFIG_H_

#include "frame/frame_impl.h"
#include "frame/frame.h"
#include "include/typedef.h"
#include "server_typedef.h"
#include <map>
#include <vector>
#include <string>
#include <string.h>

using namespace std;
using namespace FRAME;

class Station
{
public:
	Station()
	{
		m_nLongitude = 0;
		m_nLatitude = 0;
		m_nStationIndex = 0;
	}
	string			m_strStationName;
	string			m_strStationID;
	uint64_t		m_nLongitude;
	uint64_t		m_nLatitude;
	uint16_t		m_nStationIndex;
};

class BusLine
{
public:
	string						m_strCityName;
	string						m_strCityID;
	string						m_strBusLineName;
	string						m_strBusLineID;
	string						m_strStartStation;
	string						m_strEndStation;
	map<string, Station *>		m_stBusLineMap;
};

//class CityInfo
//{
//public:
//	string						m_strCityName;
//	string						m_strCityID;
//	map<string, BusLine *>		m_stCityBusInfo;
//};

class Country
{
public:
	string						m_strCountryName;
	string						m_strCountryID;
//	map<string, CityInfo *>		m_stCityInfo;
	map<string, BusLine *>	m_stCityBusInfo;
};

typedef vector<Station *>		StationList;
typedef map<string, StationList>		BusLineInfo;

class CBusConfig : public IConfig
{
public:
	CBusConfig(const char *pConfigName)
	{
		strcpy(m_szConfigFile, pConfigName);
	}

	virtual ~CBusConfig(){};

	//初始化配置
	virtual int32_t Init();
	//卸载配置
	virtual int32_t Uninit();

	virtual int32_t Parser(char *pXMLString);

	void GetStationPosition(string strBusLineID, string strStation, uint64_t &nLongtitude, uint64_t &nLatitude);

	const Station *GetStation(string strBusLineID, string strStation);

	const Station *GetPrevStation(string strBusLineID, string strStation);

	const Station *GetNextStation(string strBusLineID, string strStation);
protected:
	char			m_szConfigFile[enmMaxConfigFileNameSize];
	Country			m_stCountry;
	BusLineInfo		m_stBusLineInfo;
};

#endif /* BUS_CONFIG_H_ */
