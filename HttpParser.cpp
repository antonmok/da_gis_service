#pragma once
//#include "pch.h"


#include "..\rapidjson\reader.h"
#include "..\rapidjson\writer.h"
#include "..\rapidjson\document.h"
#include "..\rapidjson\stringbuffer.h" 
#include "..\rapidjson\istreamwrapper.h" 
#include "..\rapidjson\prettywriter.h"
#include "..\rapidjson\ostreamwrapper.h"

#include "HttpParser.h"


Netline::Netline()
{
	//skat_data = { 0 };

	memset(buf_recv, 0, RECV_BUFF_SIZE);
	size_t nCapacity = 1024 * 3.5 * XML_COUNT;
	xmlstring.reserve(nCapacity);
}
Netline::~Netline()
{
}
int Netline::Stop()
{
	bRunning = false;
	Sleep(1000);//задержка при остановке процесса
	return 1;
}
int Netline::Start(wchar_t *servicePath)
{
	char errSql[256];
	bRunning = true;

	memset(path_of_log_error_file, 0, sizeof(wchar_t)*MAX_PATH);
	memset(path_root, 0, sizeof(wchar_t)*MAX_PATH);

	for (int i = wcslen(servicePath) - 1; i > 0; i--)
	{
		if (servicePath[i] == L'\\')
		{
			//id.txt
			printf("\r\n");
			wcsncpy(path_root, servicePath, i);
			wcscat(path_root, L"\\");

			//log_error
			wcscat(path_of_log_error_file, path_root);
			wcscat(path_of_log_error_file, L"log-error-netline.log");

			sprintf(errSql, "path[LOG] = %s\r\n", utf8_encode(path_of_log_error_file).c_str());
			addErrorLogMessage(errSql, false);

			break;
		}
	}
	
	setlocale(LC_ALL, "ru-RU");
	if (load_settings() == false)
		addErrorLogMessage((char*)"load_settings error!",false);
	
	timeoffset = -1;
	while(timeoffset == -1)
	{
		timeoffset = getTimeOffset();
		Sleep(3000);
	}


	tasker = new Tasker();

	//std::thread *thread_1 = new std::thread(&Netline::thrListen, this, this);
	std::thread *thread_2 = new std::thread(&Netline::thrAnswer, this);

	thrMain();

	return 0;
}
int Netline::addErrorLogMessage(char* p, bool rn = false)
{
	//time_t timer;
	//time(&timer);
	//tm * ptm;
	//ptm = gmtime(&timer);
	//printf("[%04d-%02d-%02d %02d:%02d:%02d]\r\n", ptm->tm_year + 1900, ptm->tm_mon, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
	SYSTEMTIME time;
	GetSystemTime(&time);

	if (!rn)
		sprintf(logErrorStr, "[%04d-%02d-%02d %02d:%02d:%02d.%03d] %s\r\n", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond, time.wMilliseconds, p);
	else
		sprintf(logErrorStr, "%s ", p);
	saveLog(logErrorStr);
	return 0;
};
bool Netline::fileErrorPresent()
{
	struct _stat64i32 buffer = {0};

	int res = _wstat(path_of_log_error_file, &buffer);
	if (res == -1)
	{
		if (errno == ENOENT)
		{
			FILE *f = _wfopen(path_of_log_error_file, L"w");
			fclose(f);//_wfopen(path_of_log_error_file[fileErrorPresent]
			res = _wstat(path_of_log_error_file, &buffer);
		}
	}

	if (buffer.st_size > MAX_LOG_SIZE_FILE)
	{
		_wremove(path_of_log_error_file);
		FILE *f = _wfopen(path_of_log_error_file, L"w");
		fclose(f);//_wfopen(path_of_log_error_file [fileErrorPresent]
		res = _wstat(path_of_log_error_file, &buffer);
	}

	if (res == 0)
		return true;
	else
		return false;
}
int Netline::saveLog(char *p)
{
	FILE *f;

	if (fileErrorPresent())
	{
		f = _wfopen(path_of_log_error_file, L"at");
		size_t size = fwrite(p, 1, strlen(p), f);
		fclose(f);//_wfopen(path_of_log_error_file [saveLog]
	}

	return 0;
}
void Netline::thrAnswer()
{
	addErrorLogMessage((char*)"thrAnswer[NETLINE-1.13]");

	time_t start_timer;
	tm * start_ptm;

	//Ждем нормальной даты
	while (true)
	{
		time(&start_timer);
		start_ptm = gmtime(&start_timer);

		//todo
		char errTime[1024];
		sprintf(errTime, "time is = %04d-%02d-%02d %02d:%02d:%02d\r\n", start_ptm->tm_year + 1900, start_ptm->tm_mon + 1, start_ptm->tm_mday, start_ptm->tm_hour, start_ptm->tm_min, start_ptm->tm_sec);
		addErrorLogMessage(errTime);

		if ((start_ptm->tm_year + 1900) >= 2018)break;

		Sleep(15000);
	}

	std::string strFirstFile = "";
	int res = dbConnect();

	//__int64 min_id = dbQueryMinId();
	//__int64 min_date_id = dbQueryMinIdByDate(&strFirstFile);
	////2018-03-20 18:00:00.000000
	//__int64 start_date_id = dbQueryStartIdByDate(set.start_date);
	//printf("min_id = %I64d\r\n", min_id);
	//printf("min_date_id = %I64d\r\n", min_date_id);
	//printf("dbConnect = %d\r\n", res);

	int nerrCntFillData = 0;

	//todo
	//char err[1024];

	bool empty_repeat = false;
	int pine_res = 0;
	int ping_cnt = 0;

	while (bRunning)
	{
		//берем задание и делаем запрос в БД
		Tasker::task *res = tasker->getTask();
		
		if(res != nullptr)
			getListIdsByTask(res);

		if (ping_cnt > 1200)
		{
			pine_res = mysql_ping(mysql);
			if (pine_res != 0)
			{
				const char *pp = mysql_error(mysql);
				unsigned int sqlerrno = mysql_errno(mysql);
				printf("[mysql_ping]: %s[%d]\n", mysql_error(mysql), sqlerrno);
				dbConnect();
			}
			ping_cnt = 0;
		}

		ping_cnt++;
		Sleep(TIMEOUT);//задержка при отправке группы 3000
	}

	//printf("duplo == 0\r\n");

	Sleep(10000);//задержка при закрытии потока

	printf("delete skatTonetline\r\n");
}
std::string Netline::utf8_encode(std::wstring wstr)
{
	if (wstr.length() == 0)
		return "";

	if (wstr.empty()) return std::string();
	int size_needed = (int)wstr.size() + 2;// WideCharToMultiByte(1251, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
	std::string strTo(size_needed, 0);
	int yy = WideCharToMultiByte(1251, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
	return strTo;
}
bool Netline::load_settings()
{
	/*
	std::wstring path = path_id;// L"c:\\soap.json";

	std::ifstream ifs(path);
	rapidjson::IStreamWrapper isw(ifs);
	rapidjson::EncodedInputStream<rapidjson::UTF8<>, rapidjson::IStreamWrapper> eis(isw);
	rapidjson::Document doc;
	doc.ParseStream(eis);

	auto tp = doc.GetType();
	if (!doc.IsObject())
		return false;

	//if (!doc.HasMember("last_id"))
	//{
	//	bool res = doc["last_id"].IsInt();
	//	return false;
	//}

	//if (!doc.HasMember("reg_num"))
	//{
	//	bool res = doc["reg_num"].IsString();
	//	return false;
	//}

	if (!doc.HasMember("start_date"))
	{
		bool res = doc["start_date"].IsString();
		return false;
	}

	if (!doc.HasMember("server"))
	{
		bool res = doc["server"].IsString();
		return false;
	}

	if (!doc.HasMember("region"))
	{
		bool res = doc["region"].IsString();
		return false;
	}

	//	"last_data" : 2018 - 01 - 24,
	//	"last_time" : 18 : 17 : 01
	//rapidjson::StringBuffer buffer;
	//rapidjson::Writer<rapidjson::StringBuffer> swriter(buffer);
	//doc.Accept(swriter);
	//int hh = doc["last_id"].GetInt();
	//std::string hh0 = doc["reg_num"].GetString();

	std::string str = doc["server"].GetString();
	strcpy(set.serverPath, str.c_str());

	set.camera_id = getIdDevice();// doc["reg_num"].GetString();

	set.start_date = doc["start_date"].GetString();

	set.region = doc["region"].GetString();
	*/
	return true;
}
__int64 Netline::getTimeOffset()
{
	bool res;
	std::ifstream ifs("D:\\conf\\uconfig.json");
	rapidjson::IStreamWrapper isw(ifs);
	rapidjson::EncodedInputStream<rapidjson::UTF8<>, rapidjson::IStreamWrapper> eis(isw);
	rapidjson::Document doc;
	doc.ParseStream(eis);

	const auto name_it = doc.FindMember("olympus_uconfig");

	res = (name_it != doc.MemberEnd() && name_it->value.IsObject());
	if (!res) return -1;

	bool kk = doc["olympus_uconfig"].HasMember("complex_utc_offset");

	if (kk)
		return doc["olympus_uconfig"]["complex_utc_offset"].GetUint64();
	else return -1;
}

int Netline::dbConnect()
{
#ifndef DEBUG_PROC
	char* root_password = (char*)"2D31BD17E960BB60DAFA84A2A7B46DDB";//root
#endif

#ifdef DEBUG_PROC
	char* root_password = (char*)"ac87301fb4d6f120b3debe06c8979bc0";//prometheus
#endif

	mysql = mysql_init(NULL); if (mysql == nullptr) return false;
#ifdef DEBUG_PROC
	if (mysql_real_connect(mysql, "7.140.114.54", "prometheus", root_password, nullptr, 3306, NULL, 0) != nullptr)
#endif
#ifndef DEBUG_PROC
	if (mysql_real_connect(mysql, "127.0.0.1", "root", root_password, nullptr, 3306, NULL, 0) != nullptr)
#endif
	{
		printf("dbConnect good\r\n");
		mysql_set_character_set(mysql, "utf8");
		bool rc = true;
		int res_opt = mysql_options(mysql,MYSQL_OPT_RECONNECT,&rc);

		if (res_opt != 0)
		{
			const char *pp = mysql_error(mysql);
			unsigned int sqlerrno = mysql_errno(mysql);
			printf("[mysql_options]: %s[%d]\n", mysql_error(mysql), sqlerrno);
		}
		
		//std::string query = "GRANT ALL PRIVILEGES on *.* to 'root'@'localhost' identified by '" + root_password + "';";
		//int err = mysql_query(mysql, query.c_str());
		//if (err != 0)
		//{
		//	printf("%s\n", mysql_error(mysql));
		//	for (size_t i = 0; i < 5; i++)
		//	{
		//		Beep(500, 150);
		//		Sleep(50);
		//	}
		//	return false;
		//}
	}
	else
	{
		printf("dbConnect BAD!!!\r\n");
		printf("%s\n", mysql_error(mysql));
		return -1;
	}
	//else
	//{
	//	if (mysql_real_connect(mysql, "127.0.0.1", "root", root_password, nullptr, 3306, NULL, 0) == nullptr)
	//	{
	//		return false;
	//	}
	//}
	//mysql_set_character_set(mysql, "utf8");

	return 0;
}
//int Netline::dbQueryGetPath(__int64 id, __int64 max_id)
//{
//	char query[300];
//
//	if ((max_id - id) >= MAX_COUNT_SQL)
//	{
//		//здесь берем по айдишнику, значения
//		sprintf(query, "SELECT `id`,`file_path` FROM `main`.`materials` WHERE `event_id` >= %I64d AND `event_id` < %I64d ORDER BY `id`", id, id + MAX_COUNT_SQL);
//	}
//	else
//	{
//		sprintf(query, "SELECT `id`,`file_path` FROM `main`.`materials` WHERE `event_id` >= %I64d AND `event_id` <= %I64d ORDER BY `id`", id, max_id);
//	}
//
//	try
//	{
//		if (mysql_query(mysql, query))
//		{
//			printf("%s\n", mysql_error(mysql));
//			return -1;
//		}
//
//		if (!(g_sqlRes = mysql_store_result(mysql)))
//		{
//			printf("%s\n", mysql_error(mysql));
//			return -2;
//		}
//	}
//	catch (...)
//	{
//		printf("%s\n", mysql_error(mysql));
//		return -3;
//	}
//
//	if (mysql_num_rows(g_sqlRes) == 0)
//	{
//		printf("%s\n", mysql_error(mysql));
//		return -4;
//	}
//
//	int cnt = 0;
//	char file_path[MAX_PATH];
//
//	//#ifdef DEBUG_PROC
//	//	char errSql2[256];
//	//	sprintf(errSql2, "sqlRes->row_count = %d", g_sqlRes->row_count);
//	//	addErrorLogMessage(errSql2);
//	//#endif
//
//	//todo
//	char errSql3[256];
//	int nError = 0;
//
//	int row_count = g_sqlRes->row_count;
//
//	while ((cnt < row_count) && bRunning)
//	{
//		MYSQL_ROW row = mysql_fetch_row(g_sqlRes);
//
//		if (strcmp(tempWpath, row[1]) != 0)
//			strcpy(tempWpath, row[1]);
//		else
//		{
//			nError++;
//			sprintf(errSql3, "row[1] = %s[nError = %d]", row[1], nError);
//			addErrorLogMessage(errSql3);
//			row_count--;
//			//memset(tempWpath, 0, MAX_PATH);
//			if (nError > 3)break;
//			continue;
//		}
//
//		nError = 0;
//
//		dbRes[cnt].id = atoi(row[0]);
//		wcscpy(dbRes[cnt].wpath, g_pathMain);
//		mbstowcs(dbRes[cnt].wpath + wcslen(g_pathMain), tempWpath, MAX_PATH);
//
//		const char* perno = mysql_error(mysql);
//		int erno = mysql_errno(mysql);
//
//		if (strlen(perno) != 0 || erno != 0)
//		{
//			addErrorLogMessage((char*)perno);
//
//			char errSql[256];
//			sprintf(errSql, "errno[%d]", erno);
//			addErrorLogMessage(errSql, true);
//		}
//
//		cnt++;
//	}
//
//	mysql_free_result(g_sqlRes);
//	g_sqlRes = nullptr;
//
//
//	return cnt;
//}
//__int64 Netline::dbQueryMaxId()
//{
//	char query[300];
//
//	MYSQL_RES *sqlRes;
//	//этим азпросом берем последний айдишник события и соответсвенно кадров
//	strcpy(query, "SELECT `event_id` FROM `main`.`materials` WHERE `event_id` = (SELECT max(event_id) FROM `main`.`materials`)");
//
//	try
//	{
//		if (mysql_query(mysql, query))
//		{
//			printf("%s\n", mysql_error(mysql));
//			return -1;
//		}
//
//		if (!(sqlRes = mysql_store_result(mysql)))
//		{
//			printf("%s\n", mysql_error(mysql));
//			return -2;
//		}
//	}
//	catch (...)
//	{
//		printf("%s\n", mysql_error(mysql));
//		return -3;
//	}
//
//	if (mysql_num_rows(sqlRes) == 0)
//	{
//		printf("%s\n", mysql_error(mysql));
//		return -4;
//	}
//
//	MYSQL_ROW row = mysql_fetch_row(sqlRes);
//	__int64 id = atoi(row[0]);
//
//	mysql_free_result(sqlRes);
//
//	return id;
//}
//__int64 Netline::dbQueryStartIdByDate(std::string strDate)
//{
//	// target_id
//	// SELECT `id` FROM `main`.`targets` WHERE `detection_time` > '2018-01-01 00:00:00.000000' LIMIT 1
//
//	// id from target id
//	// SELECT `id` FROM `main`.`materials` WHERE `target_id` = target_id
//
//	// SELECT `event_id`,`file_path` FROM `main`.`materials` WHERE `target_id` = (SELECT `id` FROM `main`.`targets` WHERE `detection_time` > '2018-03-20 18:00:00.000000' LIMIT 1) AND `file_type` = 1
//	//2018-03-20 18:00:00.000000
//	char query[300];
//
//	//здесь берем по айдишнику, значения
//	sprintf(query, "SELECT `event_id`,`file_path` FROM `main`.`materials` WHERE `target_id` = (SELECT `id` FROM `main`.`targets` WHERE `detection_time` > '%s' LIMIT 1) AND `file_type` = 1", strDate.c_str());
//
//	try
//	{
//		if (mysql_query(mysql, query))
//		{
//			printf("%s\n", mysql_error(mysql));
//			return -1;
//		}
//
//		if (!(g_sqlRes = mysql_store_result(mysql)))
//		{
//			printf("%s\n", mysql_error(mysql));
//			return -2;
//		}
//	}
//	catch (...)
//	{
//		printf("%s\n", mysql_error(mysql));
//		return -3;
//	}
//
//	if (mysql_num_rows(g_sqlRes) == 0)
//	{
//		printf("%s\n", mysql_error(mysql));
//		return -4;
//	}
//
//	int cnt = 0;
//
//	char tmpPath[MAX_PATH] = { 0 };
//	__int64 dateId = 0;
//
//	char errSql3[256];
//	int nError = 0;
//
//	int row_count = g_sqlRes->row_count;
//
//	while ((cnt < row_count) && bRunning)
//	{
//		MYSQL_ROW row = mysql_fetch_row(g_sqlRes);
//
//		nError = 0;
//
//		dateId = atoi(row[0]);
//		strcpy(tmpPath, row[1]);
//
//		//todo
//		char errSql2[256];
//		sprintf(errSql2, "dateId = %I64d[%s]", dateId, tmpPath);
//		addErrorLogMessage(errSql2);
//
//		const char* perno = mysql_error(mysql);
//		int erno = mysql_errno(mysql);
//
//		if (strlen(perno) != 0 || erno != 0)
//		{
//			addErrorLogMessage((char*)perno);
//
//			//todo
//			char errSql[256];
//			sprintf(errSql, "errno[%d]", erno);
//			addErrorLogMessage(errSql, true);
//		}
//
//		cnt++;
//	}
//
//	mysql_free_result(g_sqlRes);
//	g_sqlRes = nullptr;
//
//	return dateId;
//}
//__int64 Netline::dbQueryMinIdByDate(std::string *strFirstFile)
//{
//	char path[1024] = { 0 };
//
//	int year = 0;
//	int month = 0;
//	int day = 0;
//	bool exit = false;
//
//	for (year = 2018; year < 2100 && !exit; year++)
//	{
//		for (month = 1; month <= 12 && !exit; month++)
//		{
//			for (day = 1; day <= 31 && !exit; day++)
//			{
//				sprintf(path, "dir d:\\arch\\all_fixations\\%04d\\%02d\\%02d", year, month, day);
//				if (system(path) == 0)
//				{
//					exit = true;
//					break;
//				}
//			}
//			if (exit)break;
//		}
//		if (exit)break;
//	}
//
//	wchar_t dateStr[1024] = { 0 };
//	_swprintf(dateStr, L"D:\\arch\\all_fixations\\%04d\\%02d\\%02d\\*", year, month, day);
//
//	WIN32_FIND_DATA findFileData;
//	std::wstring strPath;
//	strPath.append(dateStr);
//
//	HANDLE hFind = FindFirstFile(strPath.c_str(), &findFileData);
//	std::wstring wstrFile;
//
//	do
//	{
//		wstrFile = findFileData.cFileName;
//
//		if (wstrFile.find(L"jpg") > 0 && wstrFile.find(L"_fr.jpg") == -1 && wstrFile.find(L"sign") == -1 && wstrFile.find(L"..") == -1 && wstrFile.size() > 30)
//			break;
//
//	} while (FindNextFile(hFind, &findFileData) != 0);
//
//	*strFirstFile = utf8_encode(wstrFile).c_str();
//
//	FindClose(hFind);
//
//	char query[300];
//
//	MYSQL_RES *sqlRes;
//	// этим азпросом берем последний айдишник события и соответсвенно кадров
//	// SELECT `event_id` FROM `main`.`materials` WHERE `timestamp` > '2018-01-01 00:00:00.000000' LIMIT 1
//	// sprintf(query, "SELECT `id`,`file_path` FROM `main`.`materials` WHERE `event_id` >= %I64d AND `event_id` < %I64d ORDER BY `id`", id, id + MAX_COUNT_SQL);
//	// SELECT `event_id`, `id`,`file_path`,`timestamp`	FROM `main`.`materials`	WHERE `file_path` LIKE '2018/03/13%' AND `file_type` = 1 ORDER BY `id` LIMIT 1
//	//SELECT `event_id`,`file_path` FROM `main`.`materials` WHERE `file_path` LIKE '2018/03/14/1506018_2018_03_14_09_57_07_53_00_00.jpg'
//
//	sprintf(query, "SELECT `event_id` FROM `main`.`materials` WHERE `file_path` LIKE '%04d/%02d/%02d/%s'", year, month, day, (*strFirstFile).c_str());
//
//
//	try
//	{
//		if (mysql_query(mysql, query))
//		{
//			printf("%s\n", mysql_error(mysql));
//			return -1;
//		}
//
//		if (!(sqlRes = mysql_store_result(mysql)))
//		{
//			printf("%s\n", mysql_error(mysql));
//			return -2;
//		}
//	}
//	catch (...)
//	{
//		printf("%s\n", mysql_error(mysql));
//		return -3;
//	}
//
//	if (mysql_num_rows(sqlRes) == 0)
//	{
//		printf("%s\n", mysql_error(mysql));
//		return -4;
//	}
//
//	MYSQL_ROW row = mysql_fetch_row(sqlRes);
//	__int64 id = atoi(row[0]);
//
//	mysql_free_result(sqlRes);
//
//	return id;
//}
//__int64 Netline::dbQueryMinId()
//{
//	char query[300];
//
//	MYSQL_RES *sqlRes;
//	//этим азпросом берем последний айдишник события и соответсвенно кадров
//	strcpy(query, "SELECT `event_id` FROM `main`.`materials` WHERE `event_id` = (SELECT min(event_id) FROM `main`.`materials`)");
//
//	try
//	{
//		if (mysql_query(mysql, query))
//		{
//			printf("%s\n", mysql_error(mysql));
//			return -1;
//		}
//
//		if (!(sqlRes = mysql_store_result(mysql)))
//		{
//			printf("%s\n", mysql_error(mysql));
//			return -2;
//		}
//	}
//	catch (...)
//	{
//		printf("%s\n", mysql_error(mysql));
//		return -3;
//	}
//
//	if (mysql_num_rows(sqlRes) == 0)
//	{
//		printf("%s\n", mysql_error(mysql));
//		return -4;
//	}
//
//	MYSQL_ROW row = mysql_fetch_row(sqlRes);
//	__int64 id = atoi(row[0]);
//
//	mysql_free_result(sqlRes);
//
//	return id;
//}
int Netline::dbReconnect()
{
	char* root_password = (char*)"2D31BD17E960BB60DAFA84A2A7B46DDB";

	if (mysql_real_connect(mysql, "127.0.0.1", "root", root_password, nullptr, 3306, NULL, 0) == nullptr)
	{
		return -1;
	}
	mysql_set_character_set(mysql, "utf8");
	return 0;
}
//void Netline::connectToServ(char *ip)
//{
//
//}
void Netline::thrMain()
{
	addErrorLogMessage((char*)"thrListen[NETLINE-1.13]");
	setlocale(LC_ALL, "ru-RU");

	WSADATA     WsaData = { 0 };
	int ret = WSAStartup(0x101, &WsaData);
	if (ret != 0) return;

	bool work = true;
	sct_new = 0;
	accept_connection(LISTEN_PORT);
	waitAccept = false;

	while (work)
	{
		//if (!check_connection())

		// получаем сообщение от клиента
		// и передаем на обработку
		int err = 0;
		int res = proc_message(&err);

		if (res > 0)
		{
			while (!waitAccept)
			{
				Sleep(1000);
			}
		}

		if (waitAccept)
		{ 
			printf("waitAccept\r\n");
			waitAccept = false;
			reaccept();
		}

		Sleep(1000);
	}

	WSACleanup();
//	return 0;
}
int Netline::proc_message(int *err)
{
	memset(buf_recv,0, RECV_BUFF_SIZE);
	int bytes_num = recv(sct_new, buf_recv, RECV_BUFF_SIZE, 0);

	char *pp = strstr(buf_recv, "getevent");
	int  diff_f = 0;
	if(pp != nullptr) diff_f = strlen(buf_recv) - strlen(pp);

	if (pp == nullptr) { waitAccept = true; return -1; }
	//если иконка
	if(diff_f > 10) { waitAccept = true; return -1; }

	printf("bytes_num = %d[%d]\r\n", bytes_num,diff_f);

	if (bytes_num == SOCKET_ERROR)
	{
		*err = WSAGetLastError();
		char errSqlMin[LOG_SIZE];
		sprintf(errSqlMin, "[recv-err][%s][%d]", err, bytes_num);
		addErrorLogMessage(errSqlMin);
		return -1;
	}

	if (bytes_num == 0)
		return -2;

	if (parsing(bytes_num) != 0)
		return -2;

	return bytes_num;
}
int Netline::getListIdsByTask(Tasker::task * t)
{
	int andCounter = 0;
	char query[LOG_SIZE] = { 0 };
	// Нужно получить список тех айдишников,
	// по которым мы потом будем формировать xml
	// 
	if (t->rtype == Tasker::request_type::LIST)
	{
		sprintf(query,
			"SELECT `region_ru_samara`.`xml`.`timestamp`,`region_ru_samara`.`xml`.`timestampdb`,`region_ru_samara`.`xml`.`num`,`region_ru_samara`.`xml`.`type`,`region_ru_samara`.`xml`.`path`,`region_ru_samara`.`xml`.`id` FROM `region_ru_samara`.`xml` ");
	}

	if (t->rtype == Tasker::request_type::DATA)
	{
		sprintf(query,
			"SELECT `region_ru_samara`.`xml`.`timestamp`,\
`region_ru_samara`.`xml`.`timestampdb`,\
`region_ru_samara`.`xml`.`type`,\
`region_ru_samara`.`xml`.`path`,\
`region_ru_samara`.`xml`.`num`,\
`region_ru_samara`.`xml`.`id`,\
`region_ru_samara`.`xml`.`tAutoPatrolVersion`,\
`region_ru_samara`.`xml`.`nImage1Present`,\
`region_ru_samara`.`xml`.`nGRZPresent`,\
`region_ru_samara`.`xml`.`nXmlPresent`,\
`region_ru_samara`.`xml`.`nDisplaySpeed`,\
`region_ru_samara`.`xml`.`nSpeedThreshold`,\
`region_ru_samara`.`xml`.`nDirection`,\
`region_ru_samara`.`xml`.`nPDD`,\
`region_ru_samara`.`xml`.`tRadarId`,\
`region_ru_samara`.`xml`.`tChannel`,\
`region_ru_samara`.`xml`.`tSensorName`,\
`region_ru_samara`.`xml`.`tLocationCode`,\
`region_ru_samara`.`xml`.`tRadarName`,\
`region_ru_samara`.`xml`.`tCertificateExpDate`,\
`region_ru_samara`.`xml`.`tCertificateExpLimit`,\
`region_ru_samara`.`xml`.`tLatitude`,\
`region_ru_samara`.`xml`.`tLongitude`,\
`region_ru_samara`.`xml`.`nTZOffset`,\
`region_ru_samara`.`xml`.`nRectNumX1`,\
`region_ru_samara`.`xml`.`nRectNumY1`,\
`region_ru_samara`.`xml`.`nRectNumX2`,\
`region_ru_samara`.`xml`.`nRectNumY2`,\
`region_ru_samara`.`xml`.`nRectNumX3`,\
`region_ru_samara`.`xml`.`nRectNumY3`,\
`region_ru_samara`.`xml`.`tViolation`,`region_ru_samara`.`xml`.`nImage2Present` FROM `region_ru_samara`.`xml` ");
	}
		//sprintf(query,
		//	"SELECT `main`.`events`.`time`, `main`.`events`.`timestamp`, `main`.`events`.`liplate_data`, `main`.`events`.`type`,`main`.`materials`.`file_path` \
		//	FROM `main`.`events` JOIN `main`.`materials` ON `main`.`events`.`id`=`main`.`materials`.`event_id` AND `file_type` = 1 \
		//	AND `main`.`events`.`timestamp` > '2018-08-21 00:00:00.000000' \
		//	AND `main`.`events`.`liplate_data` LIKE '%' \
		//	AND `main`.`events`.`type` LIKE '3' \
		//	ORDER BY `main`.`events`.`timestamp` LIMIT 10");
		// SELECT `main`.`events`.`time`, `main`.`events`.`timestamp`, `main`.`events`.`liplate_data`, `main`.`events`.`type`,`main`.`materials`.`file_path` FROM `main`.`events` JOIN `main`.`materials` ON `main`.`events`.`id`=`main`.`materials`.`event_id` AND `file_type` = 1 AND `main`.`events`.`timestamp` > '2018-08-21 00:00:00.000000' AND `main`.`events`.`liplate_data` LIKE 'е285ет|178' ORDER BY `main`.`events`.`timestamp` LIMIT 10
		// SELECT `main`.`events`.`time`, `main`.`events`.`timestamp`, `main`.`events`.`liplate_data`, `main`.`events`.`type`,`main`.`materials`.`file_path` \
		// FROM `main`.`events` JOIN `main`.`materials` ON `main`.`events`.`id`=`main`.`materials`.`event_id` 
		// AND `file_type` = 1 
		// AND `main`.`events`.`timestamp` > '2018-08-21 00:00:00.000000' 
		// AND `main`.`events`.`liplate_data` LIKE '%' 
		// AND `main`.`events`.`type` LIKE '0' 
		// ORDER BY `main`.`events`.`timestamp` 
		// LIMIT 10

		if (t->start_db_time != "")
		{
			if (andCounter == 0)strcat(query, "WHERE ");
			if (andCounter > 0)strcat(query, "AND ");
			andCounter++;
			strcat(query,"`region_ru_samara`.`xml`.`timestampdb` > ");
			strcat(query, "'");
			//t->start_db_time = timeConvert(t->start_db_time,Tasker::time_addon::MINUS);
			strcat(query, t->start_db_time.c_str());
			strcat(query, "'");
			strcat(query, " ");
		}

		if (t->end_db_time != "")
		{
			if (andCounter == 0)strcat(query, "WHERE ");
			if(andCounter > 0)strcat(query, "AND ");
			andCounter++;
			strcat(query, "`region_ru_samara`.`xml`.`timestampdb` < ");
			strcat(query, "'");
			//t->end_db_time = timeConvert(t->end_db_time, Tasker::time_addon::MINUS);
			strcat(query, t->end_db_time.c_str());
			strcat(query, "'");
			strcat(query, " ");
		}

		if (t->start_time != "")
		{
			if (andCounter == 0)strcat(query, "WHERE ");
			if (andCounter > 0)strcat(query, "AND ");
			andCounter++;
			strcat(query, "`region_ru_samara`.`xml`.`timestamp` > ");
			strcat(query, "'");
			strcat(query, t->start_time.c_str());
			strcat(query, "'");
			strcat(query, " ");
		}

		if (t->end_time != "")
		{
			if (andCounter == 0)strcat(query, "WHERE ");
			if (andCounter > 0)strcat(query, "AND ");
			andCounter++;
			strcat(query, "`region_ru_samara`.`xml`.`timestamp` < ");
			strcat(query, "'");
			strcat(query, t->end_time.c_str());
			strcat(query, "'");
			strcat(query, " ");
		}

		if (t->grz != "")
		{
			if (andCounter == 0)strcat(query, "WHERE ");
			if (andCounter > 0)strcat(query, "AND ");
			andCounter++;
			//todo
			//латиницу переделать в кирилицу
			t->grz = convertLatinToCyrilyc(t->grz);
			std::replace(t->grz.begin(), t->grz.end(), '*', '%');
			strcat(query, "`region_ru_samara`.`xml`.`num` LIKE ");
			strcat(query, "'");
			strcat(query, t->grz.c_str());
			strcat(query, "'");
			strcat(query, " ");
		}

		if (t->type[0] != Tasker::viola_type::SPACE)
		{
			if (andCounter == 0)strcat(query, "WHERE ");
			if (andCounter > 0)strcat(query, "AND ");
			andCounter++;
			strcat(query, "`region_ru_samara`.`xml`.`type` IN (");

			for (size_t i = 0; i < TYPES_CNT; i++)
			{
				if (t->type[i] == Tasker::viola_type::SPACE)break;
				if ((t->type[i] >= Tasker::viola_type::C0) && (t->type[i] <= Tasker::viola_type::C16))
				{
					strcat(query, "'");
					if (t->type[i] != Tasker::viola_type::C1)
					{
						char _tp[5] = { 0 };
						_itoa(t->type[i], _tp, 10);
						strcat(query, "C");
						strcat(query, _tp);
					}
					if (t->type[i] == Tasker::viola_type::C1)
						strcat(query, "C1");
					strcat(query, "'");
					strcat(query, ",");
					
				}
			}
			query[strlen(query) - 1] = 0;
			strcat(query, ") ");
		}

		if (andCounter == 0)
		{
			char nn[10] = { 0 };
			strcat(query, "WHERE `region_ru_samara`.`xml`.`event_id` > (SELECT MAX(`region_ru_samara`.`xml`.`event_id`) - ");
			if (t->count > 0)
			{
				sprintf(nn,"%d", t->count);
				strcat(query, nn);
			}
			else strcat(query, "2");

			strcat(query, " FROM `region_ru_samara`.`xml`) ");
		}

		strcat(query, "ORDER BY `region_ru_samara`.`xml`.`timestampdb` ASC "); 

		if (t->count != 0)
		{
			char cnt[10] = { 0 };
			_itoa(t->count, cnt, 10);
			strcat(query, "LIMIT ");
			strcat(query, cnt);
		}
		else
		{
			strcat(query, "LIMIT 2000");
		}

		// отправка xml
		char errSqlMin[LOG_SIZE];
		sprintf(errSqlMin, "[SQL][%s]", query);
		addErrorLogMessage(errSqlMin);

	__int64 cnt = 0;

	if (t->rtype == Tasker::request_type::LIST)
	{
		int res = send_answer_list(query, &cnt);
		if (res < 0)waitAccept = true;
		sprintf(errSqlMin, "[LIST:SQL-RES][size is=%d][rows = %I64d]", res, cnt);
		addErrorLogMessage(errSqlMin);
	}

	if (t->rtype == Tasker::request_type::DATA)
	{
		int res = send_answer_data(query, &cnt);
		if (res < 0)
		{
			waitAccept = true;
		}
		sprintf(errSqlMin, "[DATA:SQL-RES][size is=%d][rows = %I64d]", res, cnt);
		addErrorLogMessage(errSqlMin);
	}

	

	return 0;
}
//тО ЧТО ПРИХОДИТ ОТ КЛИЕНТА В ЗАПРОСЕ ДЛЯ ПЕРЕДАЧИ В БД
std::string Netline::convertLatinToCyrilyc(std::string latin)
{
	char lat[12] = {'А','В','С','Е','Н','К','М','О','Р','Т','Х','У'};
	//char cyr[12] = { 'а','в','с','е','н','к','м','о','р','т','х','у' };
	char cyr[12] = { 'А','В','С','Е','Н','К','М','О','Р','Т','Х','У' };

	for (std::string::iterator it = latin.begin(); it != latin.end(); ++it)
	{
		for (int i = 0; i < 12; i++)
			if (*it == lat[i]) 
				*it = cyr[i];
	}

	return latin;
}
//ПЕРЕДАЕТСЯ ТО ЧТО ВОЗВРАЩЕНО ИЗ БД И РЕЗУЛЬТАТ БУДЕТ ВПИСАН В XML
std::wstring Netline::convertCyrilicToLatin(std::wstring latin)
{
	wchar_t lat[12] = { L'А',L'В',L'С',L'Е',L'Н',L'К',L'М',L'О',L'Р',L'Т',L'Х',L'У' };
	wchar_t cyr[12] = { L'а',L'в',L'с',L'е',L'н',L'к',L'м',L'о',L'р',L'т',L'х',L'у' };

	for (std::wstring::iterator it = latin.begin(); it != latin.end(); ++it)
	{
		for (int i = 0; i < 12; i++)
			if (*it == cyr[i]) 
				*it = lat[i];
	}

	return latin;
}
int Netline::parsing(int n)
{
#ifdef DEBUG_PROC
	//strcpy(buf_recv, "/geteventdata?tdb1=2018-05-24T15:03:21+04:00&type=VIOL/geteventdata?t2=2018-05-24T15:03:21+03&type=C0&num=C0*4C*177&Cnt=50/geteventdata?t1=2018-05-23T18:31:42+03&type=C0,C11&num=C0*4C*177/geteventdata?t1=2018-05-23T18:31:42+03&t2=2018-05-24T15:03:21-03&Cnt=1000/geteventdata?t1=2018-05-23T18:31:42+03:00&Cnt=1000/geteventdata?t1=2018-05-23T18:31:42+03&t2=2018-05-24T15:03:21-03&type=C0,C2,C3&num=C0*4C*177");
	//strcpy(buf_recv, "/geteventlist?tdb1=2018-05-24T15:03:21+04:00&type=VIOL/geteventlist?t2=2018-05-24T15:03:21+03&type=C0&num=C0*4C*177&Cnt=50/geteventlist?t1=2018-05-23T18:31:42+03&type=C0,C11&num=C0*4C*177/geteventlist?t1=2018-05-23T18:31:42+03&t2=2018-05-24T15:03:21-03&Cnt=1000/geteventlist?t1=2018-05-23T18:31:42+03:00&Cnt=1000/geteventlist?t1=2018-05-23T18:31:42+03&t2=2018-05-24T15:03:21-03&type=C0,C2,C3&num=C0*4C*177");
	//strcpy(buf_recv, "/geteventlist?tdb1=2018-09-10T15:00:00+04&tdb2=2018-09-10T15:10:00+04&cnt=2000");
#endif
	n = strlen(buf_recv);

	char errSqlMin[LOG_SIZE];
	sprintf(errSqlMin, "[GET][%s][%d]", buf_recv,n);
	addErrorLogMessage(errSqlMin);

	char * pch;
	int ind1 = 0; 
	int ind2 = 0;
	int cnt_cur = 0;
	pch = strchr(buf_recv, ' ');
	while (pch != NULL)
	{
		if (cnt_cur == 0) ind1 = pch - buf_recv;
		if (cnt_cur == 1) { ind2 = pch - buf_recv; break; }
		pch = strchr(pch + 1, ' ');
		cnt_cur++;
	}

	if ((ind2 - ind1) < 14) return -1;

	memcpy(buf_recv, buf_recv + ind1 + 1, ind2 - ind1 - 1);
	buf_recv[ind2 - ind1 - 1] = '\0';
	//memset(buf_recv + (ind2-ind1), 0, RECV_BUFF_SIZE - ind2);
	sprintf(errSqlMin, "[CUTTING][%s][ind1=%d][ind2=%d]", buf_recv,ind1,ind2);
	addErrorLogMessage(errSqlMin);

	char *next_token1 = nullptr;
	//разбиваем строку на куски с полями
	char del[7] = {'?','&','=','/',0xd,0xa,0};
	char *_pch = nullptr;

	_pch = strtok_s(buf_recv, del, &next_token1);

	sprintf(errSqlMin, "[strtok][0][%s]", _pch);
	addErrorLogMessage(errSqlMin);
	
	if(_pch != nullptr)
	if (strstr(_pch, ":") != nullptr)
		if (strstr(_pch, "T") == nullptr)
		{
			tasker->clear_task();
			return -2;
		}

	tasker->createTask(_pch,true);

	int cnt = 0;
	while (_pch != nullptr)
	{
		_pch = strtok_s(nullptr, del, &next_token1);

		if (_pch != nullptr)
		if (strstr(_pch,":") != nullptr)
			if (strstr(_pch, "T") == nullptr)
			{
				tasker->clear_task();
				return -2;
			}

		tasker->createTask(_pch,false);

		sprintf(errSqlMin, "[strtok][%d][%s]",cnt, _pch);
		addErrorLogMessage(errSqlMin);
		cnt++;
	}

	return 0;
}
int Netline::send_answer_list(char *query,__int64 *count_rows)
{
	for (int ii = 0; ii < XML_COUNT; ii++)
	{
		_xmlList[ii]._path = "";
		_xmlList[ii]._type = "";
		_xmlList[ii]._id = "";
		_xmlList[ii]._time = "";
		_xmlList[ii]._timestamp = "";
		_xmlList[ii]._liplate = L"";
	}

	try
	{
		if (mysql_query(mysql, query))
		{
			const char *pp = mysql_error(mysql);
			unsigned int sqlerrno = mysql_errno(mysql);
			printf("%s[%d]\n", mysql_error(mysql), sqlerrno);
			return -1;
		}

		g_sqlRes = mysql_store_result(mysql);
		if (!g_sqlRes)
		{
			printf("%s\n", mysql_error(mysql));
			return -2;
		}
	}
	catch (...)
	{
		printf("%s\n", mysql_error(mysql));
		return -3;
	}

	*count_rows = (unsigned __int64)mysql_num_rows(g_sqlRes);
	if (count_rows == 0)
	{
		printf("[SQL-ERR] RES == 0\r\n");
		return -4;
	}

	int cnt = 0;
	while ((cnt < g_sqlRes->row_count) && bRunning)
	{
		MYSQL_ROW row = mysql_fetch_row(g_sqlRes);

		//_xmlList[cnt]._time = std::string(row[0]);//time
		_xmlList[cnt]._time = timeConvert2(std::string(row[0]));

		//_xmlList[cnt]._timestamp = std::string(row[1]);//timestamp
		_xmlList[cnt]._timestamp = timeConvert2(std::string(row[1]));// , Tasker::time_addon::PLUS);

		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
		_xmlList[cnt]._liplate = converter.from_bytes(std::string(row[2]));// utf8_decode_2(std::string(row[2]));//string-liplate
		//_xmlList[cnt]._liplate = convertCyrilicToLatin(_xmlList[cnt]._liplate);
		//std::size_t found = _xmlList[cnt]._liplate.find(L"|");
		//if (found != std::string::npos)
		//	_xmlList[cnt]._liplate.erase(found,1);

		//if (strcmp(row[3], "1.1") == 0)
		//	_xmlList[cnt]._type = 1;
		//else
		//	_xmlList[cnt]._type = atoi(row[3]);//int-type
		_xmlList[cnt]._type = std::string(row[3]);

		//strcat(_xmlList[cnt]._path,"C");
		//char _tp[5] = { 0 };
		//_itoa(_xmlList[cnt]._type, _tp, 10);
		//strcat(_xmlList[cnt]._path, _tp);
		//strcat(_xmlList[cnt]._path, "/");
		//strncpy(_xmlList[cnt]._path+strlen(_xmlList[cnt]._path), row[4],10);//string-path
		_xmlList[cnt]._path = std::string(row[4]);

		//strncpy(_xmlList[cnt]._id, row[4] + 11,strlen(row[4])-15);//string-path
		_xmlList[cnt]._id = std::string(row[5]);
		cnt++;
	}

	mysql_free_result(g_sqlRes);
	g_sqlRes = nullptr;

	return sendListXml(cnt);
}
int Netline::send_answer_data(char *query, __int64 *count_rows)
{
	for (int ii = 0; ii < XML_COUNT; ii++)
	{
		_xmlData[ii]._time = "";
		_xmlData[ii]._timestamp = "";
		_xmlData[ii]._type = "";
		_xmlData[ii]._path = "";
		_xmlData[ii]._liplate = L"";
		_xmlData[ii]._id = "";
		_xmlData[ii]._autopatrol = "";
		_xmlData[ii]._Image1Present = 0;
		_xmlData[ii]._Image2Present = 0;
		_xmlData[ii]._nGrzPrezent = 0;
		_xmlData[ii]._XmlPresent = 0;
		_xmlData[ii]._DisplaySpeed = 0;
		_xmlData[ii]._Violation = L"";
		//_xmlData[ii]._OverSpeed = 0;
		//_xmlData[ii]._WrongDirection = 0;
		_xmlData[ii]._Direction = 0;
		_xmlData[ii]._SpeedThreshold = 0;
		_xmlData[ii]._PDD = "";
		_xmlData[ii]._RadarId = "";
		_xmlData[ii]._Channel = 0;
		_xmlData[ii]._SensorName = L"";
		_xmlData[ii]._LocationCode = "";
		_xmlData[ii]._RadarName = L"";
		_xmlData[ii]._CertificateExpDate = "";
		_xmlData[ii]._CertificateExpLimit = "";
		_xmlData[ii]._Latitude = 0;
		_xmlData[ii]._Longitude = 0;
		_xmlData[ii]._TZOffset = "";
		_xmlData[ii]._RectNumX1 = 0;
		_xmlData[ii]._RectNumY1 = 0;
		_xmlData[ii]._RectNumX2 = 0;
		_xmlData[ii]._RectNumY2 = 0;
		_xmlData[ii]._RectNumX3 = 0;
		_xmlData[ii]._RectNumY3 = 0;
	}

	try
	{
		if (mysql_query(mysql, query))
		{
			const char *pp = mysql_error(mysql);
			unsigned int sqlerrno = mysql_errno(mysql);
			printf("%s[%d]\n", mysql_error(mysql), sqlerrno);
			return -1;
		}

		g_sqlRes = mysql_store_result(mysql);
		if (!g_sqlRes)
		{
			printf("%s\n", mysql_error(mysql));
			return -2;
		}
	}
	catch (...)
	{
		printf("%s\n", mysql_error(mysql));
		return -3;
	}

	*count_rows = (unsigned __int64)mysql_num_rows(g_sqlRes);
	if (count_rows == 0)
	{
		printf("[SQL-ERR] RES == 0\r\n");
		return -4;
	}

	int cnt = 0;
	while ((cnt < g_sqlRes->row_count) && bRunning)
	{
		MYSQL_ROW row = mysql_fetch_row(g_sqlRes);

		_xmlData[cnt]._time = timeConvert2(std::string(row[0]));
		_xmlData[cnt]._timestamp = timeConvert2(std::string(row[1]));
		_xmlData[cnt]._type = row[2];
		_xmlData[cnt]._path = row[3];
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
		_xmlData[cnt]._liplate = converter.from_bytes(std::string(row[4]));
		_xmlData[cnt]._id = row[5];
		_xmlData[cnt]._autopatrol = row[6];
		_xmlData[cnt]._Image1Present = atoi(row[7]);
		_xmlData[cnt]._nGrzPrezent = atoi(row[8]);
		_xmlData[cnt]._XmlPresent = atoi(row[9]);
		_xmlData[cnt]._DisplaySpeed = atoi(row[10]);
		//_xmlData[cnt]._OverSpeed = atoi(row[11]);
		//_xmlData[cnt]._WrongDirection = atoi(row[12]);
		_xmlData[cnt]._SpeedThreshold = atoi(row[11]);
		_xmlData[cnt]._Direction = atoi(row[12]);
		_xmlData[cnt]._PDD = row[13];
		_xmlData[cnt]._RadarId = row[14];
		_xmlData[cnt]._Channel = atoi(row[15]);
		_xmlData[cnt]._SensorName = converter.from_bytes(std::string(row[16]));
		_xmlData[cnt]._LocationCode = row[17];
		_xmlData[cnt]._RadarName = converter.from_bytes(std::string(row[18]));
		_xmlData[cnt]._CertificateExpDate = row[19];
		_xmlData[cnt]._CertificateExpLimit = row[20];
		_xmlData[cnt]._Latitude = atoi(row[21]);
		_xmlData[cnt]._Longitude = atoi(row[22]);
		_xmlData[cnt]._TZOffset = row[23];
		_xmlData[cnt]._RectNumX1 = atoi(row[24]);
		_xmlData[cnt]._RectNumY1 = atoi(row[25]);
		_xmlData[cnt]._RectNumX2 = atoi(row[26]);
		_xmlData[cnt]._RectNumY2 = atoi(row[27]);
		_xmlData[cnt]._RectNumX3 = atoi(row[28]);
		_xmlData[cnt]._RectNumY3 = atoi(row[29]);
		_xmlData[cnt]._Violation = converter.from_bytes(std::string(row[30]));
		_xmlData[cnt]._Image2Present = atoi(row[31]);

		cnt++;
	}

	mysql_free_result(g_sqlRes);
	g_sqlRes = nullptr;

	return sendDataXml(cnt);
}
bool Netline::check_connection()
{
	char error = 0;
	int len = sizeof(error);
	int retval = getsockopt(sct_new, SOL_SOCKET, SO_ERROR, &error, &len);

	if (retval != 0) {
		/* there was a problem getting the error code */
		fprintf(stderr, "error getting socket error code: %s\n", strerror(retval));
		return false;
	}

	if (error != 0) {
		/* socket has a non zero error status */
		fprintf(stderr, "socket error: %s\n", strerror(error));
		return false;
	}

	return true;
}
void Netline::reaccept()
{
	closesocket(sct_new);
	closesocket(sct);
	//WSACleanup();
	accept_connection(LISTEN_PORT);
}
int Netline::accept_connection(int port)
{
	printf("wait accept...\r\n");
	// инициализация сокета
	sockaddr_in sct_bind;
	//WSADATA     WsaData = {0};
	//int ret = WSAStartup(0x101, &WsaData);
	//if (ret != 0) return 0;

	sct_bind.sin_family = AF_INET;
	sct_bind.sin_addr.s_addr = INADDR_ANY;//inet_addr("127.0.0.1");
	sct_bind.sin_port = htons(port);

	sct = socket(AF_INET, SOCK_STREAM, 0);

	int bOptLen = sizeof(BOOL);
	BOOL bOptVal = TRUE;

	int iResult = setsockopt(sct, SOL_SOCKET, SO_KEEPALIVE, (char *)&bOptVal, bOptLen);
	if (iResult == SOCKET_ERROR) {
		wprintf(L"setsockopt for SO_KEEPALIVE failed with error: %u\n", WSAGetLastError());
	}
	else wprintf(L"Set SO_KEEPALIVE: ON\n");

	// bind our master socket to the RTSP port and listen for a client connection
	if (bind(sct, (SOCKADDR*)&sct_bind, sizeof(sct_bind)) != 0) return 1;
	if (listen(sct, 5) != 0) return 1;

	int sct_bind_size = sizeof(sct_bind);

	// loop forever to accept client connections
	sct_new = accept(sct, (SOCKADDR*)&sct_bind, &sct_bind_size);
	printf("Client connected. Client address: %s\r\n", inet_ntoa(sct_bind.sin_addr));

	//char errSqlMin[LOG_SIZE];
	//sprintf(errSqlMin, "Client connected. Client address: %s\r\n]", inet_ntoa(sct_bind.sin_addr));
	//addErrorLogMessage(errSqlMin);

	return 0;
}
int Netline::sendListXml(int cnt)
{
	char path[MAX_PATH] = {0};
	auto event_xml = pugi::xml_document{};
	auto declarationNode = event_xml.append_child(pugi::node_declaration);
	declarationNode.append_attribute(PUGIXML_TEXT("version")) = PUGIXML_TEXT("1.0");
	declarationNode.append_attribute(PUGIXML_TEXT("encoding")) = PUGIXML_TEXT("utf-8");

	auto events = event_xml.append_child(PUGIXML_TEXT("events"));

	printf("Files: [%d]\r\n", cnt);

	for (int i = 0; i < cnt; i++)
	{
		auto event = events.append_child(PUGIXML_TEXT("event"));

		event.append_attribute(PUGIXML_TEXT("timestamp")) = utf8_decode(_xmlList[i]._time).c_str();
		event.append_attribute(PUGIXML_TEXT("timestampdb")) = utf8_decode(_xmlList[i]._timestamp).c_str();
		event.append_attribute(PUGIXML_TEXT("num")) = _xmlList[i]._liplate.c_str();
		event.append_attribute(PUGIXML_TEXT("type")) = utf8_decode(_xmlList[i]._type).c_str();
		event.append_attribute(PUGIXML_TEXT("path")) = utf8_decode(_xmlList[i]._path).c_str();
		event.append_attribute(PUGIXML_TEXT("id")) = utf8_decode(_xmlList[i]._id).c_str();
	}

	//sprintf(path, "D:\\Project\\Samara\\netline\\x64\\Debug\\xml\\xml.xml");
	//event_xml.save_file(path, PUGIXML_TEXT("\t"), pugi::format_default | pugi::format_save_file_text);

	xmlstring.clear();
	xml_string_writer writer;
	event_xml.save(writer);

	xmlstring.append("HTTP/1.1 200 OK\r\nContent-Type: text/xml; charset=utf-8\r\nContent-Length: ");
	xmlstring.append(std::to_string(writer.result.size()));
	xmlstring.append("\r\nConnection: close\r\n\r\n");
	xmlstring.append(writer.result.c_str());

	//printf("xml create: %s[%d]\r\n", xmlstring.c_str(), xmlstring.size());

	// Отправляем ответ клиенту с помощью функции send
	int result = send(sct_new, xmlstring.c_str(), xmlstring.size(), 0);
	printf("result = %d\r\n", result);
	if (result == SOCKET_ERROR)
	{
		// произошла ошибка при отправле данных
		char errSend[256];
		sprintf(errSend, "Send: error is %d\r\n", WSAGetLastError());
		addErrorLogMessage(errSend);
	}

	printf("sendListXml\r\n");
	waitAccept = true;
	return result;
}
int Netline::sendDataXml(int cnt)
{
	auto data_xml = pugi::xml_document{};
	auto declarationNode = data_xml.append_child(pugi::node_declaration);
	declarationNode.append_attribute(PUGIXML_TEXT("version")) = PUGIXML_TEXT("1.0");
	declarationNode.append_attribute(PUGIXML_TEXT("encoding")) = PUGIXML_TEXT("utf-8");

	auto events = data_xml.append_child(PUGIXML_TEXT("events"));

	printf("Files: [%d]\r\n", cnt);

	for (int i = 0; i < cnt; i++)
	{
		auto event = events.append_child(PUGIXML_TEXT("event"));

	pugi::string_t _temp;

		_temp = utf8_decode(_xmlData[i]._time);
	if(_temp.size()!=0)event.append_child(PUGIXML_TEXT("timestamp")).append_child(pugi::node_pcdata).set_value(_temp.c_str());
		_temp = utf8_decode(_xmlData[i]._timestamp);
	if(_temp.size()!=0)event.append_child(PUGIXML_TEXT("timestampdb")).append_child(pugi::node_pcdata).set_value(_temp.c_str());
		_temp = utf8_decode(_xmlData[i]._type);
	if(_temp.size()!=0)event.append_child(PUGIXML_TEXT("type")).append_child(pugi::node_pcdata).set_value(_temp.c_str());
		_temp = utf8_decode(_xmlData[i]._path);
	if(_temp.size()!=0)event.append_child(PUGIXML_TEXT("path")).append_child(pugi::node_pcdata).set_value(_temp.c_str());
		_temp = _xmlData[i]._liplate;
	if(_temp.size()!=0)event.append_child(PUGIXML_TEXT("num")).append_child(pugi::node_pcdata).set_value(_temp.c_str());
		_temp = utf8_decode(_xmlData[i]._id);
	if(_temp.size()!=0)event.append_child(PUGIXML_TEXT("id")).append_child(pugi::node_pcdata).set_value(_temp.c_str());
		_temp = utf8_decode(_xmlData[i]._autopatrol);
	if(_temp.size()!=0)event.append_child(PUGIXML_TEXT("tAutoPatrolVersion")).append_child(pugi::node_pcdata).set_value(_temp.c_str());
		_temp = std::to_wstring(_xmlData[i]._Image1Present);
	if(_temp.size()!=0)event.append_child(PUGIXML_TEXT("nImage1Present")).append_child(pugi::node_pcdata).set_value(_temp.c_str());
		_temp = std::to_wstring(_xmlData[i]._Image2Present);
	if(_temp.size()!=0)event.append_child(PUGIXML_TEXT("nImage2Present")).append_child(pugi::node_pcdata).set_value(_temp.c_str());
		_temp = std::to_wstring(_xmlData[i]._nGrzPrezent);
	if(_temp.size()!=0)event.append_child(PUGIXML_TEXT("nGRZPresent")).append_child(pugi::node_pcdata).set_value(_temp.c_str());
		_temp = std::to_wstring(_xmlData[i]._XmlPresent);
	if(_temp.size()!=0)event.append_child(PUGIXML_TEXT("nXmlPresent")).append_child(pugi::node_pcdata).set_value(_temp.c_str());
		_temp = std::to_wstring(_xmlData[i]._DisplaySpeed);
	if(_temp.size()!=0)event.append_child(PUGIXML_TEXT("nDisplaySpeed")).append_child(pugi::node_pcdata).set_value(_temp.c_str());
	
		_temp = _xmlData[i]._Violation;
	if(_temp.size()!=0)event.append_child(_xmlData[i]._Violation.c_str()).append_child(pugi::node_pcdata).set_value(L"1");

	//	_temp = std::to_wstring(_xmlData[i]._OverSpeed);
	//if(_temp.size()!=0)event.append_child(PUGIXML_TEXT("nOverSpeed")).append_child(pugi::node_pcdata).set_value(_temp.c_str());
	//	_temp = std::to_wstring(_xmlData[i]._WrongDirection);
	//if(_temp.size()!=0)event.append_child(PUGIXML_TEXT("nWrongDirection")).append_child(pugi::node_pcdata).set_value(_temp.c_str());
		_temp = std::to_wstring(_xmlData[i]._Direction);
	if(_temp.size()!=0)event.append_child(PUGIXML_TEXT("nDirection")).append_child(pugi::node_pcdata).set_value(_temp.c_str());
		_temp = std::to_wstring(_xmlData[i]._SpeedThreshold);
	if(_temp.size()!=0)event.append_child(PUGIXML_TEXT("nSpeedThreshold")).append_child(pugi::node_pcdata).set_value(_temp.c_str());
		_temp = utf8_decode(_xmlData[i]._PDD);
	if (_temp[0] != '\0')event.append_child(PUGIXML_TEXT("nPDD")).append_child(pugi::node_pcdata).set_value(_temp.c_str());
		_temp = utf8_decode(_xmlData[i]._RadarId);
	if(_temp[0] != '\0')event.append_child(PUGIXML_TEXT("tRadarId")).append_child(pugi::node_pcdata).set_value(_temp.c_str());
		_temp = std::to_wstring(_xmlData[i]._Channel);
	if(_temp.size()!=0)event.append_child(PUGIXML_TEXT("tChannel")).append_child(pugi::node_pcdata).set_value(_temp.c_str());
	_temp = _xmlData[i]._SensorName;// utf8_decode(_xmlData[i]._SensorName);
	if(_temp.size()!=0)event.append_child(PUGIXML_TEXT("tSensorName")).append_child(pugi::node_pcdata).set_value(_temp.c_str());
		_temp = utf8_decode(_xmlData[i]._LocationCode);
	if(_temp[0] != '\0')event.append_child(PUGIXML_TEXT("tLocationCode")).append_child(pugi::node_pcdata).set_value(_temp.c_str());
	_temp = _xmlData[i]._RadarName;// utf8_decode(_xmlData[i]._RadarName);
	if(_temp.size()!=0)event.append_child(PUGIXML_TEXT("tRadarName")).append_child(pugi::node_pcdata).set_value(_temp.c_str());
		_temp = utf8_decode(_xmlData[i]._CertificateExpDate);
	if(_temp[0] != '\0')event.append_child(PUGIXML_TEXT("tCertificateExpDate")).append_child(pugi::node_pcdata).set_value(_temp.c_str());
		_temp = utf8_decode(_xmlData[i]._CertificateExpLimit);
	if(_temp[0] != '\0')event.append_child(PUGIXML_TEXT("tCertificateExpLimit")).append_child(pugi::node_pcdata).set_value(_temp.c_str());
		_temp = std::to_wstring(_xmlData[i]._Latitude);
	if(_temp.size()!=0)event.append_child(PUGIXML_TEXT("tLatitude")).append_child(pugi::node_pcdata).set_value(_temp.c_str());
		_temp = std::to_wstring(_xmlData[i]._Longitude);
	if(_temp.size()!=0)event.append_child(PUGIXML_TEXT("tLongitude")).append_child(pugi::node_pcdata).set_value(_temp.c_str());
		_temp = utf8_decode(_xmlData[i]._TZOffset);
	if(_temp[0] != '\0')event.append_child(PUGIXML_TEXT("nTZOffset")).append_child(pugi::node_pcdata).set_value(_temp.c_str());
		_temp = std::to_wstring(_xmlData[i]._RectNumX1);
	if(_temp.size()!=0)event.append_child(PUGIXML_TEXT("nRectNumX1")).append_child(pugi::node_pcdata).set_value(_temp.c_str());
		_temp = std::to_wstring(_xmlData[i]._RectNumY1);
	if(_temp.size()!=0)event.append_child(PUGIXML_TEXT("nRectNumY1")).append_child(pugi::node_pcdata).set_value(_temp.c_str());
		_temp = std::to_wstring(_xmlData[i]._RectNumX2);
	if(_temp.size()!=0)event.append_child(PUGIXML_TEXT("nRectNumX2")).append_child(pugi::node_pcdata).set_value(_temp.c_str());
		_temp = std::to_wstring(_xmlData[i]._RectNumY2);
	if(_temp.size()!=0)event.append_child(PUGIXML_TEXT("nRectNumY2")).append_child(pugi::node_pcdata).set_value(_temp.c_str());
		_temp = std::to_wstring(_xmlData[i]._RectNumX3);
	if(_temp.size()!=0)event.append_child(PUGIXML_TEXT("nRectNumX3")).append_child(pugi::node_pcdata).set_value(_temp.c_str());
		_temp = std::to_wstring(_xmlData[i]._RectNumY3);
	if(_temp.size()!=0)event.append_child(PUGIXML_TEXT("nRectNumY3")).append_child(pugi::node_pcdata).set_value(_temp.c_str());

	}
	
	printf("Files: [%d]\r\n", cnt);

	//	//sprintf(path, "c:\\netline\\xml\\xml.xml");
	//	//full_xml.save_file(path, PUGIXML_TEXT("\t"), pugi::format_default | pugi::format_save_file_text);

	xmlstring.clear();
	xml_string_writer writer;
	data_xml.save(writer);

	xmlstring.append("HTTP/1.1 200 OK\r\nContent-Type: text/xml; charset=utf-8\r\nContent-Length: ");
	xmlstring.append(std::to_string(writer.result.size()));
	xmlstring.append("\r\nConnection: close\r\n\r\n");
	xmlstring.append(writer.result.c_str());

	// Отправляем ответ клиенту с помощью функции send
	int result = send(sct_new, xmlstring.c_str(), xmlstring.size(), 0);

	printf("send to socket: %d\r\n",result);

	if (result == SOCKET_ERROR)
	{
		// произошла ошибка при отправле данных
		char errSend[256];
		sprintf(errSend, "Send: error is %d\r\n", WSAGetLastError());
		addErrorLogMessage(errSend);
	}

	printf("sendDataXml\r\n");
	waitAccept = true;

	return result;
}
std::wstring Netline::utf8_decode(std::string str)
{
	int size_needed = (int)str.size() + 2;// MultiByteToWideChar(CP_ACP, 0, &str[0], (int)str.size(), NULL, 0);
	std::wstring wstrTo(size_needed, 0);
	int yy = MultiByteToWideChar(CP_ACP, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
	return wstrTo;
}
std::wstring Netline::charToWString(const char* text)
{

	const size_t size = std::strlen(text);
	std::wstring wstr;
	if (size > 0) {
		wstr.resize(size);
		std::mbstowcs(&wstr[0], text, size);
	}
	return wstr;
}
std::wstring Netline::utf8_decode_2(const std::string &str)
{
	int size_needed = MultiByteToWideChar(1251, 0, &str[0], (int)str.size(), NULL, 0);
	std::wstring wstrTo(size_needed, 0);
	MultiByteToWideChar(1251, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
	return wstrTo;
}
void Netline::copyCharToWchar(wchar_t *wch, char *ch)
{
	int j = 0;
	for (int i = 0; i < strlen(ch); i++)
	{
		if (checkChar(ch[i]))
		{
			wch[j] = ch[i];
			j++;
		}
		else
		{
			wchar_t jj = 0;
			jj = ch[i + 1] & 0x00ff;
			jj = ch[i] << 8;
			wch[j] = (wchar_t)(ch[i]);

			wch[j] |= (wchar_t)(jj);
			printf("\r\n");
			j++;
			i++;
		}
	}
}
bool Netline::checkChar(char ch)
{
	char chars[12] = { '0','1','2','3','4','5','6','7','8','9','*','|' };
	for (int i = 0; i < 12; i++)
		if (chars[i] == ch)
			return true;

	return false;
}
std::string Netline::timeConvert(std::string t, Tasker::time_addon ta)
{
	int year, mont, day, hour, min, sec, offset;
	sscanf(t.c_str(), "%04d-%02d-%02d %02d:%02d:%02d", &year, &mont, &day, &hour, &min, &sec);

	tm tt1;
	tt1.tm_hour = hour;
	tt1.tm_min = min;
	tt1.tm_sec = sec;
	tt1.tm_year = year - 1900;
	tt1.tm_mon = mont;
	tt1.tm_mday = day;

	__time64_t tt = _mkgmtime64(&tt1);
	__int64 timeOffset = timeoffset;// getTimeOffset();

	if(ta ==Tasker::time_addon::PLUS)
		tt += timeOffset / 1000;

	if (ta == Tasker::time_addon::MINUS)
		tt -= timeOffset / 1000;

	tm tt2;
	errno_t res = _gmtime64_s(&tt2,&tt);
	if (res != 0) return t;

	char temp[48] = { 0 };
	//2018-01-01 00:00:00


	double to = timeOffset / (1000 * 60 * 60);
	char po[16] = { 0 };
	if(to > 0)
		sprintf(temp, "%04d-%02d-%02dT%02d:%02d:%02d+%02d", tt2.tm_year + 1900, tt2.tm_mon, tt2.tm_mday, tt2.tm_hour, tt2.tm_min, tt2.tm_sec, (int)to);
	else
		sprintf(temp, "%04d-%02d-%02dT%02d:%02d:%02d-%02d", tt2.tm_year + 1900, tt2.tm_mon, tt2.tm_mday, tt2.tm_hour, tt2.tm_min, tt2.tm_sec, (int)to);

	return std::string(temp);
}
std::string Netline::timeConvert2(std::string t)
{
	//int year, mont, day, hour, min, sec, offset;
	//sscanf(t.c_str(), "%04d-%02d-%02d %02d:%02d:%02d", &year, &mont, &day, &hour, &min, &sec);

	//tm tt1;
	//tt1.tm_hour = hour;
	//tt1.tm_min = min;
	//tt1.tm_sec = sec;
	//tt1.tm_year = year - 1900;
	//tt1.tm_mon = mont;
	//tt1.tm_mday = day;

	//__time64_t tt = _mkgmtime64(&tt1);
	__int64 timeOffset = timeoffset;// getTimeOffset();

	//if (ta == Tasker::time_addon::PLUS)
	//	tt += timeOffset / 1000;

	//if (ta == Tasker::time_addon::MINUS)
	//	tt -= timeOffset / 1000;

	//tm tt2;
	//errno_t res = _gmtime64_s(&tt2, &tt);
	//if (res != 0) return t;

	char temp[48] = { 0 };
	//2018-01-01 00:00:00

	t.replace(10, 1, "T");

	double to = timeOffset / (1000 * 60 * 60);
	char po[16] = { 0 };
	if (to > 0)
		sprintf(temp, "%s+%02d", t.c_str(), (int)to);
	else
		sprintf(temp, "%s-%02d", t.c_str(), (int)to);

	return std::string(temp);
}
//std::wstring Netline::utf8_decode(std::string str)
//{
//	int size_needed = (int)str.size() + 2;// MultiByteToWideChar(CP_ACP, 0, &str[0], (int)str.size(), NULL, 0);
//	std::wstring wstrTo(size_needed, 0);
//	int yy = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
//	return wstrTo;
//}
//std::string Netline::utf8_encode(std::wstring wstr)
//{
//	if (wstr.length() == 0)
//		return "";
//
//	if (wstr.empty()) return std::string();
//	int size_needed = (int)wstr.size() + 2;// WideCharToMultiByte(1251, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
//	std::string strTo(size_needed, 0);
//	int yy = WideCharToMultiByte(1251, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
//	return strTo;
//}
