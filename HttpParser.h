#pragma once

#pragma comment(lib, "Ws2_32.lib")
#include <winsock2.h>

#include <thread>
#include <sys/types.h>
#include <fstream>

#include <mysql.h>
#include <atomic>

//#include <vector>
//#include <mutex>
#include <iostream>
#include <string>
#include <algorithm>
#include <iostream>

//#include <string>
//#include <locale>
#include <codecvt>
#include <Windows.h>


#include "..\pugixml\include\pugixml\pugixml.hpp"

#include "Tasker.h"

//#define DEBUG_PROC

#define MAX_LOG_SIZE_FILE 50000000


#define LISTEN_PORT 5000
#define TIMEOUT 1500
#define MAX_COUNT_SQL 50
#define MAX_CHAR_ID 16
#define RECV_BUFF_SIZE 32*1024
#define LOG_SIZE 4096



class Netline
{
//public:
//	inline void append_pcdata(pugi::xml_node& node, const pugi::char_t* const name, const pugi::char_t* const pcdata)
//	{
//		node.append_child(name).append_child(pugi::xml_node_type::node_pcdata).set_value(pcdata);
//	}
//
//	inline void append_pcdata(pugi::xml_node& node, const pugi::char_t* const name, const pugi::string_t& pcdata)
//	{
//		node.append_child(name).append_child(pugi::xml_node_type::node_pcdata).set_value(pcdata.c_str());
//	}

public:
	Netline();
	~Netline();
	int Start(wchar_t *servicePath);
	int Stop();
	void thrAnswer();
	void thrMain();
	bool fileErrorPresent();
	int saveLog(char * p);
	int addErrorLogMessage(char* p, bool rn);
	std::string utf8_encode(std::wstring wstr);
	bool load_settings();
	__int64 getTimeOffset();
	//bool retFaultAction(int faultCode);
	//int fillData(wchar_t *path, wchar_t *path_fr);
	//int sendData();
	int dbReconnect();
	//int dbQueryGetPath(__int64 id, __int64 max_id);
	//__int64 dbQueryMaxId();
	//__int64 dbQueryMinId();
	//__int64 dbQueryMinIdByDate(std::string *strFile);
	//__int64 dbQueryStartIdByDate(std::string);
	int dbConnect();
	//void connectToServ(char *ip);
	int accept_connection(int port);
	int sendListXml(int cnt);
	int sendDataXml(int cnt);
	void reaccept();
	bool check_connection();
	int proc_message(int *err);
	int getListIdsByTask(Tasker::task *t);

	struct viola
	{

		std::wstring pViolaAzimut;
		std::wstring pViolaPlace;
		std::wstring pViolaDirection;
		std::wstring pViolaSpeed;
		int pViolaSpeedThr;
		std::wstring pViolaGrz;
		std::wstring pViolaGrzAccuracy;
		std::wstring pViolaCodeCountry;
		int pViolaGrzColor;
		std::wstring pViolaLat;//¯ËÓÚ‡
		std::wstring pViolaLon;//‰ÓÎ„ÓÚ‡
		std::wstring pViolaCameraId;
		time_t pViolaTime_ms;
		__int64 pViolaTimeOffset;
		std::string pViolaMs;

		int pViola;

		int nullINt;
		std::wstring nullWstring;
		//xsd__base64Binary grz;
		//xsd__base64Binary photo;
		//xsd__base64Binary photo_extra;
		std::wstring typePhotoExtra;
	};

	struct settings
	{
		char serverPath[MAX_PATH];
		__int64 id;
		std::string camera_id;
		std::string start_date;//2018-01-01 00:00:00.000000
		std::string region;
	};

	struct dbResult
	{
		__int64 id;
		//char path[MAX_PATH];
		wchar_t wpath[MAX_PATH];
	};



private:

	wchar_t path_of_log_error_file[MAX_PATH];
	wchar_t path_root[MAX_PATH];
	char logErrorStr[4096];

	//Õ¿ Œƒ»Õ «¿œ–Œ— œŒÀ”◊¿≈Ã ƒ¬¿ œ”“»
	dbResult dbRes[MAX_COUNT_SQL * 2];

	bool bRunning = false;
	settings set;
	char tempWpath[MAX_PATH];

	MYSQL_RES *g_sqlRes;
	MYSQL	*mysql = nullptr;
	wchar_t *g_pathMain = (wchar_t*)L"D:/arch/all_fixations/";

	__int64 id = 0;
	__int64 id_last = -1;
	__int64 id_last_temp = -2;

	int reconnectCnt = 0;
	SOCKET sct_new;
	char buf_recv[RECV_BUFF_SIZE];
	int send_answer_data(char * query, __int64 *count_rows);
	int send_answer_list(char * query, __int64 *count_rows);
	//char buf_send[RECV_BUFF_SIZE];
	int parsing(int n);
	std::string convertLatinToCyrilyc(std::string latin);
	std::wstring convertCyrilicToLatin(std::wstring latin);

	int timeoffset = -1;
	Tasker *tasker = nullptr;

	struct xmlList
	{
		std::string _time;
		std::string _timestamp;
		std::wstring _liplate;
		std::string _type;
		std::string _path;
		std::string _id;
	};

	struct xmlData
	{
		std::string _time;
		std::string _timestamp;
		std::string _type;
		std::string _path;
		std::wstring _liplate;
		std::string _id;
		std::string _autopatrol;
		short int _Image1Present;
		short int _Image2Present;
		short int _nGrzPrezent;
		short int _XmlPresent;
		short int _DisplaySpeed;
		std::wstring _Violation;
		//short int _OverSpeed;
		//short int _WrongDirection;
		short int _SpeedThreshold;
		short int _Direction;
		std::string _PDD;
		std::string _RadarId;
		short int _Channel;
		std::wstring _SensorName;
		std::string _LocationCode;
		std::wstring _RadarName;
		std::string _CertificateExpDate;
		std::string _CertificateExpLimit;
		int _Latitude;
		int _Longitude;
		std::string _TZOffset;
		int _RectNumX1;
		int _RectNumY1;
		int _RectNumX2;
		int _RectNumY2;
		int _RectNumX3;
		int _RectNumY3;

	};


	xmlList _xmlList[XML_COUNT];
	xmlData _xmlData[XML_COUNT];

	std::wstring utf8_decode(std::string str);
	std::wstring charToWString(const char* text);
	std::wstring utf8_decode_2(const std::string &str);
	void copyCharToWchar(wchar_t *wch,char *ch);
	bool checkChar(char ch);
	std::string timeConvert(std::string t, Tasker::time_addon ta);
	std::string timeConvert2(std::string t);

	struct xml_string_writer : pugi::xml_writer
	{
		std::string result;
		virtual void write(const void* data, size_t size)
		{
			result += std::string(static_cast<const char*>(data), size);
		}
	};

	std::string xmlstring;

	bool waitAccept = false;
	SOCKET sct;

	//std::string xmlResultData;

//	std::wstring utf8_decode(std::string str);
//	std::string utf8_encode(std::wstring wstr);
};

