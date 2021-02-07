#pragma once
#include "OLSO.h"

#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
#define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
#else
#define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
#endif

//#include "opencv2/core/core.hpp"
//#include "opencv2/highgui/highgui.hpp"
//#include "opencv2/imgproc/imgproc_c.h"

//полный кадр в памяти (джпег + структуры + доп кадры)
unsigned char jpeg_main[MAX_SIZE_MAIN_BMP] = { 0 };

//доп.кадры
unsigned char jpeg_main2[MAX_SIZE_MAIN_BMP] = { 0 };

unsigned char jpeg_carExtra[MAX_SIZE_MAIN_BMP] = { 0 };
unsigned char jpeg_observe0[MAX_SIZE_MAIN_BMP] = { 0 };
unsigned char jpeg_observe1[MAX_SIZE_MAIN_BMP] = { 0 };
unsigned char jpeg_observe2[MAX_SIZE_MAIN_BMP] = { 0 };
//unsigned char jpeg_collage[MAX_SIZE_MAIN_BMP] = { 0 };

unsigned char bmp_carExtra[MAX_SIZE_MAIN_BMP] = { 0 };
unsigned char bmp_grz[MAX_SIZE_MAIN_BMP] = { 0 };
unsigned char jpg_grz[MAX_SIZE_MAIN_BMP] = { 0 };

unsigned char jpeg_sign_0[SIZE_SIGN] = { 0 };
unsigned char jpeg_sign_1[SIZE_SIGN] = { 0 };

wchar_t path_sign_0[MAX_PATH] = { 0 };
wchar_t path_sign_1[MAX_PATH] = { 0 };

unsigned char mkv_video[MAX_SIZE_MAIN_BMP] = { 0 };
unsigned char avi_video[MAX_SIZE_MAIN_AVI] = { 0 };




//int jpeg_sign_size_0 = SIZE_SIGN;
//int jpeg_sign_size_1 = SIZE_SIGN;


using namespace rapidjson;

wchar_t *g_pathMain = L"D:/arch/all_fixations/";

lws_context *lwscontext = nullptr;
lws *web_socket = nullptr;
lws_client_connect_info ccinfo = { 0 };
unsigned char *lws_message = new unsigned char[LWS_MESSAGE];

//unsigned char lws_message[4096];
int temp_diff_message_lws = 0;
int diff_message_lws = -1;
COLSO *ownObj = nullptr;

struct lws_mwssage 
{
	int number;
};

int ws_callback(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len);

lws_protocols protocols[2] =
{
	{
		"olympus_remote_ctrl",				/* name */
		ws_callback,		/* callback */
		sizeof(lws_mwssage),
		LWS_MESSAGE//EXAMPLE_RX_BUFFER_BYTES//sizeof(WsPerSessionData)		/* per_session_data_size */
	},
	{
		nullptr, nullptr, 0	/* End of list */
	}
};

time_t zero_time = 0;
std::wstring zero_type = L"sign";

char error_code[1024] = { 0 };
char error_string[1024] = { 0 };
SOAP_ENV__Fault *fault_describe = new SOAP_ENV__Fault();



COLSO::COLSO()
{
	skat_data = { 0 };
	ownObj = this;

	memset(jpeg_main, 0, MAX_SIZE_MAIN_BMP);
	memset(jpeg_main2, 0, MAX_SIZE_MAIN_BMP);
	memset(jpeg_observe0, 0, MAX_SIZE_MAIN_BMP);
	memset(jpeg_observe1, 0, MAX_SIZE_MAIN_BMP);
	memset(jpeg_observe2, 0, MAX_SIZE_MAIN_BMP);
	memset(jpeg_carExtra, 0, MAX_SIZE_MAIN_BMP);

	memset(bmp_carExtra, 0, MAX_SIZE_MAIN_BMP);
	memset(bmp_grz, 0, MAX_SIZE_MAIN_BMP);
	memset(jpg_grz, 0, MAX_SIZE_MAIN_BMP);

	memset(&compress_cinfo, 0, sizeof(jpeg_compress_struct));
	memset(&compress_jerr, 0, sizeof(jpeg_error_mgr));
	memset(&decompress_cinfo, 0, sizeof(jpeg_decompress_struct));
	memset(&decompress_jerr, 0, sizeof(jpeg_error_mgr));

	modeRegion = mode::MOSKVA;

	path_of_id_backup_file = new wchar_t*[MAX_CHAR_ID];
	for (int i = 0; i < MAX_CHAR_ID; ++i)
	{
		path_of_id_backup_file[i] = new wchar_t[MAX_PATH];
		memset(path_of_id_backup_file[i], 0, sizeof(wchar_t)*MAX_PATH);
	}
}

COLSO::~COLSO()
{
}
int COLSO::Stop()
{
	bRunning = false;
	Sleep(10000);//задержка при остановке процесса
	return 1;
}

int COLSO::Start(wchar_t *servicePath)
{
	char errSql[MAX_PATH_ERR_STRING_LOG];
	bRunning = true;
	//wcscpy(path_of_log_error_file, L"d:\\log-error-soap.log");
	memset(path_of_id_temp_file, 0, sizeof(wchar_t)*MAX_PATH);
	memset(path_of_id_file,0,sizeof(wchar_t)*MAX_PATH);
	memset(path_of_log_error_file, 0, sizeof(wchar_t)*MAX_PATH);
	memset(path_root, 0, sizeof(wchar_t)*MAX_PATH);
	memset(path_of_backup_dir, 0, sizeof(wchar_t)*MAX_PATH);

	for (int i = wcslen(servicePath) - 1; i > 0; i--)
	{
		if (servicePath[i] == L'\\')
		{
			//id.txt
			printf("\r\n");
			wcsncpy(path_of_id_file, servicePath, i);
			wcscat(path_of_id_file, L"\\");
			wcscat(path_root, path_of_id_file);

			//log_error
			wcscat(path_of_log_error_file, path_root);
			wcscat(path_of_log_error_file, L"log-error-soap.log");

			wcscat(path_of_id_file, L"id.txt");

			//id-temp.txt
			wcscat(path_of_id_temp_file, path_root);
			wcscat(path_of_id_temp_file, L"id-temp.txt");
	
			sprintf(errSql, "path[ID] = %s\r\npath[ID-TEMP] = %s\r\npath[LOG] = %s\r\n", utf8_encode(path_of_id_file).c_str(), utf8_encode(path_of_id_temp_file).c_str(), utf8_encode(path_of_log_error_file).c_str());
			addErrorLogMessage(errSql,false);

			wcscat(path_of_backup_dir, path_root);
			wcscat(path_of_backup_dir, L"backup");
			CreateDirectory(path_of_backup_dir,nullptr);

			for (int i = 0; i < MAX_CHAR_ID; ++i)
			{
				wcscat(path_of_id_backup_file[i], path_of_backup_dir);
				swprintf(path_of_id_backup_file[i] + wcslen(path_of_backup_dir), L"\\p.%02d", i);
			}

			//wprintf(L"id txt is: %s[%d]\r\n", path_of_id_file,i);
			//wprintf(L"log error txt is: %s[%d]\r\n", path_of_log_error_file, i);

			//addErrorLogMessage("todo-0");
			break;
		}
	}

	//std::thread *thr = new std::thread(&COLSO::thrMain, this);

	ioThread = new std::thread(&iothread,this);

	return 0;
}
int COLSO::lwsInit()
{
	lws_context_creation_info info;

	memset(&info, 0, sizeof(info));

	info.port = CONTEXT_PORT_NO_LISTEN;
	info.protocols = protocols;
	info.gid = -1;
	info.uid = -1;

	lwscontext = lws_create_context(&info);

	return 1;
}
int COLSO::lwsMain()
{
	timeval lws_tv;

	gettimeofday(&lws_tv, NULL);

	///* Connect if we are not connected to the server. */
	if (!web_socket && lws_tv.tv_sec != lws_old)
	{
		//addErrorLogMessage("lwsMain-1");
		memset(&ccinfo, 0, sizeof(lws_client_connect_info));
		ccinfo.context = lwscontext;
		ccinfo.address = "localhost";
		//ccinfo.origin = "7.140.114.54";
		//ccinfo.host = "7.140.114.54"; 
		//ws_canonical_hostname(lwscontext);
		ccinfo.host = lws_canonical_hostname(lwscontext);
		ccinfo.port = 7682;
		ccinfo.path = "/";
		ccinfo.protocol = protocols[0].name;
		//ccinfo.pwsi = &web_socket;
		web_socket = lws_client_connect_via_info(&ccinfo);
	}

	if (lws_tv.tv_sec != lws_old)
	{
		//addErrorLogMessage("lwsMain-2");
		lws_callback_on_writable(web_socket);
		lws_old = lws_tv.tv_sec;
	}

	//addErrorLogMessage("lwsMain-3");
	lws_service(lwscontext, 100);

	return 1;
}
//callback_dumb_increment
int ws_callback(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
{
	switch (reason)
	{
	case LWS_CALLBACK_HTTP:
	{
		//ownObj->addErrorLogMessage("LWS_CALLBACK_HTTP");
		lws_callback_on_writable(wsi);
		break;
	}

	case LWS_CALLBACK_CLIENT_ESTABLISHED:
	{
		//ownObj->addErrorLogMessage("LWS_CALLBACK_CLIENT_ESTABLISHED");
		lws_callback_on_writable(wsi);
		break;
	}

	case LWS_CALLBACK_CLIENT_RECEIVE:
		/* Handle incomming messages here. */
		//ownObj->addErrorLogMessage("LWS_CALLBACK_CLIENT_RECEIVE");
		break;

	case LWS_CALLBACK_CLIENT_WRITEABLE:
	{
		//char ddfg[MAX_PATH] = { 0 };
		//sprintf(ddfg,"LWS_CALLBACK_CLIENT_WRITEABLE: temp_diff_message_lws = %d diff_message_lws = %d\r\n", temp_diff_message_lws, diff_message_lws);
		//ownObj->addErrorLogMessage(ddfg,false);

		//if(diff_message_lws != temp_diff_message_lws)
		if (temp_diff_message_lws >= 0)
		{
			memset(lws_message, 0, LWS_MESSAGE);
			int res = sprintf((char*)lws_message, "{\"jsonrpc\": \"2.0\", \"method\" : \"set\",\"params\" : {\"count_violations_remained_send\":%d}, \"id\" : \"1\"}", temp_diff_message_lws);
			int nbb = lws_write(wsi, lws_message, res, LWS_WRITE_TEXT);
			diff_message_lws = temp_diff_message_lws;
		}
		break;
	}

	case LWS_CALLBACK_CLOSED:
	{
		//ownObj->addErrorLogMessage("LWS_CALLBACK_CLOSED", false);
		web_socket = nullptr;
		break;
	}

	case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
	{
		//ownObj->addErrorLogMessage("LWS_CALLBACK_CLIENT_CONNECTION_ERROR", false);
		web_socket = nullptr;
		break;
	}
	default:
		break;
	}

	return 0;
}
void iothread(COLSO* parent)
{
	parent->lwsInit();

	Sleep(3000);

	while (parent->bRunning)
	{
		Sleep(50);
		parent->lwsMain();
	};
}

int COLSO::addErrorLogMessage(char* p,bool rn = false)
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
bool COLSO::fileErrorPresent()
{
	struct _stat buffer;

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
		int rmv = _wremove(path_of_log_error_file);
		if (rmv == 0)
		{
			FILE *f = _wfopen(path_of_log_error_file, L"w");
			fclose(f);//_wfopen(path_of_log_error_file [fileErrorPresent]
			res = _wstat(path_of_log_error_file, &buffer);
		}
	}

	if (res == 0)
		return true;
	else
		return false;
}
int COLSO::saveLog(char *p)
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
int COLSO::saveVersion(char *p)
{
	FILE *f;

	//if (fileErrorPresent())
	{
		f = _wfopen(L"c:\\soap\\version.txt", L"wt");
		size_t size = fwrite(p, 1, strlen(p), f);
		fclose(f);//_wfopen(path_of_log_error_file [saveLog]
	}

	return 0;
}
int COLSO::thrMain(/*int argc, wchar_t* argv[]*/)
{
	saveVersion(LOG_VERSION);
	addErrorLogMessage(LOG_VERSION);

#ifdef POLITEH_STLE
	addErrorLogMessage("PESHEHOD FORMAT!");
#endif


	std::srand(RAND_MAX);

	bPipeConnect = false;
	serv = new ChanelizerServer(L"\\\\.\\pipe\\group_five");

	//if (serv->wait_for_connection())
	//{
	//	printf("pipe\\group_five connected!\n");
	//	bPipeConnect = true;
	//}
	//else
	//{
	//	printf("pipe\\group_five IS NOT connected!\n");
	//	bPipeConnect = false;
	//}



	setlocale(LC_ALL, "ru-RU");
	if(load_settings() == false)
		addErrorLogMessage("load_settings error!");

	//memset(&violaData, 0, sizeof(viola));

//#ifdef DEBUG_PROC
	//saveIdBackup(456789);
	//id_backup = readIdBackup();
	//saveIdBackup(456789);
//	objDuplo = connectToServ(set.serverPath);
//	printf("connectToServ = 0x%x\r\n", objDuplo);
//#endif

	senddata_psd = new ns1__process();
	senddata_psd->message = new ns1__message();// ->soap_alloc();
	senddata_psd->message->tr_USCOREcheckIn = new ns1__trCheckIn(); //->soap_alloc();

	senddata_psd->message->tr_USCOREcheckIn->v_USCOREphoto_USCOREgrz = new xsd__base64Binary();
	senddata_psd->message->tr_USCOREcheckIn->v_USCOREphoto_USCOREgrz->__size = MAX_SIZE_MAIN_BMP;
	senddata_psd->message->tr_USCOREcheckIn->v_USCOREphoto_USCOREgrz->__ptr = jpg_grz;

	senddata_psd->message->tr_USCOREcheckIn->v_USCOREphoto_USCOREts = new xsd__base64Binary();
	senddata_psd->message->tr_USCOREcheckIn->v_USCOREphoto_USCOREts->__size = MAX_SIZE_MAIN_BMP;
	senddata_psd->message->tr_USCOREcheckIn->v_USCOREphoto_USCOREts->__ptr = jpeg_carExtra;// jpeg_main;

	skatToDuploExtra = new ns1__photoExtra();
	skatToDuploExtra->v_USCOREphoto_USCOREextra = new xsd__base64Binary();

	skatToDuploExtra2 = new ns1__photoExtra();
	skatToDuploExtra2->v_USCOREphoto_USCOREextra = new xsd__base64Binary();

	skatToDuploExtra3 = new ns1__photoExtra();
	skatToDuploExtra3->v_USCOREphoto_USCOREextra = new xsd__base64Binary();

	skatToDuploExtra4 = new ns1__photoExtra();
	skatToDuploExtra4->v_USCOREphoto_USCOREextra = new xsd__base64Binary();
	
	skatToDuploSignCar = new ns1__photoExtra();
	skatToDuploSignCar->v_USCOREphoto_USCOREextra = new xsd__base64Binary();
	skatToDuploSignCar->v_USCOREphoto_USCOREextra->__size = SIZE_SIGN;
	skatToDuploSignCar->v_USCOREphoto_USCOREextra->__ptr = jpeg_sign_0;

	skatToDuploSignView = new ns1__photoExtra();
	skatToDuploSignView->v_USCOREphoto_USCOREextra = new xsd__base64Binary();
	skatToDuploSignView->v_USCOREphoto_USCOREextra->__size = SIZE_SIGN;
	skatToDuploSignView->v_USCOREphoto_USCOREextra->__ptr = jpeg_sign_1;

	skatToDuploExtraVideo = new ns1__photoExtra();
	skatToDuploExtraVideo->v_USCOREphoto_USCOREextra = new xsd__base64Binary();
	skatToDuploExtraVideo->v_USCOREphoto_USCOREextra->__size = MAX_SIZE_MAIN_BMP;
	skatToDuploExtraVideo->v_USCOREphoto_USCOREextra->__ptr = mkv_video;

	skatToDuploExtraAviVideo = new ns1__photoExtra();
	skatToDuploExtraAviVideo->v_USCOREphoto_USCOREextra = new xsd__base64Binary();
	skatToDuploExtraAviVideo->v_USCOREphoto_USCOREextra->__size = MAX_SIZE_MAIN_AVI;
	skatToDuploExtraAviVideo->v_USCOREphoto_USCOREextra->__ptr = avi_video;


	senddata_psd->message->photo_USCOREextra.reserve(4);

//	InitJpeg();


#ifdef DEBUG_PROC
//	connectToServ(set.serverPath);
//	//std::string strFirstFile11 = "";
//	//int res11 = dbConnect();
//	//__int64 min_id11 = dbQueryMinId();
//	//__int64 min_date_id11 = dbQueryMinIdByDate(&strFirstFile11);
//
//	std::string reg0 = "";
//	if (set.region == "moscow")
//	{
//		modeRegion = mode::MOSKVA;
//		reg0 = "MOSKVA";
//	}
//
//	if (set.region == "ulyanovsk")
//	{
	//char p1[MAX_PATH] = { 0 };
	//char p2[MAX_PATH] = { 0 };
	//getCoordAndPlace(p1,p2);

	//objDuplo = connectToServ(set.serverPath);
	
	//int ggg = sizeof(fileInfo);

		//load_settings();
		modeRegion = mode::MOSKVA;
//		reg0 = "ULYANOVSK";
//	}
//
		while (true)
		{
			//	int ghgh = 0;
			//	fillData(L"D:\\__1\\22\\1\\1711032_2018_07_14_08_37_27_36_00_07.jpg", L"D:\\__1\\22\\1\\1711032_2018_07_14_08_37_27_36_00_07_fr.jpg");
			//	fillData(L"D:\\__1\\32\\1511032_2018_12_23_15_07_55_31_00_00.jpg", L"D:\\__1\\32\\1511032_2018_12_23_15_07_55_31_00_00_fr.jpg");

			//politeh
			//fillData(L"D:\\arch\\all_fixations\\2019\\09\\29\\1811013_2019_06_14_19_02_18_73_00_15.jpg", L"D:\\arch\\all_fixations\\2019\\09\\29\\1811013_2019_06_14_19_02_18_73_00_15_fr.jpg");

			//politeh-0
			//fillData(L"D:\\__1\\54\\1801029_2019_09_27_05_04_17_41_00_00.jpg",L"D:\\__1\\54\\1801029_2019_09_27_05_04_17_41_00_00_fr.jpg");
			
			

			//fillData(L"D:\\__1\\53\\1809116_2019_09_19_13_36_09_04_00_07.jpg", L"D:\\__1\\53\\1809116_2019_09_19_13_36_09_04_00_07_fr.jpg");
			int rrr = fillData(L"D:\\temp\\__1\\1807157_2020_07_27_06_29_55_33_00_16.jpg", L"D:\\temp\\__1\\1807157_2020_07_27_06_29_55_33_00_16_fr.jpg");
			
			//fillData(L"D:\\__1\\55\\1809114_2019_10_02_19_12_45_34_00_00.jpg", L"D:\\__1\\55\\1809114_2019_10_02_19_12_45_34_00_00_fr.jpg");
			if(rrr == 0)
			sendData(objDuplo);

			
			
			//sendData(objDuplo);

			Sleep(1000);
		}
//	while (1)
//	{
//		//fillData(L"D:\\__1\\12\\1506018_2018_04_01_16_35_32_40_00_07.jpg", L"D:\\__1\\12\\1506018_2018_04_01_16_35_32_40_00_07_fr.jpg");
//		//fillData(L"D:\\__1\\11\\1506019_2018_03_21_17_53_49_49_00_00.jpg", L"D:\\__1\\11\\1506019_2018_03_21_17_53_49_49_00_00_fr.jpg");
//		//fillData(L"D:\\__1\\9\\1506018_2018_03_21_20_26_37_40_00_00.jpg", L"D:\\__1\\9\\1506018_2018_03_21_20_26_37_40_00_00_fr.jpg");
//		fillData(L"D:\\__1\\15\\1804005_2018_05_23_08_00_36_46_00_00.jpg", L"D:\\__1\\15\\1804005_2018_05_23_08_00_36_46_00_00_fr.jpg");
//		sendData(objDuplo);
//
//		temp_diff_message_lws = ghgh++;
//		lwsMain();
//
//		Sleep(100);
//	};
#endif

	//double hScale = 0.9;
	//double vScale = 0.9;
	//int    lineWidth = 3.9;
	//IplImage* iplDebugImage = 0;
	//CvFont font_black_string;
	//cvInitFont(&font_black_string, CV_FONT_HERSHEY_SIMPLEX | CV_FONT_ITALIC, hScale, vScale, 0, lineWidth);
	//iplDebugImage = cvCreateImage(cvSize(nW, nH), IPL_DEPTH_8U, 1);
	//iplDebugImage->imageData = (char*)tempToView;
	//cvResize(iplDebugImage, pDebugResizeImage, CV_INTER_LINEAR);
	//void OffsetImage(Mat &image, cv::Scalar bordercolour, int xoffset, int yoffset)
	//{
	//	Mat offsetImage = Mat::zeros(image.size(), image.type());
	//	padded = Mat(image.rows + 2 * abs(yoffset), image.cols + 2 * abs(xoffset), CV_8UC3, bordercolour);
	//	image.copyTo(padded(Rect(abs(xoffset), abs(yoffset), image.cols, image.rows)));
	//	image = Mat(padded, Rect(abs(xoffset) + xoffset, abs(yoffset) + yoffset, image.cols, image.rows));
	//}

	///****************begin*********************
	// айди хранить снаружи
	// в инишнике
	// или хранить дату от которой отчитывать при первом запуске

	time_t start_timer;
	tm * start_ptm;

	while(true)
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

	id = readId();
	printf("id = %I64d\r\n", id);
	std::string strFirstFile = "";
	int res = dbConnect();
	__int64 min_id = dbQueryMinId();
	__int64 min_date_id = dbQueryMinIdByDate(&strFirstFile);
	__int64 id_temp = readIdTemp();
	id_backup = 0;
	id_backup = readIdBackup();

	//2018-03-20 18:00:00.000000
	__int64 start_date_id = dbQueryStartIdByDate(set.start_date);

	std::string reg = "REGION IS NOT VALID!!!![moscow][ulyanovsk][buryatia][moscow2][stavropol]";
	if (set.region == "moscow")
	{
		modeRegion = mode::MOSKVA;
		reg = "MOSKVA";
	}

	if (set.region == "ulyanovsk")
	{
		modeRegion = mode::ULYANOVSK;
		reg = "ULYANOVSK";
	}

	if (set.region == "buryatia")
	{
		modeRegion = mode::BURYATIYA;
		reg = "BURYATIYA";
	}

	if (set.region == "moscow2")
	{
		modeRegion = mode::MOSKVA2;
		reg = "MOSKVA2";
	}

	if (modeRegion == mode::MOSKVA)
	{
		if (set.c_type.compare("p") == 0)
			addErrorLogMessage("PEREDVIGKA");
		else
		{
			addErrorLogMessage("STACIONAR");
		}
	}

	if (modeRegion == mode::MOSKVA2)
	{
		if (set.c_type.compare("p") == 0)
			addErrorLogMessage("PEREDVIGKA");
		else
		{
			addErrorLogMessage("STACIONAR");
		}
	}
	
	if (set.region == "stavropol")
	{
		modeRegion = mode::STAVROPOL;
		reg = "STAVROPOL";
	}

	printf("min_id = %I64d\r\n", min_id);
	printf("min_date_id = %I64d\r\n", min_date_id);
	
	//todo
	char errSqlMin[MAX_PATH_ERR_STRING_LOG];
	sprintf(errSqlMin, "[region = %s][%s][min_id = %I64d][id = %I64d]\r\n[min_date_id = %I64d][id_temp = %I64d][id_backup = %I64d][%s]", set.region.c_str(),reg.c_str(), min_id, id, min_date_id, id_temp, id_backup, strFirstFile.c_str());
	addErrorLogMessage(errSqlMin);

	if (id == 0) id = start_date_id;
	
	if (id < min_id) id = min_id;			//МИНИМАЛЬНЫЙ АЙДИШНИК В БД
	if (id < min_date_id) id = min_date_id;	//МИНИМАЛЬНЫЙ АЙДИШНИК ТОГО ФАЙЛА ЧТО ХРАНИТСЯ В ARCH
	if (id < id_temp) id = id_temp;			//ЗАПОМНЕННЫЙ АЙДИШНИК
	if (id < id_backup) id = id_backup;		//ЗАПОМНЕННЫЙ АЙДИШНИК ЧЕРЕЗ BACKUP

	//char err_debug[MAX_PATH_ERR_STRING_LOG];
	//sprintf(err_debug, "[min_id = %I64d][id = %I64d]\r\n[min_date_id = %I64d][id_temp = %I64d][id_backup = %I64d]", min_id, id, min_date_id, id_temp, id_backup);
	//addErrorLogMessage(err_debug);

	middle_time_send = 0;
	middle_cnt_send = 0;

	printf("dbConnect = %d\r\n", res);
	//int nPingCnt = 0;
	//int nMaxIdCnt = 301;


	//todo
	char err[MAX_PATH_ERR_STRING_LOG];

	bool empty_repeat = false;
	reconnectCnt = 0;

	///признак работы программы
	while (bRunning)
	{
		// слушаем айдишник с айди от БД
		// функция для записи айдишника и добавления к списку
		// функция длдя чтения из списка, при чтени следующего айдишника предыдущий удаляем

		// коннект до сервера
		// пинг до него
		// проверка на ошибки
		objDuplo = connectToServ(set.serverPath);
		printf("connectToServ = %s\r\n", set.serverPath);

		while (objDuplo != 0 && bRunning)
		{
			// проверка на новый айдишник, берем из потока thListenInputId черех mutex
			// там должен быть режим без списка просто
			// сами проверяем наличие нового айдишника

			id_last = dbQueryMaxId();

			temp_diff_message_lws = id_last - id;

			//todo
			//char errDuplo11[MAX_PATH_ERR_STRING_LOG];
			//sprintf(errDuplo11, "11:temp_diff_message_lws=%d id_last=%I64d id=%I64d", temp_diff_message_lws, id_last, id);
			//addErrorLogMessage(errDuplo11);

			//!lwsMain();

			if (id_last <= 0)
			{
				Sleep(3000);//задержка при get max id
				continue;
			}

			if (id_last >= id)
			{
					char erridd[256];
					sprintf(erridd, "[start]id_last = %I64d id = %I64d", id_last, id);
					addErrorLogMessage(erridd);

				// путь к нарушению
				// берем запрос к БД
				// и вытягиваем данные о нарушении
				//addErrorLogMessage("send path query....");

				memset(dbRes, 0, sizeof(dbResult)*MAX_COUNT_SQL * 2);
				addErrorLogMessage("dbQueryGetPath");
				int resCnt = dbQueryGetPath(id, id_last);
				//addErrorLogMessage("dbQueryGetPath-end");
				if (resCnt <= 0)
				{
					//todo
					char errSql[256];
					sprintf(errSql, "dbQueryGetPath = %d", resCnt);
					addErrorLogMessage(errSql);

					if (resCnt == -4)
					{
						if (id_last > id)
						{
							addErrorLogMessage("mysql_num_rows = 0");
							if((id_last - id) >= MAX_COUNT_SQL)
								id = id + MAX_COUNT_SQL;
							else
								id = id_last;
							continue;
						}
					}
					else
					{
						Sleep(3000);//задержка при получении путей
						continue;
					}
				}

				int tempResCnt = 0;

				std::string s1;
				std::string s2;

				while ((resCnt > tempResCnt) && bRunning)
				{
					int index0 = tempResCnt;
					int index1 = tempResCnt + 1;

					// берем данные из граф.подписи
					// коллаж итп.
					
					s1 = utf8_encode(dbRes[index0].wpath);
					s2 = utf8_encode(dbRes[index1].wpath);

					temp_diff_message_lws = id_last - id;
					//todo
					//char errDuplo[MAX_PATH_ERR_STRING_LOG];
					//sprintf(errDuplo, "1:temp_diff_message_lws=%d id_last=%I64d id=%I64d", temp_diff_message_lws,id_last,id);
					//addErrorLogMessage(errDuplo);

					//!lwsMain();
					//addErrorLogMessage("lwsMain-end");

					if ((id_last - id) == 0)
					{
						Sleep(5000);//задержка при разнице в 0, ждем пока файл запишеться
						//unRepeatLastSend++;
						//if (unRepeatLastSend > 2) 
						//	break;
						addErrorLogMessage("timeout 5s diff = 0");
					}

					//if ((id_last - id) != 0)
					//unRepeatLastSend = 0;

					bool bSaveId = true;

					if (fillData(dbRes[index0].wpath, dbRes[index1].wpath) == 0)
					{
						//todo
						memset(err, 0, MAX_PATH_ERR_STRING_LOG);
						sprintf(err, "\r\n[%I64d][%d]s1=%s[%d]\r\n[%I64d][%d]s2=%s[%d]\r\ncheck id = %I64d[max is %I64d][diff=%I64d]", id, index0, s1.c_str(), wcslen(dbRes[index0].wpath), id, index1, s2.c_str(), wcslen(dbRes[index1].wpath), id, id_last, id_last - id);
						addErrorLogMessage(err);

						//memset(err, 0, 1024);
						//sprintf(err, "check id = %I64d[max is %I64d][diff=%I64d]", id, id_last, id_last - id);
						//addErrorLogMessage(err);
			
						//addErrorLogMessage("lwsMain-start");
						temp_diff_message_lws = id_last - id;
						//todo
/*						char errDuplo[MAX_PATH_ERR_STRING_LOG];
						sprintf(errDuplo, "2:temp_diff_message_lws=%d id_last=%I64d id=%I64d", temp_diff_message_lws, id_last, id);
						addErrorLogMessage(errDuplo)*/
						//!lwsMain();
						//addErrorLogMessage("lwsMain-end");

						//addErrorLogMessage("sendData start");
						int fault = sendData(objDuplo);
						//addErrorLogMessage("sendData end");

						if (retFaultAction(fault))
						{
							bSaveId = false;
							// повторный отсыл
							// до бесконечности!!!
							// пока смерть не разлучит нас
							printf("In cycle while error!!\r\n");
							int repeat_send_error = fault;
							while (retFaultAction(repeat_send_error) && bRunning)
							{
								//todo
								char errSend[MAX_PATH_ERR_STRING_LOG];
								sprintf(errSend, "try to send. fault = %d.repeat_send_error = %d. id_last = %I64d[diff=%I64d][id=%I64d]", fault, repeat_send_error, id_last, id_last - id,id);
								addErrorLogMessage(errSend);

								id_last = dbQueryMaxId();
								temp_diff_message_lws = id_last - id;
								//todo
								//char errDuplo[MAX_PATH_ERR_STRING_LOG];
								//sprintf(errDuplo, "3:temp_diff_message_lws=%d id_last=%I64d id=%I64d", temp_diff_message_lws, id_last, id);
								//addErrorLogMessage(errDuplo);
								//!lwsMain();

								repeat_send_error = sendData(objDuplo);
								Sleep(5000);//если ошибки при отсылке

								reconnectCnt++;
								if (reconnectCnt > 10)
								{
									addErrorLogMessage("REconnect!!");
									reconnectCnt = 0;
									if (objDuplo != 0)
									{
										objDuplo->soap_close_socket();
										delete objDuplo;
										objDuplo = connectToServ(set.serverPath);

										char errSendReconnect[256];
										sprintf(errSendReconnect, "Reconnect = %s", set.serverPath);
										addErrorLogMessage(errSendReconnect);
										printf("connectToServ = %s\r\n", set.serverPath);
									}
								}
							}

							if(repeat_send_error == SOAP_OK) bSaveId = true;
						}
						else
							bSaveId = true;

						Sleep(50);//100
					}
					else
					{
						//todo
						char errFill[256];
						sprintf(errFill, "errFillData = %d[check id = %I64d][diff=%I64d]", ++nerrCntFillData, id, id_last - id);
						addErrorLogMessage(errFill);
					}

					if (bSaveId)
					{
						if (id_temp <= id)
						{
							if (id % set.cntBackup == 0)
								saveIdBackup(id);

							if (id % set.cntTemp == 0)
							saveIdTemp(id);

							saveId(id);
							id_temp = id;
							id++;
						}
						else
							id = id_temp;
					}

					tempResCnt = tempResCnt + 2;
				}
			}

			////раз в 600 секунд
			//if (nPingCnt > 6000)
			//{
				//ping_alive();
			//	nPingCnt = 0;
			//}

			Sleep(TIMEOUT);//задержка при отправке группы 3000
			//nPingCnt++;
			//nMaxIdCnt++;
		}

		printf("duplo == 0\r\n");
		//todo
		char firstExit[256];
		sprintf(firstExit, "first exit: objDuplo = 0x%x bRunning = %d", objDuplo, bRunning);
		addErrorLogMessage(firstExit);
		//addErrorLogMessage("first exit");

		//разрыв связи или что-то ужасное
		if (objDuplo != 0)
			objDuplo->soap_close_socket();

		objDuplo = 0;
		Sleep(10000);//задержка при закрытии потока
	}

	//todo
	char lastExit[256];
	sprintf(lastExit, "last exit: objDuplo = 0x%x bRunning = %d", objDuplo, bRunning);
	addErrorLogMessage(lastExit);
	//addErrorLogMessage("last exit");

	//delete skatToDuplo;
	//delete skatToDuploExtra;

	//delete __ptrSenddata_Grz64Bin;
	//delete __ptrSenddata_Ts64Bin;
	//delete __ptrSenddata_Extra64Bin;

	delete fault_describe;
	delete serv;
	//delete pSenddata_psd;
	lws_context_destroy(lwscontext);
	printf("delete skatToDuplo\r\n");

	for (int i = 0; i < MAX_CHAR_ID; ++i)
		delete[] path_of_id_backup_file[i];

	delete[] path_of_id_backup_file;

//	KillJpeg();

	return 1;
}
bool COLSO::retFaultAction(int faultCode)
{
	if (faultCode == 8003)return true;
	if (faultCode == 8006)return true;
	if (faultCode == UNUSUAL_ERROR)return true;

	if (faultCode == -3)return false;
	if (faultCode == -2)return false;
	if (faultCode == -1)return false;
	if (faultCode == 0)return false;
	if (faultCode == 100)return false;
	if (faultCode == 101)return false;
	if (faultCode == 102)return false;
	if (faultCode == 103)return false;
	if (faultCode == 104)return false; 
	if (faultCode == 200)return false;

	return true;
}
int COLSO::checkFault()
{
	/*
	1.Ошибки валидации данных о проезде:
	"100" - Передано пустое сообщение;
	"101 -  Передано сообщение без данных о проезде
	"102" - В сообщении нет идентификатора камеры
	"103" - Требуется не пустой идентификатор камеры
	"104" - Не передано время проезда.

	2. Пользовательские ошибки сохранения данных проезда в БД Траффик  (TRAFFIC-ERROR),
	например, дубликат проезда или отсутствие камеры в справочнике камер ЦАФАП.
	(В этом случае повторную отсылку данных проезда отправлять не надо).
	"200" - текст ошибки (TRAFFIC-ERROR)

	3. SQL-ошибки СУБД при сохранении данных проезда в БД Траффик (SQL-ERROR). Код ошибки - это 5-ти значный код PostgreSQL (и префикс “SQL_”), например
	"SQL_08003" -  connection_does_not_exist
	"SQL_08006" -  connection_failure
	текст SQL-ошибки НЕ передаётся, передается строка "SQL-ERROR"
	Коды ошибок см. http://www.postgresql.org/docs/current/static/errcodes-appendix.html
	Во всех случаях, за исключением ошибок соединения с базой-роутером [а именно для кодов "08003" и  "08006"], повторная отсылка данных на сервер бессмысленна.

	*/

	printf("/////***************ERROR***************///////////\r\n");
	time_t timer;
	time(&timer);
	tm * ptm;
	ptm = gmtime(&timer);
	printf("[%04d-%02d-%02d %02d:%02d:%02d]\r\n", ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);

	//const char *strError = objDuplo->soap_fault_string();
	//printf("fault_detail = %s\r\n",objDuplo->soap_fault_detail());
	//printf("fault_string = %s\r\n", strError);
	//printf("\r\n");
	
	objDuplo->soap_stream_fault(std::cout);
	const char* ll = objDuplo->soap_fault_string();
	const char* ll2 = objDuplo->soap_fault_detail();

	//todo
	char err0[256];
	sprintf(err0, "soap_fault_string = %s[%s]\r\n", ll, ll2);
	addErrorLogMessage(err0);

	//soap_get_SOAP_ENV__Fault(objDuplo->soap, fault_describe, error_code, error_string);

	printf("\r\n");

	int numError = 0;
	if (objDuplo->soap_fault())
		if (objDuplo->soap_fault()->SOAP_ENV__Detail)
			if (objDuplo->soap_fault()->SOAP_ENV__Detail->ns1__DuploFault)
				if (objDuplo->soap_fault()->SOAP_ENV__Detail->ns1__DuploFault->faultCode)
				{
					numError = _wtoi(objDuplo->soap_fault()->SOAP_ENV__Detail->ns1__DuploFault->faultCode->c_str());
					printf("!!!!!!-------faultCode = %d\r\n", numError);
				}


	//if (objDuplo->soap_fault())
	//	if (objDuplo->soap_fault()->SOAP_ENV__Detail)
	//		if (objDuplo->soap_fault()->SOAP_ENV__Detail->ns1__DuploFault)
	//			if (objDuplo->soap_fault()->SOAP_ENV__Detail->ns1__DuploFault->faultMessage)
	//			{
	//				printf("!!!!!!-------faultMessage = %s\r\n", utf8_encode(*objDuplo->soap_fault()->SOAP_ENV__Detail->ns1__DuploFault->faultMessage).c_str());
	//			}

	int nCode = 0;
	if (objDuplo->soap_fault())
		if (objDuplo->soap_fault()->faultcode)
		{
			char err[MAX_PATH_ERR_STRING_LOG];
			sprintf(err, "faultcode = %s", objDuplo->soap_fault()->faultcode);
			addErrorLogMessage(err);

			nCode = atoi(objDuplo->soap_fault()->faultcode);
			printf("!!!!!!-------code = %d\r\n", nCode);
		}



	if (nCode != 0)
	{
		if (objDuplo->soap_fault())
			if (objDuplo->soap_fault()->faultstring)
			{
				printf("!!!!!!-------faultstring = %s\r\n", objDuplo->soap_fault()->faultstring);
				char err[MAX_PATH_ERR_STRING_LOG];
				sprintf(err, "faultcode_string = %s", objDuplo->soap_fault()->faultstring);
				addErrorLogMessage(err);
			}
	}

	//todo
	char err[256];
	sprintf(err, "numError = %d nCode = %d", numError, nCode);
	addErrorLogMessage(err);

	if (fault_describe->faultstring != nullptr)
	{
		if (strlen(fault_describe->faultstring) > 0)
		{
			sprintf(err, "faultstring = %s", fault_describe->faultstring);
			addErrorLogMessage(err);
		}
	}

	
	int server_error = 0;
	if (fault_describe->faultcode != nullptr)
	{
		server_error = atoi(fault_describe->faultcode);
		if (server_error != 0)
		{
			sprintf(err, "faultcode = %s", fault_describe->faultcode);
			addErrorLogMessage(err);
		}
	}

	//char err[256];
	//sprintf(err, "errnCode = %d", nCode);
	//addErrorLogMessage(err);

	printf("/////***************ERROR***************///////////\r\n");

	if (server_error != 0)
		return server_error;

	////!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//проверить!!
	if (numError != 0)
		return numError;

	if (nCode != 0)
		return nCode;

	return 0;
}
int COLSO::ping_alive()
{
	getVersion(objDuplo);

	return checkFault();
}
MessengerServiceSoapBindingProxy *COLSO::connectToServ(char *ip)
{
	objDuplo = new MessengerServiceSoapBindingProxy(ip, SOAP_IO_KEEPALIVE /*| SOAP_IO_CHUNK*/);


	/*
	set.accept_timeout = 60;//30
	set.connect_timeout = 60;//30
	set.recv_timeout = 60;//60
	set.send_timeout = 120;//60
	set.transfer_timeout = 120;//60
*/

	objDuplo->soap->accept_timeout = set.accept_timeout;//30
	objDuplo->soap->connect_timeout = set.connect_timeout;//30
	objDuplo->soap->recv_timeout = set.recv_timeout;//60
	objDuplo->soap->send_timeout = set.send_timeout;//60
	objDuplo->soap->transfer_timeout = set.transfer_timeout;//60

	//todo
	char errConnect[256];
	sprintf(errConnect, "connectToServ = 0x%x[buflen = %d][SOAP_BUFLEN = %d][%s][err=%d][errN=%d][errM=%d]", objDuplo, objDuplo->soap->buflen, SOAP_BUFLEN, ip, objDuplo->soap->error, objDuplo->soap->errnum, objDuplo->soap->errmode);
	addErrorLogMessage(errConnect);
	
	int cnnt = 0;
	int ret = SOAP_TCP_ERROR;
	while (ret != SOAP_OK)
	{
		ret = getInfo(objDuplo, &set.camera_id);
		if (ret != SOAP_OK)
		{
			//sprintf(errSql, "getInfo = %I64d[%I64d]", id_last,id);
			addErrorLogMessage("getInfo error");
			sprintf(errConnect, "[err=%d][errN=%d][errM=%d]", objDuplo->soap->error, objDuplo->soap->errnum, objDuplo->soap->errmode);
			addErrorLogMessage(errConnect);

		}
		Sleep(5000);
		cnnt++;
		if (cnnt > 3)break;
	}


	if (getVersion(objDuplo) != 0)
	{
		//sprintf(errSql, "getVersion error", id_last,id);
		addErrorLogMessage("getVersion error");
		sprintf(errConnect, "[err=%d][errN=%d][errM=%d]", objDuplo->soap->error, objDuplo->soap->errnum, objDuplo->soap->errmode);
		addErrorLogMessage(errConnect);

	}

	//todo
	objDuplo->soap->tcp_keep_alive = 1;
	objDuplo->soap->keep_alive = 1;

	return objDuplo;
} 
bool COLSO::fileIdBackupPresent()
{
	if (bFileIdBackupPresent) return true;

	struct _stat buffer;
	int res = 0;
	char zero = '0';

	for (int i = 0; i < MAX_CHAR_ID; i++)
	{
		res = _wstat(path_of_id_backup_file[i], &buffer);
		if (res == -1)
		{
			if (errno == ENOENT)
			{
				FILE *f = _wfopen(path_of_id_backup_file[i], L"w");
				fwrite(&zero, 1, sizeof(char), f);
				fclose(f);
				res = _wstat(path_of_id_backup_file[i], &buffer);
				if (res != 0) break;
			}
		}
	}

	if (res == 0)
	{
		bFileIdBackupPresent = true;
		return true;
	}
	else
	{
		bFileIdBackupPresent = false;
		return false;
	}
}
__int64 COLSO::readIdBackup()
{
	FILE *f = nullptr;
	char aRead[MAX_CHAR_ID] = { 0 };
	char aRead2[MAX_CHAR_ID + 1] = { 0 };
	__int64 read = -1;

	if (fileIdBackupPresent())
	{
		for (int i = 0; i < MAX_CHAR_ID; i++)
		{
			f = _wfopen(path_of_id_backup_file[i], L"rt");
			if (f == nullptr) { bFileIdBackupPresent = false; return 0; }
			size_t size = fread(&(aRead[MAX_CHAR_ID - i - 1]), 1, sizeof(char), f);
			if (size != 1 || aRead[MAX_CHAR_ID - i - 1] > 57 || aRead[MAX_CHAR_ID - i - 1] < 48)
				aRead[MAX_CHAR_ID - i - 1] = '0';
			fclose(f);
		}
	}

	//этот замесь нужен т.к. в конце нет символа '\0'
	//bool zero = false;
	//int cnt = 0;
	//for (int i = 0; i < MAX_CHAR_ID; i++)
	//{
	//	if (aRead[i] != '0' && !zero)
	//		zero = true;
	//	if (zero)
	//	{
	//		aRead2[cnt] = aRead[i];
	//		cnt++;
	//	}
	//}

	memcpy(aRead2, aRead, MAX_CHAR_ID);

	if (strlen(aRead2) != 0)
		read = _atoi64(aRead2);
	else
		read = 0;
	return read;
}
int COLSO::saveIdBackup(__int64 write)
{
	FILE *f = nullptr;
	char writeId[MAX_CHAR_ID] = { 0 };
	char writeIdTemp[MAX_CHAR_ID + 1] = { 0 };
	char writeIdTemp2[MAX_CHAR_ID + 1] = { 0 };

	sprintf(writeIdTemp, "%016I64d", id_backup);
	sprintf(writeIdTemp2, "%016I64d", write);

	if (fileIdBackupPresent())
	{
		for (int i = 0; i < MAX_CHAR_ID; i++)
		{
			if (writeIdTemp2[MAX_CHAR_ID - i - 1] != writeIdTemp[MAX_CHAR_ID - i - 1])
			{
				f = _wfopen(path_of_id_backup_file[i], L"wt");
				if (f == nullptr) { bFileIdBackupPresent = false; return -1; }

				size_t size = 0;
				size = fwrite(&(writeIdTemp2[MAX_CHAR_ID - i - 1]), 1, sizeof(char), f);
				fclose(f);
			}
		}
	}

	id_backup = write;

	return 0;
}
bool COLSO::fileIdTempPresent()
{
	if (bFileIdTempPresent) return true;

	struct _stat buffer;
	int res = _wstat(path_of_id_temp_file, &buffer);
	if (res == -1)
	{
		if (errno == ENOENT)
		{
			FILE *f = _wfopen(path_of_id_temp_file, L"w");
			fclose(f);//_wfopen(path_of_id_temp_file [fileIdPresent]
			res = _wstat(path_of_id_temp_file, &buffer);
		}
	}

	if (res == 0)
	{
		bFileIdTempPresent = true;
		return true;
	}
	else
	{
		bFileIdTempPresent = false;
		return false;
	}
}
__int64 COLSO::readIdTemp()
{
	FILE *f;
	char aRead[MAX_CHAR_ID] = { 0 };
	__int64 read = -1;

	if (fileIdTempPresent())
	{
		f = _wfopen(path_of_id_temp_file, L"rt");
		if (f == nullptr){ bFileIdTempPresent = false; return 0; }
		size_t size = fread(aRead, 1, sizeof(char) * MAX_CHAR_ID, f);
		//printf("size = %d[%c %c %c %c %c %c %c %c]\r\n", size, aRead[0], aRead[1], aRead[2], aRead[3], aRead[4], aRead[5], aRead[6], aRead[7]);
		fclose(f);//_wfopen(path_of_id_temp_file [readId]
	}

	read = _atoi64(aRead);
	return read;
}
int COLSO::saveIdTemp(__int64 write)
{
	FILE *f;
	char writeId[MAX_CHAR_ID] = { 0 };//todo
	_i64toa(write, writeId, 10);

	if (fileIdTempPresent())
	{
		f = _wfopen(path_of_id_temp_file, L"wt");
		if (f == nullptr){ bFileIdTempPresent = false; return -1; }
		size_t size = fwrite(writeId, 1, strlen(writeId), f);
		fclose(f);//_wfopen(path_of_id_temp_file [saveId]
	}

	return 0;
}
bool COLSO::fileIdPresent()
{
	if (bFileIdPresent) return true;

	struct _stat buffer;
	int res = _wstat(path_of_id_file, &buffer);
	if (res == -1)
	{
		if (errno == ENOENT)
		{
			FILE *f = _wfopen(path_of_id_file, L"w");
			fclose(f);//_wfopen(path_of_id_file [fileIdPresent]
			res = _wstat(path_of_id_file, &buffer);
		}
	}

	if (res == 0)
	{
		bFileIdPresent = true;
		return true;
	}
	else
	{
		bFileIdPresent = false;
		return false;
	}
}
__int64 COLSO::readId()
{
	FILE *f;
	char aRead[MAX_CHAR_ID] = { 0 };
	__int64 read = -1;

	if (fileIdPresent())
	{
		f = _wfopen(path_of_id_file, L"rt");
		if (f == nullptr){ bFileIdPresent = false; return 0; }
		size_t size = fread(aRead, 1, sizeof(char) * MAX_CHAR_ID, f);
		//printf("size = %d[%c %c %c %c %c %c %c %c]\r\n", size, aRead[0], aRead[1], aRead[2], aRead[3], aRead[4], aRead[5], aRead[6], aRead[7]);
		fclose(f);//_wfopen(path_of_id_file [readId]
	}

	read = _atoi64(aRead);
	return read;
}
int COLSO::saveId(__int64 write)
{
	FILE *f;
	char writeId[MAX_CHAR_ID] = { 0 };//todo
	_i64toa(write, writeId, 10);

	if (fileIdPresent())
	{
		f = _wfopen(path_of_id_file, L"wt");
		if (f == nullptr){ bFileIdPresent = false; return -1; }
		size_t size = fwrite(writeId, 1, strlen(writeId), f);
		fclose(f);//_wfopen(path_of_id_file [saveId]
	}

	return 0;
}
int COLSO::dbConnect()
{
	//пароль под root под политех
#ifdef POLITEH_STLE
	char* root_password = "0940D56E2FB226A7F8EB8C736E6B7F93";//root
#endif

	//стандартный скатовский пароль
#ifndef POLITEH_STLE
	char* root_password = "2D31BD17E960BB60DAFA84A2A7B46DDB";//root
#endif
	//char* root_password = "ac87301fb4d6f120b3debe06c8979bc0";//prometheus

	mysql = mysql_init(NULL); if (mysql == nullptr) return false;
	//if (mysql_real_connect(mysql, "7.199.88.244", "prometheus", root_password, nullptr, 3306, NULL, 0) != nullptr)
	if (mysql_real_connect(mysql, "127.0.0.1", "root", root_password, nullptr, 3306, NULL, 0) != nullptr)
	{
		printf("dbConnect good\r\n");
		mysql_set_character_set(mysql, "utf8");
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
int COLSO::dbQueryGetPath(__int64 id, __int64 max_id)
{
	//file_type
	char query[300];

//#ifndef POLITEH_STLE
	if ((max_id - id) >= MAX_COUNT_SQL)
	{
		//здесь берем по айдишнику, значения
		sprintf(query, "SELECT `id`,`file_path` FROM `main`.`materials` WHERE `event_id` >= %I64d AND `event_id` < %I64d AND `file_type` = 1 ORDER BY `id` DESC LIMIT %d", id, id + MAX_COUNT_SQL, MAX_COUNT_SQL);
	}
	else
	{
		//sprintf(query, "SELECT `id`,`file_path` FROM `main`.`materials` WHERE `event_id` >= %I64d AND `event_id` <= %I64d AND `file_type` = 1 ORDER BY `id` DESC LIMIT %d", id, max_id, MAX_COUNT_SQL);
		if(id != max_id)
			sprintf(query, "SELECT `id`,`file_path` FROM `main`.`materials` WHERE `event_id` >= %I64d AND `event_id` <= %I64d AND `file_type` = 1 ORDER BY `id` DESC LIMIT %d", id, max_id, max_id - id + 1);
		else
			sprintf(query, "SELECT `id`,`file_path` FROM `main`.`materials` WHERE `event_id` = %I64d AND `file_type` = 1 ORDER BY `id` DESC LIMIT 1", id);
	}
//#endif

//из-за того, что возможна ситуация, когда формирование обзорки может быть отключено, то завязываемся на вырезку
//#ifdef POLITEH_STLE
//	if ((max_id - id) >= MAX_COUNT_SQL)
//	{
//		//здесь берем по айдишнику, значения
//		sprintf(query, "SELECT `id`,`file_path` FROM `main`.`materials` WHERE `event_id` >= %I64d AND `event_id` < %I64d AND `file_type` = 2 ORDER BY `id` DESC LIMIT %d", id, id + MAX_COUNT_SQL, MAX_COUNT_SQL);
//	}
//	else
//	{
//		//sprintf(query, "SELECT `id`,`file_path` FROM `main`.`materials` WHERE `event_id` >= %I64d AND `event_id` <= %I64d AND `file_type` = 1 ORDER BY `id` DESC LIMIT %d", id, max_id, MAX_COUNT_SQL);
//		sprintf(query, "SELECT `id`,`file_path` FROM `main`.`materials` WHERE `event_id` >= %I64d AND `event_id` <= %I64d AND `file_type` = 2 ORDER BY `id` DESC LIMIT %d", id, max_id, max_id - id + 1);
//	}
//#endif
	//addErrorLogMessage(query);

	try
	{
		if (mysql_query(mysql, query))
		{
			errMySqlShutdowner();
			printf("%s\n", mysql_error(mysql));
			return -1;
		}

		if (!(g_sqlRes = mysql_store_result(mysql)))
		{
			errMySqlShutdowner();
			printf("%s\n", mysql_error(mysql));
			return -2;
		}
	}
	catch (...)
	{
		errMySqlShutdowner();
		printf("%s\n", mysql_error(mysql));
		return -3;
	}

	if (mysql_num_rows(g_sqlRes) == 0)
	{
		printf("%s\n", mysql_error(mysql));

		return -4;
	}

	int cnt = 0;
	char file_path[MAX_PATH];

//#ifdef DEBUG_PROC
//	char errSql2[256];
//	sprintf(errSql2, "sqlRes->row_count = %d", g_sqlRes->row_count);
//	addErrorLogMessage(errSql2);
//#endif

	//todo
	char errSql3[256];
	int nError = 0;

	int row_count = g_sqlRes->row_count;

	while ((cnt < row_count * 2) && bRunning)
	{
		MYSQL_ROW row = mysql_fetch_row(g_sqlRes);
		
		if (strcmp(tempWpath, row[1]) != 0)
			strcpy(tempWpath, row[1]);
		else
		{
			nError++;
			sprintf(errSql3, "row[1] = %s[nError = %d]", row[1], nError);
			addErrorLogMessage(errSql3);
			row_count = row_count - 2;
			//memset(tempWpath, 0, MAX_PATH);
			if (nError > 3)break;
			continue;
		}

		nError = 0;

		memset(fr_wpath,0, MAX_PATH);
		strncpy(fr_wpath,tempWpath,47);
		strcat(fr_wpath,"_fr.jpg");

		dbRes[cnt].id = atoi(row[0]);
		wcscpy(dbRes[cnt].wpath, g_pathMain);
		wcscpy(dbRes[cnt+1].wpath, g_pathMain);

//		std::wstring logBd;

		//запись обзорки
		mbstowcs(dbRes[cnt].wpath + wcslen(g_pathMain), tempWpath, MAX_PATH);
		//logBd = dbRes[cnt].wpath;
		//logBd += L" N =  ";
		//logBd += std::to_wstring(cnt);

		//запись вырезки
		cnt++;
		mbstowcs(dbRes[cnt].wpath + wcslen(g_pathMain), fr_wpath, MAX_PATH);

		//logBd += L"\r\n";
		//logBd += dbRes[cnt].wpath;
		//logBd += L" size = ";
		//logBd += std::to_wstring(row_count);
		//logBd += L"\r\n";

		//FILE *f = _wfopen(L"c:/soap/bd.log", L"at, ccs=UTF-16LE");
		//size_t size = fwrite(logBd.c_str(), sizeof(wchar_t), logBd.size(), f);
		//fclose(f);

		const char* perno = mysql_error(mysql);
		int erno = mysql_errno(mysql);

		if (strlen(perno) != 0 || erno != 0)
		{
			addErrorLogMessage((char*)perno);

			char errSql[256];
			sprintf(errSql, "errno[%d]", erno);
			addErrorLogMessage(errSql,true);
		}
		
		cnt++;
	}

	mysql_free_result(g_sqlRes);
	g_sqlRes = nullptr;

	errMySqlZero();
	return cnt;
}
__int64 COLSO::dbQueryMaxId()
{
	char query[300];

	MYSQL_RES *sqlRes;
	//этим азпросом берем последний айдишник события и соответсвенно кадров
	//strcpy(query, "SELECT `event_id` FROM `main`.`materials` WHERE `event_id` = (SELECT max(event_id) FROM `main`.`materials`)");
	//strcpy(query, "SELECT max(event_id) FROM `main`.`materials`");
	strcpy(query, "SELECT `main`.`events`.`id` FROM `main`.`events` ORDER BY `main`.`events`.`id` DESC LIMIT 1");
	

	try
	{
		if (mysql_query(mysql, query))
		{
			errMySqlShutdowner();
			printf("%s\n", mysql_error(mysql));
			addErrorLogMessage((char*)(mysql_error(mysql)));
			return -1;
		}

		if (!(sqlRes = mysql_store_result(mysql)))
		{
			errMySqlShutdowner();
			printf("%s\n", mysql_error(mysql));
			addErrorLogMessage((char*)(mysql_error(mysql)));
			return -2;
		}
	}
	catch (...)
	{
		errMySqlShutdowner();
		printf("%s\n", mysql_error(mysql));
		addErrorLogMessage((char*)(mysql_error(mysql)));
		return -3;
	}

	if (mysql_num_rows(sqlRes) == 0)
	{
		printf("%s\n", mysql_error(mysql));
		addErrorLogMessage((char*)(mysql_error(mysql)));
		return -4;
	}

	MYSQL_ROW row = mysql_fetch_row(sqlRes);
	__int64 id = atoi(row[0]);

	mysql_free_result(sqlRes);

	errMySqlZero();
	return id;
}
__int64 COLSO::dbQueryStartIdByDate(std::string strDate)
{
	// target_id
	// SELECT `id` FROM `main`.`targets` WHERE `detection_time` > '2018-01-01 00:00:00.000000' LIMIT 1

	// id from target id
	// SELECT `id` FROM `main`.`materials` WHERE `target_id` = target_id
	
	// SELECT `event_id`,`file_path` FROM `main`.`materials` WHERE `target_id` = (SELECT `id` FROM `main`.`targets` WHERE `detection_time` > '2018-03-20 18:00:00.000000' LIMIT 1) AND `file_type` = 1
	//2018-03-20 18:00:00.000000
	char query[300];

	//здесь берем по айдишнику, значения
	sprintf(query, "SELECT `event_id`,`file_path` FROM `main`.`materials` WHERE `target_id` = (SELECT `id` FROM `main`.`targets` WHERE `detection_time` > '%s' LIMIT 1) AND `file_type` = 1", strDate.c_str());

	try
	{
		if (mysql_query(mysql, query))
		{
			errMySqlShutdowner();
			printf("%s\n", mysql_error(mysql));
			return -1;
		}

		if (!(g_sqlRes = mysql_store_result(mysql)))
		{
			errMySqlShutdowner();
			printf("%s\n", mysql_error(mysql));
			return -2;
		}
	}
	catch (...)
	{
		errMySqlShutdowner();
		printf("%s\n", mysql_error(mysql));
		return -3;
	}

	if (mysql_num_rows(g_sqlRes) == 0)
	{
		printf("%s\n", mysql_error(mysql));
		return -4;
	}

	int cnt = 0;
	//char file_path[MAX_PATH];

//#ifdef DEBUG_PROC
//	char errSql2[256];
//	sprintf(errSql2, "sqlRes->row_count = %d", g_sqlRes->row_count);
//	addErrorLogMessage(errSql2);
//#endif

	char tmpPath[MAX_PATH] = {0};
	__int64 dateId = 0;

	char errSql3[MAX_PATH_ERR_STRING_LOG];
	int nError = 0;

	int row_count = g_sqlRes->row_count;

	while ((cnt < row_count) && bRunning)
	{
		MYSQL_ROW row = mysql_fetch_row(g_sqlRes);

		//if (strcmp(tempWpath, row[1]) != 0)
		//	strcpy(tempWpath, row[1]);
		//else
		//{
		//	nError++;
		//	sprintf(errSql3, "row[1] = %s[nError = %d]", row[1], nError);
		//	addErrorLogMessage(errSql3);
		//	row_count--;
		//	//memset(tempWpath, 0, MAX_PATH);
		//	if (nError > 3)break;
		//	continue;
		//}

		nError = 0;

		dateId = atoi(row[0]);
		strcpy(tmpPath, row[1]);

		//todo
		char errSql2[MAX_PATH_ERR_STRING_LOG];
		sprintf(errSql2, "dateId = %I64d[%s]", dateId, tmpPath);
		addErrorLogMessage(errSql2);

		const char* perno = mysql_error(mysql);
		int erno = mysql_errno(mysql);

		if (strlen(perno) != 0 || erno != 0)
		{
			addErrorLogMessage((char*)perno);

			//todo
			char errSql[256];
			sprintf(errSql, "errno[%d]", erno);
			addErrorLogMessage(errSql, true);
		}

		cnt++;
	}

	errMySqlZero();
	mysql_free_result(g_sqlRes);
	g_sqlRes = nullptr;

	return dateId;
 }
__int64 COLSO::dbQueryMinIdByDate(std::string *strFirstFile)
{
	char path[1024] = { 0 };

	int year = 0;
	int month = 0;
	int day = 0;
	bool exit = false;

	for (year = 2018; year < 2100 && !exit; year++)
	{
		for (month = 1; month <= 12 && !exit; month++)
		{
			for (day = 1; day <= 31 && !exit; day++)
			{
				sprintf(path, "dir d:\\arch\\all_fixations\\%04d\\%02d\\%02d", year, month, day);
					if (system(path) == 0)
				{
					exit = true;
					break;
				}
			}
			if (exit)break;
		}
		if (exit)break;
	}

	wchar_t dateStr[1024] = { 0 };
	_swprintf(dateStr, L"D:\\arch\\all_fixations\\%04d\\%02d\\%02d\\*", year, month, day);

	WIN32_FIND_DATA findFileData;
	std::wstring strPath;
	strPath.append(dateStr);

	addErrorLogMessage((char*)utf8_encode(strPath).c_str());

	HANDLE hFind = FindFirstFile(strPath.c_str(), &findFileData);
	std::wstring wstrFile;

	do
	{
		wstrFile = findFileData.cFileName;
	
		if (wstrFile.find(L"jpg") > 0 && wstrFile.find(L"_fr.jpg") == -1 && wstrFile.find(L"sign") == -1 && wstrFile.find(L"..") == -1 && wstrFile.size() > 30)
			break;

	} while (FindNextFile(hFind, &findFileData) != 0);

	*strFirstFile = utf8_encode(wstrFile).c_str();

	FindClose(hFind);

	char query[300];

	MYSQL_RES *sqlRes;
	// этим азпросом берем последний айдишник события и соответсвенно кадров
	// SELECT `event_id` FROM `main`.`materials` WHERE `timestamp` > '2018-01-01 00:00:00.000000' LIMIT 1
	// sprintf(query, "SELECT `id`,`file_path` FROM `main`.`materials` WHERE `event_id` >= %I64d AND `event_id` < %I64d ORDER BY `id`", id, id + MAX_COUNT_SQL);
	// SELECT `event_id`, `id`,`file_path`,`timestamp`	FROM `main`.`materials`	WHERE `file_path` LIKE '2018/03/13%' AND `file_type` = 1 ORDER BY `id` LIMIT 1
	//SELECT `event_id`,`file_path` FROM `main`.`materials` WHERE `file_path` LIKE '2018/03/14/1506018_2018_03_14_09_57_07_53_00_00.jpg'

	sprintf(query, "SELECT `event_id` FROM `main`.`materials` WHERE `file_path` LIKE '%04d/%02d/%02d/%s'", year, month, day, (*strFirstFile).c_str());

	//char err_debug[MAX_PATH_ERR_STRING_LOG];
	//memset(err_debug, 0, MAX_PATH_ERR_STRING_LOG);
	//sprintf(err_debug, "DEBUG:[sql = %s]", query);
	//addErrorLogMessage(err_debug);

	try
	{
		if (mysql_query(mysql, query))
		{
			errMySqlShutdowner();
			printf("%s\n", mysql_error(mysql));
			return -1;
		}

		if (!(sqlRes = mysql_store_result(mysql)))
		{
			errMySqlShutdowner();
			printf("%s\n", mysql_error(mysql));
			return -2;
		}
	}
	catch (...)
	{
		errMySqlShutdowner();
		printf("%s\n", mysql_error(mysql));
		return -3;
	}

	if (mysql_num_rows(sqlRes) == 0)
	{
		printf("%s\n", mysql_error(mysql));
		return -4;
	}

	MYSQL_ROW row = mysql_fetch_row(sqlRes);
	__int64 id = atoi(row[0]);

	errMySqlZero();
	mysql_free_result(sqlRes);

	return id;
}
__int64 COLSO::dbQueryMinId()
{
	char query[300];

	MYSQL_RES *sqlRes;
	//этим азпросом берем последний айдишник события и соответсвенно кадров
	//strcpy(query, "SELECT `event_id` FROM `main`.`materials` WHERE `event_id` = (SELECT min(event_id) FROM `main`.`materials`)");

	strcpy(query, "SELECT `main`.`events`.`id` FROM `main`.`events` ORDER BY `main`.`events`.`id` ASC LIMIT 1");

	

	try
	{
		if (mysql_query(mysql, query))
		{
			errMySqlShutdowner();
			printf("%s\n", mysql_error(mysql));
			return -1;
		}

		if (!(sqlRes = mysql_store_result(mysql)))
		{
			errMySqlShutdowner();
			printf("%s\n", mysql_error(mysql));
			return -2;
		}
	}
	catch (...)
	{
		errMySqlShutdowner();
		printf("%s\n", mysql_error(mysql));
		return -3;
	}

	if (mysql_num_rows(sqlRes) == 0)
	{
		printf("%s\n", mysql_error(mysql));
		return -4;
	}

	MYSQL_ROW row = mysql_fetch_row(sqlRes);
	__int64 id = atoi(row[0]);

	errMySqlZero();
	mysql_free_result(sqlRes);

	return id;
}
int COLSO::dbReconnect()
{
	char* root_password = "2D31BD17E960BB60DAFA84A2A7B46DDB";

	if (mysql_real_connect(mysql, "127.0.0.1", "root", root_password, nullptr, 3306, NULL, 0) == nullptr)
	{
		return -1;
	}
	mysql_set_character_set(mysql, "utf8");
	return 0;
}
int COLSO::fillData(wchar_t* path, wchar_t *path_fr)
{
	//эту функцию прямо здесь и развернуть
	// jpg_grz
	// jpeg_carCrop
	// jpeg_observe0
	// jpeg_observe1
	// skat_data.info

	int res = parse(path, path_fr);
	if (res != 0)
		printf("parse error!!\r\n");
	else
		printf("parse ok!!\r\n");

	if (res == -2)
	{
		
		if ((id_last - id) < 5)
		{
			addErrorLogMessage("errFillData: try parse... wait save file...");
			Sleep(30000);//задержка при повтороном парсинге, вдруг не успело скопироваться все
		}

		res = parse(path, path_fr);

		if (res != 0)
		{
			printf("repeat parse error!!\r\n");
			
			if ((id_last - id) < 5)
			{
				addErrorLogMessage("errFillData: try parse... wait save file...");
				Sleep(30000);//задержка при повтороном парсинге, вдруг не успело скопироваться все
			}
			res = parse(path, path_fr);
			char tess[MAX_PATH_ERR_STRING_LOG];
			sprintf(tess, "\r\nopen file = %s\r\nfile_fr = %s", utf8_encode(path).c_str(), utf8_encode(path_fr).c_str());
			addErrorLogMessage(tess);
		}

		if (res == 0)
			printf("repeat parse ok!!\r\n");
		else
			printf("parse error!!\r\n");
	}


	return res;
}
int COLSO::parse(wchar_t* path, wchar_t *path_fr)
{
	addErrorLogMessage("parse...");
	char tess[MAX_PATH_ERR_STRING_LOG];
	sprintf(tess, "\r\nopen file = %s\r\nfile_fr = %s", utf8_encode(path).c_str(), utf8_encode(path_fr).c_str());
	addErrorLogMessage(tess);

	skat_data = { 0 };

	FILE *f_main = nullptr;
	try
	{
		errno_t errOpen = _wfopen_s(&f_main, path, L"rb");
		if (errOpen != 0)
		{
			//todo
			char eeeeOPen[MAX_PATH_ERR_STRING_LOG];
			sprintf(eeeeOPen, "[parse_error]errno open file = %s[%d-%d] 0x%x", utf8_encode(path).c_str(), errno, errOpen, f_main);
			addErrorLogMessage(eeeeOPen);
		}
	}
	catch (...)
	{
		DWORD hh = GetLastError();
		char eeee[MAX_PATH_ERR_STRING_LOG];
		_i64toa(hh, eeee, 10);
		addErrorLogMessage(eeee, true);
	}

	if (f_main == 0)
	{
		printf("[parse_error][path]_wfopen error\r\n");
		return -1;
	}

	struct _stat buffer;
	_wstat(path, &buffer);

	memset(jpeg_main, 0, MAX_SIZE_MAIN_BMP);
	int readFromFile = 0;

	if (buffer.st_size < MAX_SIZE_MAIN_BMP)
		readFromFile = fread((void*)jpeg_main, sizeof(char), buffer.st_size, f_main);
	else
	{
		printf("[parse_error][path]buffer.st_size < MAX_SIZE_MAIN_BMP[%d]\r\n", buffer.st_size);
		fclose(f_main);//_wfopen_s(&f_main [parse]
		return -1;
	}

	if(jpeg_main[0] == 0)
	{
		addErrorLogMessage("[parse_error]jpeg_main[0] == 0");
		fclose(f_main);//_wfopen_s(&f_main [parse]
		return -1;
	}


	if (buffer.st_size != readFromFile)
	{
		printf("[parse_error][path]buffer.st_size != readFromFile[%d != %d]\r\n", buffer.st_size, readFromFile);
		fclose(f_main);//_wfopen_s(&f_main [parse]
		return -1;
	}

	fclose(f_main);//_wfopen_s(&f_main [parse]
	f_main = nullptr;

	//ищем конец доп кадров
	if (jpeg_main[readFromFile - sizeof(fileInfo) - 1] != 0x96 && jpeg_main[readFromFile - sizeof(fileInfo) - 2] != 0xa5)
	{
		printf("ends of binary data not found\r\n");
		addErrorLogMessage("[parse_error]ERROR:ends of binary data not found!!!");
		//if (modeRegion != mode::ULYANOVSK)
		//return -2;

		// если москва, и это нарушение, то проверяем на наличие доп кадров
		// если москва и просто проезд, то проверять не надо
		//if (modeRegion == mode::MOSKVA)
		//	if (skat_data.info.VIOLA_REASON != 0)

		return -2;
	}

	unsigned long res_size_Maincar = readFromFile;

	//определяем общее кол-во структур
	int nAddonNumber = 0;
	fileAddInfo tempAddon;
	memcpy(&tempAddon, jpeg_main + readFromFile - sizeof(fileInfo) - 2 - sizeof(fileAddInfo), sizeof(fileAddInfo));
	
	//высчитываем индексы положения кадров джпег
	int cnt = 0;
	unsigned int markerOffset[MAX_MARK_JPEG] = { 0 };
	int cnt0 = 0;
	bool checkAddOns = true;

	//ищем концы джпегов
	while ((cnt < readFromFile) && bRunning)
	{
		if (jpeg_main[cnt] == 0xff && jpeg_main[cnt + 1] == 0xd9)
		{
			//если нет доп кадров
			if (checkAddOns)
			{
				checkAddOns = false;
				if (jpeg_main[cnt + 2] != 0xff && jpeg_main[cnt + 3] != 0xd8)
					break;
			}

			markerOffset[cnt0] = cnt + 2;

			if ((tempAddon.queueNum + 1) == cnt0) 
				break;

			//printf("markerOffset = %d\r\n", markerOffset[cnt0]);
			//чтобы не обнаружить конец джпега в середине структуры

			cnt += sizeof(fileAddInfo) + 2;

			if (jpeg_main[cnt] == 0xa5 && jpeg_main[cnt + 1] == 0x96) 
				break;

			cnt0++;
			if (cnt0 >= MAX_MARK_JPEG)
			{
				cnt0--;
				break;
			}
		}
		cnt++;
	}

	if (modeRegion != mode::STAVROPOL)
		if (modeRegion != mode::BURYATIYA)
		{
			if (markerOffset[0] != 0)
				res_size_Maincar = markerOffset[0];
			else
				res_size_Maincar = readFromFile - sizeof(fileInfo);//TODO здесь еще 2 байта надо вычесть !!!!
		}

	skat_data.nAddonFrames = cnt0;// -1;

	if (skat_data.nAddonFrames > 10)
	{
		char err[MAX_PATH_ERR_STRING_LOG];
		sprintf(err, "[parse_error]skat_data.nAddonFrames = %d", skat_data.nAddonFrames);
		addErrorLogMessage(err);
	}

	memset(jpeg_observe0, 0, MAX_SIZE_MAIN_BMP);
	memset(jpeg_observe1, 0, MAX_SIZE_MAIN_BMP);
	memset(jpeg_observe2, 0, MAX_SIZE_MAIN_BMP);

	//main frame
	//memcpy(,&bmp_main,markerOffset[0]);
	memcpy((void*)(stInfoSkat.infoRaw), jpeg_main + readFromFile - sizeof(fileInfo), sizeof(fileInfo));
	if ((stInfoSkat.info.empty12[0] != 0xa5) && (stInfoSkat.info.empty12[0] != 0x96))
		return -2;

	memcpy(&skat_data.info, &stInfoSkat.info, sizeof(fileInfo));

	//временное хранение бинарной подписи от обзорки
	memcpy(_testinfoRaw, stInfoSkat.infoRaw,1133);


	if (modeRegion != mode::BURYATIYA)
		if(modeRegion != mode::STAVROPOL)
		{
			if (skat_data.nAddonFrames < 2)
			{
				printf("[parse_error]skat_data.nAddonFrames < 2\r\n");
				addErrorLogMessage("[parse_error]skat_data.nAddonFrames < 2");
				//if (modeRegion != mode::ULYANOVSK)
				//	return -2;

				// если москва, и это нарушение, то проверяем на наличие доп кадров
				// если москва и просто проезд, то проверять не надо
				if (modeRegion == mode::MOSKVA)
					if (skat_data.info.VIOLA_REASON != 0)
						return -2;
			}
		}

	//метсоположение
	std::string strPlace;
	skat_data.info.PLACE_NAME[200] = 0;
	strPlace = (char*)skat_data.info.PLACE_NAME;

	if (modeRegion == mode::ULYANOVSK)
	{
		strPlace += ", ";
		strPlace += (char*)skat_data.info.PLACE_DIRECTION;
		strPlace.erase(std::remove(strPlace.begin(), strPlace.end(), '\n'), strPlace.end());
	}

	violaData.pViolaCounterOneSecond = 0;
	std::wstring counterOne(path);
	std::wstring res = counterOne.substr(counterOne.size() - 9,2);
	violaData.pViolaCounterOneSecond = std::stoi(res);


	//strPlace.append(2, 0);
	violaData.pViolaPlace = utf8_decode(strPlace);

	//std::wstring pPlace = charToWString(strPlace.c_str());
	//std::wstring pPlace(strPlace.begin(), strPlace.end());

	//направление

	if (skat_data.info.VIOLA_DIRECTION == 1)//попутка
		violaData.pViolaDirection = L"0";

	if (skat_data.info.VIOLA_DIRECTION == 0)//встречка
		violaData.pViolaDirection = L"1";

	//скорость
	std::string strSpeed;
	char chSpeed[5] = { 0 };
	itoa(skat_data.info.VIOLA_SPEED, chSpeed, 10);
	strSpeed = chSpeed;
	//strSpeed.append(2, 0);
	violaData.pViolaSpeed = utf8_decode(strSpeed);
	//std::wstring pSpeed(strSpeed.begin(), strSpeed.end());

	//предел скорости
	violaData.pViolaSpeedThr = skat_data.info.PLACE_SPEED_MIN;

	//ГРЗ
	std::string strGrz;
	strGrz = (char*)skat_data.info.RECOGN_GRZ1;
	//strGrz.append(2, 0);
	violaData.pViolaGrz = utf8_decode(strGrz);
	//std::wstring pGrz(strGrz.begin(), strGrz.end());

	//ВЕРОЯТНОСТЬ
	std::string strGrzAccuracy;
	char chGrzAccur[5] = { 0 };
	itoa(skat_data.info.RECOGN_PROB, chGrzAccur, 10);
	strGrzAccuracy = chGrzAccur;
	//strGrzAccuracy.append(2, 0);
	violaData.pViolaGrzAccuracy = utf8_decode(strGrzAccuracy);
	//std::wstring pGrzAccuracy(strGrzAccuracy.begin(), strGrzAccuracy.end());

	//код страны
	std::string strCodeCountry;
	std::string strMs;
	Type type_zone = Type::none;

	char * pch = strtok((char*)skat_data.info.RESERV_INFO, ";");

	cnt = 0;
	while ((pch != nullptr) && bRunning)
	{
		strCodeCountry = pch;

		if (cnt == 9)//по счету поле тип полосы(зоны)
			type_zone = (Type)atoi(pch);

		if (cnt == 10)//по счету поле
			strMs = pch;
		pch = strtok(nullptr, ";");
		cnt++;
		if (cnt == 14)//по счету поле
			break;
	}

	violaData.pViolaMs = strMs;// atoi(strMs.c_str());
	//std::wstring pCodeCountry(strCodeCountry.begin(), strCodeCountry.end());
	//strCodeCountry.append(2, 0);
	violaData.pViolaCodeCountry = utf8_decode(strCodeCountry);

	//цвет грз
	violaData.pViolaGrzColor = 0;
	/*
	цвета в бинарной подписи(наши обозначения)
	3 – белый,
	4 – черный,
	5 – красный,
	6 – зеленый,
	7 – синий,
	8 – желтый,
	9 – циан,
	10 – оранжевый
	*/

#ifndef POLITEH_STLE
	if (skat_data.info.empty2[0] == 3)
		violaData.pViolaGrzColor = 0;

	if (skat_data.info.empty2[0] == 4)
		violaData.pViolaGrzColor = 4;

	if (skat_data.info.empty2[0] == 7)
		violaData.pViolaGrzColor = 2;

	if (skat_data.info.empty2[0] == 8)
		violaData.pViolaGrzColor = 1;

	if (skat_data.info.empty2[0] == 5)
		violaData.pViolaGrzColor = 3;
#endif

	/*
	"0":  None - 0
	"1.1": OverSpeed - 2
	"3": Side - 54
	"4": BicycleLane - не требуется
	"6": BusLane - 5 (для Московской области)
	"7": WrongDirection - 4
	"11": DoubleLane - не требуется
	"8": Выезд на трамвайные пути встерчного движения - 42 (новое)

	0 – проезд без нарушения,
	1 – превышение порога фиксации,
	3 – движение по обочине,
	4 – все остальные причины (в т.ч. движение в направлении противоположном разрешенному направлению),
	7 – движение по тротуару или велосипедной дорожке,
	8 – движение по полосе общественного
	транспорта,
	11 – движение по разделительной полосе
	*/

	violaData.pViola = 0;

//#ifdef DEBUG_PROC
//	violaData.pViola = 4;
//	skat_data.info.VIOLA_REASON = 7;
//#endif

	if (skat_data.info.VIOLA_REASON == 0) violaData.pViola = 0;//без нарушений+
	if (skat_data.info.VIOLA_REASON == 1) violaData.pViola = 2;//превышение +
	if (skat_data.info.VIOLA_REASON == 3) violaData.pViola = 54;//обочина+
	
	if (skat_data.info.VIOLA_REASON == 7)
		skat_data.info.VIOLA_REASON = 4;


	if (skat_data.info.VIOLA_REASON == 4)
	{
		switch (type_zone)
		{
		case Type::bike:
			violaData.pViola = 55;//По велосипедной дорожке
			break;
		case Type::sidewalk:
			violaData.pViola = 57;//По дорожке
			break;
		default:
			//это из-за косяка в основном софте, на с7 пишется с4. Поэтому такой костыль
			if (skat_data.info.RECOGN_GRZ1[0] == 0 && skat_data.info.RECOGN_GRZ2[0] == 0)
				skat_data.info.VIOLA_REASON = 0;
			else
				skat_data.info.VIOLA_REASON = 7;
			break;
		}
	}


	if (skat_data.info.VIOLA_REASON == 6) violaData.pViola = 5;// движение по полосе маршрутных ТС
	if (skat_data.info.VIOLA_REASON == 7) violaData.pViola = 4;// встречка
	if (skat_data.info.VIOLA_REASON == 8) violaData.pViola = 42;// трамвай
	if (skat_data.info.VIOLA_REASON == 9) violaData.pViola = 67;// встрчка + односторонее
	if (skat_data.info.VIOLA_REASON == 11) violaData.pViola = 85;//разделительная полоса+

	//Грузовые ТС с МРМ более 2,5 т. в левой полосе 107
	if (skat_data.info.VIOLA_REASON == 17) violaData.pViola = 107;// встрчка + односторонее

	//Движение грузовиков далее второго ряда 77
	if (skat_data.info.VIOLA_REASON == 18) violaData.pViola = 77;//разделительная полоса+
	if (skat_data.info.VIOLA_REASON == 80) violaData.pViola = 80;//нарушение разметки - островок
	if (skat_data.info.VIOLA_REASON == 19) violaData.pViola = 19;//нарушение разметки - пересечение сплошной
	if (skat_data.info.VIOLA_REASON == 20) violaData.pViola = 103;//нарушение разметки - ДОРОЖНЫЙ ЗНАК 5.15.1

	if (modeRegion == mode::ULYANOVSK)
	{
		if (skat_data.info.VIOLA_REASON == 15) violaData.pViola = 58;//пешеход
		if (skat_data.info.VIOLA_REASON == 10) violaData.pViola = 118;//задний ход
	}

	if (modeRegion == mode::MOSKVA2)
	{
		if (skat_data.info.VIOLA_REASON == 0) violaData.pViola = 0;//без нарушений+
		if (skat_data.info.VIOLA_REASON == 1) violaData.pViola = 2;//превышение +
		if (skat_data.info.VIOLA_REASON == 3) violaData.pViola = 54;//обочина+
		if (skat_data.info.VIOLA_REASON == 10) violaData.pViola = 118;//разворот
		if (skat_data.info.VIOLA_REASON == 15) violaData.pViola = 58;//пешеход
	}



	//координаты
	std::string strLat;
	std::string strLon;

	char chLat[10] = { 0 };
	char chLon[11] = { 0 };

	if (skat_data.info.empty8[0] != 0)
	{
		//широта 10
		memcpy(chLat, skat_data.info.empty8 + 1, 9);
		strLat = chLat;
		//std::wstring pLat(strLat.begin(), strLat.end());
		//strLat.append(2, 0);
		violaData.pViolaLat = utf8_decode(strLat);

		//долгта 11
		memcpy(chLon, skat_data.info.empty8 + 11, 11);
		strLon = chLon;
		//std::wstring pLon(strLon.begin(), strLon.end());
		//strLon.append(2, 0);
		violaData.pViolaLon = utf8_decode(strLon);
	}
	else
	{
		calcGns(skat_data.info.VIOLA_GNS, chLat, chLon);

		strLat = chLat;
		strLon = chLon;

		//std::wstring pLat(strLat.begin(), strLat.end());
		//strLat.append(2, 0);
		violaData.pViolaLat = utf8_decode(strLat);

		//std::wstring pLon(strLon.begin(), strLon.end());
		//strLon.append(2, 0);
		violaData.pViolaLon = utf8_decode(strLon);
	}


	if (modeRegion != mode::BURYATIYA)
		if (modeRegion != mode::STAVROPOL)
		{
			if (violaData.pViolaLon[0] == '\0' || violaData.pViolaLat[0] == '\0')
			{
				printf("[parse_error]Latitude or Longitude == 0\r\n");
				addErrorLogMessage("[parse_error]Latitude or Longitude == 0");
				return -1;
			}

			int cntDotLon = dotCount(violaData.pViolaLon);
			int cntDotLat = dotCount(violaData.pViolaLat);

			if (cntDotLon > 1 || cntDotLat > 1)
			{
				printf("[parse_error]Latitude or Longitude too many dots\r\n");
				addErrorLogMessage("[parse_error]Latitude or Longitude too many dots");
				return -1;
			}
		}

	//id камеры
	std::string strCameraId;
	strCameraId = (char*)skat_data.info.DEVICE_NUMBER;
	//strCameraId.append(2, 0);
	
	//"complex_type": "p",
	//"complex_type": "s",

	if (modeRegion == mode::STAVROPOL)
		violaData.pViolaCameraId = utf8_decode(strCameraId);

	if (modeRegion == mode::ULYANOVSK)
		violaData.pViolaCameraId = utf8_decode(strCameraId);

	if (modeRegion == mode::BURYATIYA)
		violaData.pViolaCameraId = utf8_decode(strCameraId);

	if (modeRegion == mode::MOSKVA)
	{
		if (set.c_type.compare("p") == 0)
			violaData.pViolaCameraId = utf8_decode(strCameraId);
		else
		{
			if(set.prefix)violaData.pViolaCameraId.append(L"SK");

			violaData.pViolaCameraId.append(utf8_decode(strCameraId)); //utf8_decode(strCameraId);
			addErrorLogMessage("CameraId is");
			addErrorLogMessage((char*)utf8_encode(violaData.pViolaCameraId).c_str(),false);
		}
	}

	if (modeRegion == mode::MOSKVA2)
	{
		if (set.c_type.compare("p") == 0)
			violaData.pViolaCameraId = utf8_decode(strCameraId);
		else
		{
			if (set.prefix)violaData.pViolaCameraId.append(L"SK");

			violaData.pViolaCameraId.append(utf8_decode(strCameraId)); //utf8_decode(strCameraId);
			addErrorLogMessage("CameraId is");
			addErrorLogMessage((char*)utf8_encode(violaData.pViolaCameraId).c_str(), false);
		}
	}

//#ifdef DEBUG_PROC
//	violaData.pViolaCameraId = L"1506018"; //utf8_decode(strCameraId);
//#endif

	//readFromFile - размер главного джпега

	//int sumLenAddJpeg = 0;
	unsigned long res_size_Observe0 = 0;
	unsigned long res_size_Observe1 = 0;
	unsigned long res_size_Observe2 = 0;

	int k = 0;

	for (int i = 0; i < skat_data.nAddonFrames; i++)
	{
		if (i != 0) k = 1;

		//структура доп.кадра
		memcpy(&skat_data.stAddonFrames[i], jpeg_main + markerOffset[i + 1], sizeof(fileAddInfo));
		//память с джпегом
		memcpy(jpeg_main2, jpeg_main + markerOffset[i] + k*sizeof(fileAddInfo), markerOffset[i + 1] - (markerOffset[i] + k*sizeof(fileAddInfo)));
		int len = markerOffset[i + 1] - (markerOffset[i] + k*sizeof(fileAddInfo));
		//sumLenAddJpeg += len;
		if (i == 0)
		{
			memcpy(jpeg_observe0, jpeg_main2, len);
			res_size_Observe0 = len;
		}
		//jpegToBmp(jpeg_main2, jpeg_observe0, len);

		if (i == 1)
		{
			memcpy(jpeg_observe1, jpeg_main2, len);
			res_size_Observe1 = len;
			//break;
		}

		if (i == 2)
		{
			memcpy(jpeg_observe2, jpeg_main2, len);
			res_size_Observe2 = len;
			break;
		}
		//jpegToBmp(jpeg_main2, jpeg_observe1, len);
	}

	
	memset(jpeg_main2, 0, MAX_SIZE_MAIN_BMP);
	//memset(jpeg_main, 0, MAX_SIZE_MAIN_BMP);

	int sizeMainJpeg = markerOffset[0];// readFromFile - (sizeof(fileInfo) + sizeof(fileAddInfo)*skat_data.nAddonFrames + sumLenAddJpeg);
	//printf("sizeMainJpeg = %d\r\n", sizeMainJpeg);

	//вырезка тс
	FILE *f_fr = nullptr;
	f_fr = _wfopen(path_fr, L"rb");
	if (f_fr == 0)
	{
		printf("[parse_error][path_fr]_wfopen error\r\n");
		return -1;
	}

	struct _stat buffer_fr;
	_wstat(path_fr, &buffer_fr);

	if (buffer_fr.st_size < MAX_SIZE_MAIN_BMP)
		readFromFile = fread((void*)jpeg_main2, sizeof(char), buffer_fr.st_size, f_fr);
	else
	{
		printf("[parse_error][path_fr]buffer_fr.st_size < MAX_SIZE_MAIN_BMP[%d]\r\n", buffer_fr.st_size);
		fclose(f_fr);//_wfopen(path_fr [parse]
		return -1;
	}

	if (jpeg_main2[0] == 0)
	{
		addErrorLogMessage("[parse_error]jpeg_main2[0] == 0");
		fclose(f_fr);//_wfopen_s(&f_main [parse]
		return -1;
	}

	if (buffer_fr.st_size != readFromFile)
	{
		printf("[parse_error][path]buffer.st_size != readFromFile[%d != %d]\r\n", buffer_fr.st_size, readFromFile);
		fclose(f_fr);//_wfopen(path_fr [parse]
		return -1;
	}
	fclose(f_fr);//_wfopen(path_fr [parse]

	//считывание бинарных подписей
	if ((modeRegion == mode::BURYATIYA) || (modeRegion == mode::STAVROPOL))
	{
		//здесь jpeg заменить на sign
		wcscpy(path_sign_0, path);
		int s0 = wcslen(path_sign_0);
		path_sign_0[s0 - 3] = 0;
		wcscat(path_sign_0,L"sign");

		wcscpy(path_sign_1, path_fr);
		int s1 = wcslen(path_sign_1);
		path_sign_1[s1 - 3] = 0;
		wcscat(path_sign_1, L"sign");

		//sign0
		FILE *f_sign = nullptr;
		//addErrorLogMessage("parse-7");

		struct _stat buffer_sign;
		_wstat(path_sign_0, &buffer_sign);

		int sizeSign = 0;

		if (buffer_sign.st_size == SIZE_SIGN)
		{
			f_sign = _wfopen(path_sign_0, L"rb");
			if (f_sign == 0)
			{
				addErrorLogMessage("[parse_error][path_sign]_wfopen error\r\n");
				return -1;
			}
			sizeSign = fread((void*)jpeg_sign_0, sizeof(char), SIZE_SIGN, f_sign);
		}
		else
		{
			printf("[parse_error][path_sign]buffer_sign.st_size < SIZE_SIGN[%d]\r\n", buffer_sign.st_size);
			fclose(f_sign);//_wfopen(path_fr [parse]
			return -1;
		}

		//addErrorLogMessage("parse-8");

		if (SIZE_SIGN != sizeSign)
		{
			printf("[parse_error][path_sign]sizeSign != SIZE_SIGN[%d != %d]\r\n", sizeSign, SIZE_SIGN);
			fclose(f_sign);//_wfopen(path_fr [parse]
			return -1;
		}

		fclose(f_sign);
		f_sign = nullptr;

		//sign1
		FILE *f_sign1 = nullptr;

		struct _stat buffer_sign1;
		_wstat(path_sign_1, &buffer_sign1);

		if (buffer_sign1.st_size == SIZE_SIGN)
		{
			f_sign1 = _wfopen(path_sign_1, L"rb");
			if (f_sign1 == 0)
			{
				printf("[parse_error][path_sign1]_wfopen error\r\n");
				return -1;
			}

			sizeSign = fread((void*)jpeg_sign_1, sizeof(char), SIZE_SIGN, f_sign1);
		}
		else
		{
			printf("[parse_error][path_sign1]buffer_sign1.st_size < SIZE_SIGN[%d]\r\n", buffer_sign1.st_size);
			fclose(f_sign1);//_wfopen(path_fr [parse]
			return -1;
		}

		//addErrorLogMessage("parse-11");

		if (SIZE_SIGN != sizeSign)
		{
			printf("[parse_error][path_sign1]sizeSign != SIZE_SIGN[%d != %d]\r\n", sizeSign, SIZE_SIGN);
			fclose(f_sign1);//_wfopen(path_fr [parse]
			return -1;
		}

		fclose(f_sign1);
		f_sign1 = nullptr;
		//addErrorLogMessage("parse-12");
	}

	//теже структуры переписываем данными из кадра с вырезкой
	//чтобы потом взять вырезку ГРЗ
	memcpy((void*)(stInfoSkat.infoRaw), jpeg_main2 + readFromFile - sizeof(fileInfo), sizeof(fileInfo));
	memcpy(&skat_data.info, &stInfoSkat.info, sizeof(fileInfo));

	if ((stInfoSkat.info.empty12[0] != 0xa5) && (stInfoSkat.info.empty12[0] != 0x96))
		return -2;

	violaData.pViolaTime_ms = 0;

	violaData.pViolaTime_ms |= skat_data.info.VIOLA_UTC[0] << (8 * 7);
	violaData.pViolaTime_ms |= skat_data.info.VIOLA_UTC[1] << (8 * 6);
	violaData.pViolaTime_ms |= skat_data.info.VIOLA_UTC[2] << (8 * 5);
	violaData.pViolaTime_ms |= skat_data.info.VIOLA_UTC[3] << (8 * 4);
	violaData.pViolaTime_ms |= skat_data.info.VIOLA_UTC[4] << (8 * 3);
	violaData.pViolaTime_ms |= skat_data.info.VIOLA_UTC[5] << (8 * 2);
	violaData.pViolaTime_ms |= skat_data.info.VIOLA_UTC[6] << (8 * 1);
	violaData.pViolaTime_ms |= skat_data.info.VIOLA_UTC[7] << (8 * 0);

	//unsigned char VIOLA_GMT[2];
	//__int64 rtrtr = 10800000;
	//VIOLA_GMT[0] = ((rtrtr / (100 * 60 * 60)) >> 8 * 1) & 0xff;
	//VIOLA_GMT[1] = ((rtrtr / (100 * 60 * 60)) >> 8 * 0) & 0xff;;

	short int hours10 = 0;//10800000 = +3 часа
	//complex_utc_offset |= skat_data.info.VIOLA_GMT[0] << (8 * 1);
	//complex_utc_offset |= skat_data.info.VIOLA_GMT[1] << (8 * 0);

	hours10 |= skat_data.info.VIOLA_GMT[0] << (8 * 1);
	hours10 |= skat_data.info.VIOLA_GMT[1] << (8 * 0);

	violaData.pViolaTimeOffset = (hours10 * 1000 * 60 * 60) / 10;
	//violaData.pViolaTime_ms += (violaData.pViolaTimeOffset/1000);

	//это изминение только для новой москвы
#ifdef POLITEH_STLE
	if (modeRegion == mode::MOSKVA2)
	{
		violaData.pViolaTime_ms += violaData.pViolaTimeOffset / 1000;
		//char tempTime[256] = { 0 };
		//sprintf(tempTime,"pViolaTime_ms = %I64d", violaData.pViolaTime_ms);
		//addErrorLogMessage(tempTime);
	}
#endif

	unsigned long res_size_grz = 0;
	unsigned long res_size_Extracar = 0;

	// todo!!!
	// нужно вписывать именно размер джепега
	// а не размер файла, нужно вычитать структуру подписи
	// и вычитать возможные допкадры из ВЫРЕЗКИ!!!
	memcpy(jpeg_carExtra, jpeg_main2, readFromFile);
	res_size_Extracar = readFromFile;// - sizeof(fileInfo)

#ifndef POLITEH_STLE

	if (modeRegion != mode::BURYATIYA)
		if (modeRegion != mode::STAVROPOL)
		{
			// вырезка грз
			// jpeg_carCrop -> bmp_carCrop
			int width_extra = decompress(/*"D:\\Project\\SOAP\\frames\\bmp.bmp", */readFromFile);
			if (width_extra == -1)
			{
				printf("[parse_error]width_extra == -1\r\n");
				return -1;
			}

			int x0 = skat_data.info.RECOGN_FRAME[1];
			x0 = x0 << 8;
			x0 |= skat_data.info.RECOGN_FRAME[0];

			int x1 = skat_data.info.RECOGN_FRAME[3];
			x1 = x1 << 8;
			x1 |= skat_data.info.RECOGN_FRAME[2];

			int y0 = skat_data.info.RECOGN_FRAME[5];
			y0 = y0 << 8;
			y0 |= skat_data.info.RECOGN_FRAME[4];

			int y1 = skat_data.info.RECOGN_FRAME[7];
			y1 = y1 << 8;
			y1 |= skat_data.info.RECOGN_FRAME[6];

			int IMAGE_HEIGHT = skat_data.info.IMAGE_HEIGHT[1];
			IMAGE_HEIGHT = IMAGE_HEIGHT << 8;
			IMAGE_HEIGHT |= skat_data.info.IMAGE_HEIGHT[0];

			if (y0 < 0)
			{
				addErrorLogMessage("[parse_error]y0 < 0!\r\n");
				y0 = 0;
			}

			if (y1 > IMAGE_HEIGHT)
			{
				addErrorLogMessage("[parse_error]y1 > IMAGE_HEIGHT!\r\n");
				y1 = IMAGE_HEIGHT;
			}

			//if ((y0 == 0) || (y1 == 0))
			//{
			//	addErrorLogMessage("y0 or y1 is null!\r\n");
			//	return -1;
			//}

			int IMAGE_WIDTH = skat_data.info.IMAGE_WIDTH[1];
			IMAGE_WIDTH = IMAGE_WIDTH << 8;
			IMAGE_WIDTH |= skat_data.info.IMAGE_WIDTH[0];

			if (x0 < 0)
			{
				addErrorLogMessage("[parse_error]x0 < 0!\r\n");
				x0 = 0;
			}

			if (x1 > IMAGE_WIDTH)
			{
				addErrorLogMessage("[parse_error]x1 > IMAGE_WIDTH!\r\n");
				x1 = IMAGE_WIDTH;
			}

			//if ((x0 == 0) || (x1 == 0))
			//{
			//	addErrorLogMessage("x0 or x1 is null !\r\n");
			//	return -1;
			//}

			if ((IMAGE_WIDTH == 0) || (IMAGE_HEIGHT == 0))
			{
				addErrorLogMessage("[parse_error]IMAGE_WIDTH or IMAGE_HEIGHT is null!\r\n");
				return -1;
			}

			int nHeightGrz = y1 - y0;
			int nWidthGrz = 1 + x1 - x0;
			int nCounter = 0;

			//прямой порядок байт
			for (int h = 0; h < nHeightGrz; h++)
			{
				for (int w = 0; w < nWidthGrz; w++)
				{
					bmp_grz[nCounter] = bmp_carExtra[width_extra * (y0 + h) + x0 + w];
					nCounter++;
				}
			}

			jpg_grz[0] = 0;

			//bmp to jpg
			if (nHeightGrz > 0 && nWidthGrz > 0)
				res_size_grz = compress(nWidthGrz, nHeightGrz, jpg_grz, MAX_SIZE_MAIN_BMP, bmp_grz);
		}



	//обратный порядок считывания байт (зеркальный номер по двум осям)
	//for (int h = nHeightGrz; h > 0; h--)
	//{
	//	int kk0 = width_crop * (y0 + h);
	//	for (int w = 0; w < nWidthGrz * 1; w++)
	//	{
	//		int kk1 = x0 + w;
	//		//printf("kk1 = %d[%d]\r\n", kk1, w);

	//		bmp_grz[nCounter] = bmp_carCrop[width_crop * (y0 + h) + x0 + w]; //h*w);
	//		nCounter++;
	//	}
	//}

#endif

#ifdef POLITEH_STLE
	res_size_grz = markerOffset[1] - markerOffset[0];
	memcpy(jpg_grz, jpeg_main + markerOffset[0], res_size_grz);

	//FILE* f_jpeg;
	//fopen_s(&f_jpeg, "D:\\__1\\51\\GG-2.jpg", "wb");
	//fwrite(jpg_grz, 1, res_size_grz, f_jpeg);
	//fclose(f_jpeg);

#endif

	//SaveBMP("D:\\__1\\30\\GG.bmp", nWidthGrz, nHeightGrz, bmp_grz);
	//char pp[MAX_PATH] = {0};
	//sprintf(pp, "c:\\soap\\1\\jpg-%d.jpg", jpgCnt++);
	//jpeg_carExtra res_size_Extracar

	violaData.nullWstring = L"";
	violaData.nullINt = 0;
	violaData.pViolaAzimut = utf8_decode(set.azimut);

	//"complex_type": "p",
	//"complex_type": "s",

#ifdef AZIMUT_ENABLE
	senddata_psd->message->tr_USCOREcheckIn->v_USCOREazimut = &violaData.pViolaAzimut; //&violaData.nullWstring;// &violaData.nullWstring;
#endif
	senddata_psd->message->tr_USCOREcheckIn->v_USCOREcamera = violaData.pViolaCameraId;//камера
	senddata_psd->message->tr_USCOREcheckIn->v_USCOREcamera_USCOREplace = &violaData.pViolaPlace;//расположение камеры
	senddata_psd->message->tr_USCOREcheckIn->v_USCOREdirection = &violaData.pViolaDirection;//null-стоит, 1-встречка,0-попутка
	senddata_psd->message->tr_USCOREcheckIn->v_USCOREgps_USCOREx = &violaData.pViolaLon;
	senddata_psd->message->tr_USCOREcheckIn->v_USCOREgps_USCOREy = &violaData.pViolaLat;
	senddata_psd->message->tr_USCOREcheckIn->v_USCORElane_USCOREnum = &violaData.nullINt;
	senddata_psd->message->tr_USCOREcheckIn->v_USCOREparking_USCOREnum = &violaData.nullINt;

	if (modeRegion != mode::BURYATIYA)
		if (modeRegion != mode::STAVROPOL)
			senddata_psd->message->tr_USCOREcheckIn->v_USCOREparking_USCOREzone = &violaData.nullINt;
	else
		senddata_psd->message->tr_USCOREcheckIn->v_USCOREparking_USCOREzone = &violaData.pViolaCounterOneSecond;

	//grz
	//base64_encode(&base64Str_grz,jpg_grz, res_size_grz);
	//int lenBase64 = base64Str_grz.size();
	//memcpy(__ptrSenddata_Grz64Bin, base64Str_grz.data(), lenBase64);
	//memcpy(__ptrSenddata_Grz64Bin, jpg_grz, res_size_grz);

	printf("res_size_grz = %d\r\n", res_size_grz);

	senddata_psd->message->tr_USCOREcheckIn->v_USCOREphoto_USCOREgrz->__size = 0;

	if (modeRegion != mode::BURYATIYA)
		if (modeRegion != mode::STAVROPOL)
				senddata_psd->message->tr_USCOREcheckIn->v_USCOREphoto_USCOREgrz->__size = res_size_grz;

	//base64Str_main
	//base64_encode(&base64Str_main, jpeg_main, sizeMainJpeg);
	//lenBase64 = base64Str_main.size();
	//memcpy(__ptrSenddata_Ts64Bin, base64Str_main.data(), lenBase64);
	//memcpy(__ptrSenddata_Ts64Bin, jpeg_main, sizeMainJpeg);
	printf("sizeMainJpeg = %d\r\n", sizeMainJpeg);
	senddata_psd->message->tr_USCOREcheckIn->v_USCOREphoto_USCOREts->__size = res_size_Extracar;// sizeMainJpeg;

	//pSenddata_Ts64Bin->__size = sizeMainJpeg;
	//skatToDuplo->v_USCOREphoto_USCOREts = pSenddata_Ts64Bin;
	senddata_psd->message->tr_USCOREcheckIn->v_USCOREpr_USCOREviol.clear();

#ifdef POLITEH_STLE
	if(violaData.pViola != 0)
	senddata_psd->message->tr_USCOREcheckIn->v_USCOREpr_USCOREviol.push_back(violaData.pViola);
#endif

#ifndef POLITEH_STLE
	// из-за дублирования нарушения 80
	if (skat_data.info.VIOLA_REASON == 19)
	{
		senddata_psd->message->tr_USCOREcheckIn->v_USCOREpr_USCOREviol.push_back(pViola80);
	}
	else
		senddata_psd->message->tr_USCOREcheckIn->v_USCOREpr_USCOREviol.push_back(violaData.pViola);

#endif

	//char err_debug[MAX_PATH_ERR_STRING_LOG];
	//memset(err_debug, 0, MAX_PATH_ERR_STRING_LOG);
	//sprintf(err_debug, "DEBUG:[duplo_code = %d][skat-code = %d]", violaData.pViola, skat_data.info.VIOLA_REASON);
	//addErrorLogMessage(err_debug);

	senddata_psd->message->tr_USCOREcheckIn->v_USCORErecognition_USCOREaccuracy = &violaData.pViolaGrzAccuracy;
	
#ifdef POLITEH_STLE
	size_t pos = violaData.pViolaGrz.find_first_of(L"|",0);
	if(pos != std::wstring::npos)
		violaData.pViolaGrz.erase(pos,1);

	senddata_psd->message->tr_USCOREcheckIn->v_USCOREregno = &violaData.pViolaGrz;
#endif

#ifndef POLITEH_STLE
	senddata_psd->message->tr_USCOREcheckIn->v_USCOREregno = &violaData.pViolaGrz;
#endif
	senddata_psd->message->tr_USCOREcheckIn->v_USCOREregno_USCOREcolor_USCOREid = &violaData.pViolaGrzColor;//0-белый,1-желтый,2-синий,3-красный,4-черный
	senddata_psd->message->tr_USCOREcheckIn->v_USCOREregno_USCOREcountry_USCOREid = &violaData.pViolaCodeCountry;
	senddata_psd->message->tr_USCOREcheckIn->v_USCOREspeed = &violaData.pViolaSpeed;
	senddata_psd->message->tr_USCOREcheckIn->v_USCOREspeed_USCORElimit = &violaData.pViolaSpeedThr;
	senddata_psd->message->tr_USCOREcheckIn->v_USCOREtime_USCOREcheck = violaData.pViolaTime_ms;

	//base64Str_carCrop
	//base64_encode(&base64Str_carCrop, jpeg_carCrop, res_size_cropcar);
	//lenBase64 = base64Str_carCrop.size();
	//memcpy(__ptrSenddata_Extra64Bin, base64Str_carCrop.data(), lenBase64);
	//memcpy(__ptrSenddata_Extra64Bin, jpeg_carCrop, res_size_cropcar);
	printf("res_size_extracar = %d\r\n", res_size_Extracar);

	//jpeg_main
	//sizeMainJpeg

	//jpeg_observe0
	//res_size_Observe0

	//jpeg_observe1
	//res_size_Observe1

	senddata_psd->message->photo_USCOREextra.clear();

	if (modeRegion != mode::STAVROPOL)
		if (modeRegion != mode::BURYATIYA)
		{
			// добавление к обзорке цифровой подписи
			memcpy(jpeg_main + res_size_Maincar, _testinfoRaw, sizeof(fileInfo));
			res_size_Maincar += sizeof(fileInfo);
		}
	//**************************************************************************//

	if (modeRegion == mode::MOSKVA2)
	{
		printf("violaData.pViola = %d\r\n", violaData.pViola);
		violaData.typePhotoExtra = L"1";

		//проезд
		if (violaData.pViola == 0)
		{
			//вырезка
			senddata_psd->message->tr_USCOREcheckIn->v_USCOREphoto_USCOREts->__size = res_size_Extracar;
			senddata_psd->message->tr_USCOREcheckIn->v_USCOREphoto_USCOREts->__ptr = jpeg_carExtra;// jpeg_main;
		}

		//превышение скорости
		if (violaData.pViola == 2)
		{
			//вырезка
			senddata_psd->message->tr_USCOREcheckIn->v_USCOREphoto_USCOREts->__size = res_size_Extracar;
			senddata_psd->message->tr_USCOREcheckIn->v_USCOREphoto_USCOREts->__ptr = jpeg_carExtra;// jpeg_main;

			//observe0
			skatToDuploExtra->v_USCOREphoto_USCOREextra->__ptr = jpeg_observe0;
			skatToDuploExtra->v_USCOREphoto_USCOREextra->__size = res_size_Observe0;

			skatToDuploExtra->v_USCOREframe_USCOREdatetime = &violaData.pViolaTime_ms;
			skatToDuploExtra->v_USCOREtype_USCOREphoto = &violaData.typePhotoExtra;

			//observe1
			skatToDuploExtra2->v_USCOREphoto_USCOREextra->__ptr = jpeg_observe1;
			skatToDuploExtra2->v_USCOREphoto_USCOREextra->__size = res_size_Observe1;

			skatToDuploExtra2->v_USCOREframe_USCOREdatetime = &violaData.pViolaTime_ms;
			skatToDuploExtra2->v_USCOREtype_USCOREphoto = &violaData.typePhotoExtra;

			senddata_psd->message->photo_USCOREextra.push_back(skatToDuploExtra);
			senddata_psd->message->photo_USCOREextra.push_back(skatToDuploExtra2);
		}

		//обочина
		if (violaData.pViola == 54)
		{
			//коллаж
			senddata_psd->message->tr_USCOREcheckIn->v_USCOREphoto_USCOREts->__size = res_size_Maincar;
			senddata_psd->message->tr_USCOREcheckIn->v_USCOREphoto_USCOREts->__ptr = jpeg_main;// jpeg_carExtra;// jpeg_main;

			//нарушение
			skatToDuploExtra->v_USCOREphoto_USCOREextra->__ptr = jpeg_carExtra;
			skatToDuploExtra->v_USCOREphoto_USCOREextra->__size = res_size_Extracar;

			skatToDuploExtra->v_USCOREframe_USCOREdatetime = &violaData.pViolaTime_ms;
			skatToDuploExtra->v_USCOREtype_USCOREphoto = &violaData.typePhotoExtra;

			senddata_psd->message->photo_USCOREextra.push_back(skatToDuploExtra);
		}

		//пешеход
		if (violaData.pViola == 58)
		{

			int sizeVideo = getMkvVideo(path);
			//addErrorLogMessage((char*)utf8_encode(std::wstring(path)).c_str());
			//обзорка
			senddata_psd->message->tr_USCOREcheckIn->v_USCOREphoto_USCOREts->__size = res_size_Maincar;
			senddata_psd->message->tr_USCOREcheckIn->v_USCOREphoto_USCOREts->__ptr = jpeg_main;

			if (sizeVideo > 0)
			{
				violaData.typePhotoVideoExtra = L"3";
				addErrorLogMessage("video is ready to send");
				//передача видео
				skatToDuploExtraVideo->v_USCOREphoto_USCOREextra->__ptr = mkv_video;
				skatToDuploExtraVideo->v_USCOREphoto_USCOREextra->__size = sizeVideo;

				skatToDuploExtraVideo->v_USCOREframe_USCOREdatetime = &violaData.pViolaTime_ms;
				skatToDuploExtraVideo->v_USCOREtype_USCOREphoto = &violaData.typePhotoVideoExtra;

				senddata_psd->message->photo_USCOREextra.push_back(skatToDuploExtraVideo);
			}
		}
	}

	if (modeRegion == mode::MOSKVA)
	{
		printf("violaData.pViola = %d\r\n", violaData.pViola);
		violaData.typePhotoExtra = L"1";

		if (violaData.pViola == 0)
		{
			//вырезка
			senddata_psd->message->tr_USCOREcheckIn->v_USCOREphoto_USCOREts->__size = res_size_Extracar;
			senddata_psd->message->tr_USCOREcheckIn->v_USCOREphoto_USCOREts->__ptr = jpeg_carExtra;// jpeg_main;
		}

		if (violaData.pViola == 2)
		{
			//вырезка
			senddata_psd->message->tr_USCOREcheckIn->v_USCOREphoto_USCOREts->__size = res_size_Extracar;
			senddata_psd->message->tr_USCOREcheckIn->v_USCOREphoto_USCOREts->__ptr = jpeg_carExtra;// jpeg_main;

			//observe0
			skatToDuploExtra->v_USCOREphoto_USCOREextra->__ptr = jpeg_observe0;
			skatToDuploExtra->v_USCOREphoto_USCOREextra->__size = res_size_Observe0;

			skatToDuploExtra->v_USCOREframe_USCOREdatetime = &violaData.pViolaTime_ms;
			skatToDuploExtra->v_USCOREtype_USCOREphoto = &violaData.typePhotoExtra;

			//observe1
			skatToDuploExtra2->v_USCOREphoto_USCOREextra->__ptr = jpeg_observe1;
			skatToDuploExtra2->v_USCOREphoto_USCOREextra->__size = res_size_Observe1;

			skatToDuploExtra2->v_USCOREframe_USCOREdatetime = &violaData.pViolaTime_ms;
			skatToDuploExtra2->v_USCOREtype_USCOREphoto = &violaData.typePhotoExtra;

			senddata_psd->message->photo_USCOREextra.push_back(skatToDuploExtra);
			senddata_psd->message->photo_USCOREextra.push_back(skatToDuploExtra2);
		}

		if (violaData.pViola == 54)
		{
			//коллаж
			senddata_psd->message->tr_USCOREcheckIn->v_USCOREphoto_USCOREts->__size = res_size_Maincar;
			senddata_psd->message->tr_USCOREcheckIn->v_USCOREphoto_USCOREts->__ptr = jpeg_main;// jpeg_carExtra;// jpeg_main;

			//нарушение
			skatToDuploExtra->v_USCOREphoto_USCOREextra->__ptr = jpeg_carExtra;
			skatToDuploExtra->v_USCOREphoto_USCOREextra->__size = res_size_Extracar;

			skatToDuploExtra->v_USCOREframe_USCOREdatetime = &violaData.pViolaTime_ms;
			skatToDuploExtra->v_USCOREtype_USCOREphoto = &violaData.typePhotoExtra;

			//observe0
			skatToDuploExtra2->v_USCOREphoto_USCOREextra->__ptr = jpeg_observe0;
			skatToDuploExtra2->v_USCOREphoto_USCOREextra->__size = res_size_Observe0;

			skatToDuploExtra2->v_USCOREframe_USCOREdatetime = &violaData.pViolaTime_ms;
			skatToDuploExtra2->v_USCOREtype_USCOREphoto = &violaData.typePhotoExtra;

			//observe1
			skatToDuploExtra3->v_USCOREphoto_USCOREextra->__ptr = jpeg_observe1;
			skatToDuploExtra3->v_USCOREphoto_USCOREextra->__size = res_size_Observe1;

			skatToDuploExtra3->v_USCOREframe_USCOREdatetime = &violaData.pViolaTime_ms;
			skatToDuploExtra3->v_USCOREtype_USCOREphoto = &violaData.typePhotoExtra;

			senddata_psd->message->photo_USCOREextra.push_back(skatToDuploExtra);
			senddata_psd->message->photo_USCOREextra.push_back(skatToDuploExtra2);
			senddata_psd->message->photo_USCOREextra.push_back(skatToDuploExtra3);
		}

		if (violaData.pViola == 19)
		{
			//нарушение
			senddata_psd->message->tr_USCOREcheckIn->v_USCOREphoto_USCOREts->__size = res_size_Extracar;
			senddata_psd->message->tr_USCOREcheckIn->v_USCOREphoto_USCOREts->__ptr = jpeg_carExtra;// jpeg_carExtra;// jpeg_main;

			//observe0
			skatToDuploExtra->v_USCOREphoto_USCOREextra->__ptr = jpeg_observe0;
			skatToDuploExtra->v_USCOREphoto_USCOREextra->__size = res_size_Observe0;

			skatToDuploExtra->v_USCOREframe_USCOREdatetime = &violaData.pViolaTime_ms;
			skatToDuploExtra->v_USCOREtype_USCOREphoto = &violaData.typePhotoExtra;

			//observe1
			skatToDuploExtra2->v_USCOREphoto_USCOREextra->__ptr = jpeg_observe1;
			skatToDuploExtra2->v_USCOREphoto_USCOREextra->__size = res_size_Observe1;

			skatToDuploExtra2->v_USCOREframe_USCOREdatetime = &violaData.pViolaTime_ms;
			skatToDuploExtra2->v_USCOREtype_USCOREphoto = &violaData.typePhotoExtra;

			//observe2
			skatToDuploExtra3->v_USCOREphoto_USCOREextra->__ptr = jpeg_observe2;
			skatToDuploExtra3->v_USCOREphoto_USCOREextra->__size = res_size_Observe2;

			skatToDuploExtra3->v_USCOREframe_USCOREdatetime = &violaData.pViolaTime_ms;
			skatToDuploExtra3->v_USCOREtype_USCOREphoto = &violaData.typePhotoExtra;

			//коллаж
			skatToDuploExtra4->v_USCOREphoto_USCOREextra->__ptr = jpeg_main;
			skatToDuploExtra4->v_USCOREphoto_USCOREextra->__size = res_size_Maincar;

			skatToDuploExtra4->v_USCOREframe_USCOREdatetime = &violaData.pViolaTime_ms;
			skatToDuploExtra4->v_USCOREtype_USCOREphoto = &violaData.typePhotoExtra;

			senddata_psd->message->photo_USCOREextra.push_back(skatToDuploExtra);
			senddata_psd->message->photo_USCOREextra.push_back(skatToDuploExtra2);
			senddata_psd->message->photo_USCOREextra.push_back(skatToDuploExtra3);
			senddata_psd->message->photo_USCOREextra.push_back(skatToDuploExtra4);
		}

		if (violaData.pViola == 4)
		{
			//нарушение
			senddata_psd->message->tr_USCOREcheckIn->v_USCOREphoto_USCOREts->__size = res_size_Extracar;
			senddata_psd->message->tr_USCOREcheckIn->v_USCOREphoto_USCOREts->__ptr = jpeg_carExtra;// jpeg_carExtra;// jpeg_main;

			//observe0
			skatToDuploExtra->v_USCOREphoto_USCOREextra->__ptr = jpeg_observe0;
			skatToDuploExtra->v_USCOREphoto_USCOREextra->__size = res_size_Observe0;

			skatToDuploExtra->v_USCOREframe_USCOREdatetime = &violaData.pViolaTime_ms;
			skatToDuploExtra->v_USCOREtype_USCOREphoto = &violaData.typePhotoExtra;

			//observe1
			skatToDuploExtra2->v_USCOREphoto_USCOREextra->__ptr = jpeg_observe1;
			skatToDuploExtra2->v_USCOREphoto_USCOREextra->__size = res_size_Observe1;

			skatToDuploExtra2->v_USCOREframe_USCOREdatetime = &violaData.pViolaTime_ms;
			skatToDuploExtra2->v_USCOREtype_USCOREphoto = &violaData.typePhotoExtra;

			//observe2
			skatToDuploExtra3->v_USCOREphoto_USCOREextra->__ptr = jpeg_observe2;
			skatToDuploExtra3->v_USCOREphoto_USCOREextra->__size = res_size_Observe2;

			skatToDuploExtra3->v_USCOREframe_USCOREdatetime = &violaData.pViolaTime_ms;
			skatToDuploExtra3->v_USCOREtype_USCOREphoto = &violaData.typePhotoExtra;

			//коллаж
			skatToDuploExtra4->v_USCOREphoto_USCOREextra->__ptr = jpeg_main;
			skatToDuploExtra4->v_USCOREphoto_USCOREextra->__size = res_size_Maincar;

			skatToDuploExtra4->v_USCOREframe_USCOREdatetime = &violaData.pViolaTime_ms;
			skatToDuploExtra4->v_USCOREtype_USCOREphoto = &violaData.typePhotoExtra;

			senddata_psd->message->photo_USCOREextra.push_back(skatToDuploExtra);
			senddata_psd->message->photo_USCOREextra.push_back(skatToDuploExtra2);
			senddata_psd->message->photo_USCOREextra.push_back(skatToDuploExtra3);
			senddata_psd->message->photo_USCOREextra.push_back(skatToDuploExtra4);
		}

		//if (violaData.pViola == 80)
		//{
		//	//коллаж
		//	senddata_psd->message->tr_USCOREcheckIn->v_USCOREphoto_USCOREts->__size = res_size_Maincar;
		//	senddata_psd->message->tr_USCOREcheckIn->v_USCOREphoto_USCOREts->__ptr = jpeg_main;// jpeg_carExtra;// jpeg_main;

		//	//нарушение
		//	skatToDuploExtra->v_USCOREphoto_USCOREextra->__ptr = jpeg_carExtra;
		//	skatToDuploExtra->v_USCOREphoto_USCOREextra->__size = res_size_Extracar;

		//	skatToDuploExtra->v_USCOREframe_USCOREdatetime = &violaData.pViolaTime_ms;
		//	skatToDuploExtra->v_USCOREtype_USCOREphoto = &violaData.typePhotoExtra;

		//	senddata_psd->message->photo_USCOREextra.push_back(skatToDuploExtra);
		//}

		//маршрутные ТС
		if (violaData.pViola == 5)
		{
			//коллаж
			senddata_psd->message->tr_USCOREcheckIn->v_USCOREphoto_USCOREts->__size = res_size_Maincar;
			senddata_psd->message->tr_USCOREcheckIn->v_USCOREphoto_USCOREts->__ptr = jpeg_main;// jpeg_carExtra;// jpeg_main;

			//нарушение
			skatToDuploExtra->v_USCOREphoto_USCOREextra->__ptr = jpeg_carExtra;
			skatToDuploExtra->v_USCOREphoto_USCOREextra->__size = res_size_Extracar;

			skatToDuploExtra->v_USCOREframe_USCOREdatetime = &violaData.pViolaTime_ms;
			skatToDuploExtra->v_USCOREtype_USCOREphoto = &violaData.typePhotoExtra;

			//observe0
			skatToDuploExtra2->v_USCOREphoto_USCOREextra->__ptr = jpeg_observe0;
			skatToDuploExtra2->v_USCOREphoto_USCOREextra->__size = res_size_Observe0;

			skatToDuploExtra2->v_USCOREframe_USCOREdatetime = &violaData.pViolaTime_ms;
			skatToDuploExtra2->v_USCOREtype_USCOREphoto = &violaData.typePhotoExtra;

			//observe1
			skatToDuploExtra3->v_USCOREphoto_USCOREextra->__ptr = jpeg_observe1;
			skatToDuploExtra3->v_USCOREphoto_USCOREextra->__size = res_size_Observe1;

			skatToDuploExtra3->v_USCOREframe_USCOREdatetime = &violaData.pViolaTime_ms;
			skatToDuploExtra3->v_USCOREtype_USCOREphoto = &violaData.typePhotoExtra;

			senddata_psd->message->photo_USCOREextra.push_back(skatToDuploExtra);
			senddata_psd->message->photo_USCOREextra.push_back(skatToDuploExtra2);
			senddata_psd->message->photo_USCOREextra.push_back(skatToDuploExtra3);
		}

		//Грузовые ТС с МРМ более 2,5 т. в левой полосе 107
		if (violaData.pViola == 107 || violaData.pViola == 80)
		{
			//коллаж
			senddata_psd->message->tr_USCOREcheckIn->v_USCOREphoto_USCOREts->__size = res_size_Maincar;
			senddata_psd->message->tr_USCOREcheckIn->v_USCOREphoto_USCOREts->__ptr = jpeg_main;// jpeg_carExtra;// jpeg_main;

			//нарушение
			skatToDuploExtra->v_USCOREphoto_USCOREextra->__ptr = jpeg_carExtra;
			skatToDuploExtra->v_USCOREphoto_USCOREextra->__size = res_size_Extracar;

			skatToDuploExtra->v_USCOREframe_USCOREdatetime = &violaData.pViolaTime_ms;
			skatToDuploExtra->v_USCOREtype_USCOREphoto = &violaData.typePhotoExtra;

			senddata_psd->message->photo_USCOREextra.push_back(skatToDuploExtra);

			//observe0
			skatToDuploExtra2->v_USCOREphoto_USCOREextra->__ptr = jpeg_observe0;
			skatToDuploExtra2->v_USCOREphoto_USCOREextra->__size = res_size_Observe0;

			skatToDuploExtra2->v_USCOREframe_USCOREdatetime = &violaData.pViolaTime_ms;
			skatToDuploExtra2->v_USCOREtype_USCOREphoto = &violaData.typePhotoExtra;
			
			senddata_psd->message->photo_USCOREextra.push_back(skatToDuploExtra2);

			//observe1
			skatToDuploExtra3->v_USCOREphoto_USCOREextra->__ptr = jpeg_observe1;
			skatToDuploExtra3->v_USCOREphoto_USCOREextra->__size = res_size_Observe1;

			skatToDuploExtra3->v_USCOREframe_USCOREdatetime = &violaData.pViolaTime_ms;
			skatToDuploExtra3->v_USCOREtype_USCOREphoto = &violaData.typePhotoExtra;
						
			senddata_psd->message->photo_USCOREextra.push_back(skatToDuploExtra3);
		}

		//Движение грузовиков далее второго ряда 77
		if (violaData.pViola == 77)
		{
			//коллаж
			senddata_psd->message->tr_USCOREcheckIn->v_USCOREphoto_USCOREts->__size = res_size_Maincar;
			senddata_psd->message->tr_USCOREcheckIn->v_USCOREphoto_USCOREts->__ptr = jpeg_main;// jpeg_carExtra;// jpeg_main;

			//нарушение
			skatToDuploExtra->v_USCOREphoto_USCOREextra->__ptr = jpeg_carExtra;
			skatToDuploExtra->v_USCOREphoto_USCOREextra->__size = res_size_Extracar;

			skatToDuploExtra->v_USCOREframe_USCOREdatetime = &violaData.pViolaTime_ms;
			skatToDuploExtra->v_USCOREtype_USCOREphoto = &violaData.typePhotoExtra;

			senddata_psd->message->photo_USCOREextra.push_back(skatToDuploExtra);

			//observe0
			skatToDuploExtra2->v_USCOREphoto_USCOREextra->__ptr = jpeg_observe0;
			skatToDuploExtra2->v_USCOREphoto_USCOREextra->__size = res_size_Observe0;

			skatToDuploExtra2->v_USCOREframe_USCOREdatetime = &violaData.pViolaTime_ms;
			skatToDuploExtra2->v_USCOREtype_USCOREphoto = &violaData.typePhotoExtra;

			senddata_psd->message->photo_USCOREextra.push_back(skatToDuploExtra2);

			//observe1
			skatToDuploExtra3->v_USCOREphoto_USCOREextra->__ptr = jpeg_observe1;
			skatToDuploExtra3->v_USCOREphoto_USCOREextra->__size = res_size_Observe1;

			skatToDuploExtra3->v_USCOREframe_USCOREdatetime = &violaData.pViolaTime_ms;
			skatToDuploExtra3->v_USCOREtype_USCOREphoto = &violaData.typePhotoExtra;

			senddata_psd->message->photo_USCOREextra.push_back(skatToDuploExtra3);
		}

		//Движение грузовиков ДОРОЖНЫЙ ЗНАК 5.15.1
		if (violaData.pViola == 103)
		{
			//коллаж
			senddata_psd->message->tr_USCOREcheckIn->v_USCOREphoto_USCOREts->__size = res_size_Maincar;
			senddata_psd->message->tr_USCOREcheckIn->v_USCOREphoto_USCOREts->__ptr = jpeg_main;// jpeg_carExtra;// jpeg_main;

			//нарушение
			skatToDuploExtra->v_USCOREphoto_USCOREextra->__ptr = jpeg_carExtra;
			skatToDuploExtra->v_USCOREphoto_USCOREextra->__size = res_size_Extracar;

			skatToDuploExtra->v_USCOREframe_USCOREdatetime = &violaData.pViolaTime_ms;
			skatToDuploExtra->v_USCOREtype_USCOREphoto = &violaData.typePhotoExtra;

			senddata_psd->message->photo_USCOREextra.push_back(skatToDuploExtra);

			//observe0
			skatToDuploExtra2->v_USCOREphoto_USCOREextra->__ptr = jpeg_observe0;
			skatToDuploExtra2->v_USCOREphoto_USCOREextra->__size = res_size_Observe0;

			skatToDuploExtra2->v_USCOREframe_USCOREdatetime = &violaData.pViolaTime_ms;
			skatToDuploExtra2->v_USCOREtype_USCOREphoto = &violaData.typePhotoExtra;

			senddata_psd->message->photo_USCOREextra.push_back(skatToDuploExtra2);

			//observe1
			skatToDuploExtra3->v_USCOREphoto_USCOREextra->__ptr = jpeg_observe1;
			skatToDuploExtra3->v_USCOREphoto_USCOREextra->__size = res_size_Observe1;

			skatToDuploExtra3->v_USCOREframe_USCOREdatetime = &violaData.pViolaTime_ms;
			skatToDuploExtra3->v_USCOREtype_USCOREphoto = &violaData.typePhotoExtra;

			senddata_psd->message->photo_USCOREextra.push_back(skatToDuploExtra3);
		}
	
	}

	if (modeRegion == mode::ULYANOVSK)
	{

		//если нарушение - коллаж + вырезка
		//есди проезд - вырезка

		printf("violaData.pViola = %d\r\n", violaData.pViola);
		violaData.typePhotoExtra = L"1";

		//нарушение
		if (violaData.pViola != 0)
		{
			//коллаж
			senddata_psd->message->tr_USCOREcheckIn->v_USCOREphoto_USCOREts->__size = res_size_Maincar;
			senddata_psd->message->tr_USCOREcheckIn->v_USCOREphoto_USCOREts->__ptr = jpeg_main;// jpeg_carExtra;// jpeg_main;

			//нарушение
			skatToDuploExtra->v_USCOREphoto_USCOREextra->__ptr = jpeg_carExtra;
			skatToDuploExtra->v_USCOREphoto_USCOREextra->__size = res_size_Extracar;

			skatToDuploExtra->v_USCOREframe_USCOREdatetime = &violaData.pViolaTime_ms;
			skatToDuploExtra->v_USCOREtype_USCOREphoto = &violaData.typePhotoExtra;

			senddata_psd->message->photo_USCOREextra.push_back(skatToDuploExtra);
		}
		
		//проезд
		if (violaData.pViola == 0)
		{
			senddata_psd->message->tr_USCOREcheckIn->v_USCOREphoto_USCOREts->__size = res_size_Extracar;
			senddata_psd->message->tr_USCOREcheckIn->v_USCOREphoto_USCOREts->__ptr = jpeg_carExtra;// jpeg_main;
		}

		//FILE *f1 = _wfopen(L"c:\\1111.jpg", L"wb");
		//size_t size1 = fwrite(senddata_psd->message->tr_USCOREcheckIn->v_USCOREphoto_USCOREts->__ptr, 1, senddata_psd->message->tr_USCOREcheckIn->v_USCOREphoto_USCOREts->__size, f1);
		//fclose(f1);
		//FILE *f2 = _wfopen(L"c:\\2222.jpg", L"wb");
		//size_t size2 = fwrite(skatToDuploExtra->v_USCOREphoto_USCOREextra->__ptr, 1, skatToDuploExtra->v_USCOREphoto_USCOREextra->__size, f2);
		//fclose(f2);
	}

	//бурятия
	if (modeRegion == mode::BURYATIYA)
	{

		//если нарушение - коллаж + вырезка
		//есди проезд - вырезка

		printf("violaData.pViola = %d\r\n", violaData.pViola);
		violaData.typePhotoExtra = L"1";

		//нарушение и проезд
		if (violaData.pViola != 0)
		{
			//коллаж
			senddata_psd->message->tr_USCOREcheckIn->v_USCOREphoto_USCOREts->__size = res_size_Maincar;
			senddata_psd->message->tr_USCOREcheckIn->v_USCOREphoto_USCOREts->__ptr = jpeg_main;// jpeg_carExtra;// jpeg_main;

			//нарушение
			skatToDuploExtra->v_USCOREphoto_USCOREextra->__ptr = jpeg_carExtra;
			skatToDuploExtra->v_USCOREphoto_USCOREextra->__size = res_size_Extracar;

			skatToDuploExtra->v_USCOREframe_USCOREdatetime = &violaData.pViolaTime_ms;
			skatToDuploExtra->v_USCOREtype_USCOREphoto = &violaData.typePhotoExtra;

			senddata_psd->message->photo_USCOREextra.push_back(skatToDuploExtra);

			//бинарная подпись-1
			skatToDuploSignCar->v_USCOREphoto_USCOREextra->__ptr = jpeg_sign_0;
			skatToDuploSignCar->v_USCOREphoto_USCOREextra->__size = SIZE_SIGN;

			skatToDuploSignCar->v_USCOREframe_USCOREdatetime = &zero_time;
			skatToDuploSignCar->v_USCOREtype_USCOREphoto = &zero_type;//передаем "sign"

			senddata_psd->message->photo_USCOREextra.push_back(skatToDuploSignCar);

			//бинарная подпись-2
			skatToDuploSignView->v_USCOREphoto_USCOREextra->__ptr = jpeg_sign_1;
			skatToDuploSignView->v_USCOREphoto_USCOREextra->__size = SIZE_SIGN;

			skatToDuploSignView->v_USCOREframe_USCOREdatetime = &zero_time;
			skatToDuploSignView->v_USCOREtype_USCOREphoto = &zero_type;//передаем "sign"

			senddata_psd->message->photo_USCOREextra.push_back(skatToDuploSignView);

			int sizeVideo = getAviVideo(path);

			if (sizeVideo > 0)
			{
				violaData.typePhotoVideoExtra = L"3";
				addErrorLogMessage("video is ready to send");
				//передача видео
				skatToDuploExtraAviVideo->v_USCOREphoto_USCOREextra->__ptr = avi_video;
				skatToDuploExtraAviVideo->v_USCOREphoto_USCOREextra->__size = sizeVideo;

				skatToDuploExtraAviVideo->v_USCOREframe_USCOREdatetime = &violaData.pViolaTime_ms;
				skatToDuploExtraAviVideo->v_USCOREtype_USCOREphoto = &violaData.typePhotoVideoExtra;

				senddata_psd->message->photo_USCOREextra.push_back(skatToDuploExtraAviVideo);
			}
			
		}

		//проезд
		if (violaData.pViola == 0)
		{
			senddata_psd->message->tr_USCOREcheckIn->v_USCOREphoto_USCOREts->__size = res_size_Extracar;
			senddata_psd->message->tr_USCOREcheckIn->v_USCOREphoto_USCOREts->__ptr = jpeg_carExtra;// jpeg_main;

			//бинарная подпись-1
			skatToDuploSignCar->v_USCOREphoto_USCOREextra->__ptr = jpeg_sign_0;
			skatToDuploSignCar->v_USCOREphoto_USCOREextra->__size = SIZE_SIGN;

			skatToDuploSignCar->v_USCOREframe_USCOREdatetime = &zero_time;
			skatToDuploSignCar->v_USCOREtype_USCOREphoto = &zero_type;

			senddata_psd->message->photo_USCOREextra.push_back(skatToDuploSignCar);
		}
	}

	//ставрполь
	if (modeRegion == mode::STAVROPOL)
	{

		//если нарушение - коллаж + вырезка
		//есди проезд - вырезка

		printf("violaData.pViola = %d\r\n", violaData.pViola);
		violaData.typePhotoExtra = L"1";

		//нарушение и проезд
		if (violaData.pViola != 0)
		{
			//коллаж
			senddata_psd->message->tr_USCOREcheckIn->v_USCOREphoto_USCOREts->__size = res_size_Maincar;
			senddata_psd->message->tr_USCOREcheckIn->v_USCOREphoto_USCOREts->__ptr = jpeg_main;// jpeg_carExtra;// jpeg_main;

			//нарушение
			skatToDuploExtra->v_USCOREphoto_USCOREextra->__ptr = jpeg_carExtra;
			skatToDuploExtra->v_USCOREphoto_USCOREextra->__size = res_size_Extracar;

			skatToDuploExtra->v_USCOREframe_USCOREdatetime = &violaData.pViolaTime_ms;
			skatToDuploExtra->v_USCOREtype_USCOREphoto = &violaData.typePhotoExtra;

			senddata_psd->message->photo_USCOREextra.push_back(skatToDuploExtra);

			//бинарная подпись-1
			skatToDuploSignCar->v_USCOREphoto_USCOREextra->__ptr = jpeg_sign_0;
			skatToDuploSignCar->v_USCOREphoto_USCOREextra->__size = SIZE_SIGN;

			skatToDuploSignCar->v_USCOREframe_USCOREdatetime = &zero_time;
			skatToDuploSignCar->v_USCOREtype_USCOREphoto = &zero_type;//передаем "sign"

			senddata_psd->message->photo_USCOREextra.push_back(skatToDuploSignCar);

			//бинарная подпись-2
			skatToDuploSignView->v_USCOREphoto_USCOREextra->__ptr = jpeg_sign_1;
			skatToDuploSignView->v_USCOREphoto_USCOREextra->__size = SIZE_SIGN;

			skatToDuploSignView->v_USCOREframe_USCOREdatetime = &zero_time;
			skatToDuploSignView->v_USCOREtype_USCOREphoto = &zero_type;//передаем "sign"

			senddata_psd->message->photo_USCOREextra.push_back(skatToDuploSignView);



			int sizeVideo = getAviVideo(path);

			if (sizeVideo > 0)
			{
				violaData.typePhotoVideoExtra = L"3";
				addErrorLogMessage("video is ready to send");
				//передача видео
				skatToDuploExtraAviVideo->v_USCOREphoto_USCOREextra->__ptr = avi_video;
				skatToDuploExtraAviVideo->v_USCOREphoto_USCOREextra->__size = sizeVideo;

				skatToDuploExtraAviVideo->v_USCOREframe_USCOREdatetime = &violaData.pViolaTime_ms;
				skatToDuploExtraAviVideo->v_USCOREtype_USCOREphoto = &violaData.typePhotoVideoExtra;

				senddata_psd->message->photo_USCOREextra.push_back(skatToDuploExtraAviVideo);
			}
		}
		else return -1;
	}

	printf("photo_USCOREextra = %d\r\n", senddata_psd->message->photo_USCOREextra.size());

	return 0;
}
int COLSO::calcGns(unsigned char* bArray, char* pLat, char* pLon)
{
	//Координаты
	// Широта
	int nLatitude3 = bArray[0];
	int nLatitude2 = bArray[1];
	int nLatitude1 = bArray[2];
	int nLatitudeNS = bArray[3];

	// Долгота
	int nLongitude3 = bArray[5];
	int nLongitude2 = bArray[6];
	int nLongitude1 = bArray[7];
	int nLongitudeWE = bArray[8];

	int nLonitude11 = (char)nLongitudeWE & 0x0f;
	nLongitudeWE = ((char)nLongitudeWE & 0xF0) >> 4;
	nLatitudeNS = ((char)nLatitudeNS & 0xF0) >> 4;

	if (nLongitude1 != 0 && nLatitude1 != 0)
	{
		char chLatitudeNS = 'N';
		if (nLatitudeNS == 1)
			chLatitudeNS = 'S';
		else
			chLatitudeNS = 'N';

		char chLongitudeWE = 'E';
		if (nLongitudeWE == 1)
			chLongitudeWE = 'W';
		else
			chLongitudeWE = 'E';

		//		sprintf(pLat,"%c%02x.%02x%02x", chLatitudeNS, nLatitude1, nLatitude2, nLatitude3);
		//		sprintf(pLon, "%c%d%02x.%02x%02x", chLongitudeWE, nLonitude11, nLongitude1, nLongitude2, nLongitude3);

		sprintf(pLat, "%02x.%02x%02x", nLatitude1, nLatitude2, nLatitude3);
		sprintf(pLon, "%d%02x.%02x%02x", nLonitude11, nLongitude1, nLongitude2, nLongitude3);
	}

	return 1;
}
int COLSO::compress(int width, int height, unsigned char *outbuf, int allSize, unsigned char *image_buffer)
{
	unsigned long outsize = allSize;

	//compress
	compress_cinfo.err = jpeg_std_error(&compress_jerr);
	jpeg_create_compress(&compress_cinfo);

	jpeg_mem_dest(&compress_cinfo, &outbuf, &outsize);

	compress_cinfo.image_width = width;      /* image width and height, in pixels */
	compress_cinfo.image_height = height;
	compress_cinfo.input_components = 1;     /* # of color components per pixel */
	compress_cinfo.in_color_space = JCS_GRAYSCALE; /* colorspace of input image */
	jpeg_set_defaults(&compress_cinfo);

	jpeg_start_compress(&compress_cinfo, TRUE);

	JSAMPROW row_pointer[1];        /* pointer to a single row */
	int row_stride;                 /* physical row width in buffer */
	row_stride = width * 1;   /* JSAMPLEs per row in image_buffer */
	while ((compress_cinfo.next_scanline < compress_cinfo.image_height) && bRunning) {
		row_pointer[0] = &image_buffer[compress_cinfo.next_scanline * row_stride];
		jpeg_write_scanlines(&compress_cinfo, row_pointer, 1);
	}

	jpeg_finish_compress(&compress_cinfo);

	jpeg_destroy_compress(&compress_cinfo);

	return outsize;

}
int COLSO::decompress(/*char* bmp_path,*/int size_copy)
{

	//dcompress
	decompress_cinfo.err = jpeg_std_error(&decompress_jerr);
	jpeg_create_decompress(&decompress_cinfo);

	jpeg_mem_src(&decompress_cinfo, jpeg_carExtra, size_copy);

	int rc = jpeg_read_header(&decompress_cinfo, TRUE);

	if (rc != 1)
	{
		return -1;
	}

	jpeg_start_decompress(&decompress_cinfo);


	unsigned int width_extra = decompress_cinfo.output_width;
	unsigned int height_crop = decompress_cinfo.output_height;
	unsigned int pixel_size = decompress_cinfo.output_components;
	unsigned int bmp_size = width_extra * height_crop * pixel_size;

	unsigned int row_stride = width_extra * pixel_size;

	while ((decompress_cinfo.output_scanline < decompress_cinfo.output_height) && bRunning)
	{
		unsigned char *buffer_array[1];
		buffer_array[0] = bmp_carExtra + (decompress_cinfo.output_scanline) * row_stride;
		jpeg_read_scanlines(&decompress_cinfo, buffer_array, 1);
	}

	//SaveBMP(bmp_path, width_crop, height_crop, bmp_carCrop);

	jpeg_finish_decompress(&decompress_cinfo);

	jpeg_destroy_decompress(&decompress_cinfo);

	return width_extra;
}
//int COLSO::InitJpeg()
//{	return 1;
//}
//int COLSO::KillJpeg()
//{	return 1;
//}
bool COLSO::SaveBMP(char* pFileName_i, int nWidth_i, int nHeight_i, unsigned char* pbyData_i)
{

	//неправильное сохранение, нужно уменьшит на 1, какой то из размеров
	FILE* pFile = 0;
	//try
	{
		if (0 == pbyData_i) return false;

		int nNewWidth = nWidth_i;//ceil((nWidth_i * 4) / 4.0) * 4.0;

		int nBMPType = 19778; // BM
		int nOffsetHeader = sizeof(BITMAPINFOHEADER) + sizeof(BITMAPFILEHEADER) + sizeof(RGBQUAD) * 256;
		int nFileSize = (nHeight_i * nNewWidth) + nOffsetHeader;

		// Fill the Bitmap File header.
		BITMAPFILEHEADER stBitmapFileHeader = { 0 };
		stBitmapFileHeader.bfType = nBMPType;
		stBitmapFileHeader.bfOffBits = nOffsetHeader;
		stBitmapFileHeader.bfReserved1 = 0;
		stBitmapFileHeader.bfSize = nFileSize;
		stBitmapFileHeader.bfReserved2 = 0;

		int nWrappedWidth = nWidth_i;// ceil( (float)nWidth_i / 4.0 ) * 4;
		// Fill the Bitmap Info header.

		//stBitmapInfo = (BITMAPINFO *)m_chBmpBuf;

		stBitmapInfo->bmiHeader.biClrImportant = 0;
		stBitmapInfo->bmiHeader.biClrUsed = 256;
		stBitmapInfo->bmiHeader.biCompression = 0;
		stBitmapInfo->bmiHeader.biPlanes = 1;
		stBitmapInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		stBitmapInfo->bmiHeader.biXPelsPerMeter = 0;
		stBitmapInfo->bmiHeader.biYPelsPerMeter = 0;
		stBitmapInfo->bmiHeader.biBitCount = 1 * 8;
		stBitmapInfo->bmiHeader.biSizeImage = 0;// nHeight_i * nNewWidth;
		stBitmapInfo->bmiHeader.biHeight = -nHeight_i;
		stBitmapInfo->bmiHeader.biWidth = nWidth_i;

		for (int i = 0; i < 256; i++)
		{
			stBitmapInfo->bmiColors[i].rgbBlue = i;
			stBitmapInfo->bmiColors[i].rgbGreen = i;
			stBitmapInfo->bmiColors[i].rgbRed = i;
			stBitmapInfo->bmiColors[i].rgbReserved = 0x0;
		}

		FILE* fpFile;
		fopen_s(&fpFile, pFileName_i, "wb");
		fwrite(&stBitmapFileHeader, 1, sizeof(BITMAPFILEHEADER), fpFile);
		fwrite(stBitmapInfo, 1, sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 256, fpFile);


		//for (int nDataSize = 0; nDataSize < nHeight_i * nNewWidth; nDataSize+=1024)
		{
			//int nRemSize = ( ( nDataSize + 1024) <= nHeight_i * nNewWidth ) ? 1024 : nHeight_i * nNewWidth - (nDataSize * 1024);
			fwrite(pbyData_i, 1, (nHeight_i * nWidth_i * 1), fpFile);
		}
		fclose(fpFile);//[Save BMP]
		return true;
	}


}
int COLSO::sendData(MessengerServiceSoapBindingProxy *obj)
{
	printf("-------------------------------------------------\r\n");


	printf("camera = %s\r\n", utf8_encode(senddata_psd->message->tr_USCOREcheckIn->v_USCOREcamera).c_str());
#ifdef AZIMUT_ENABLE
	if (senddata_psd->message->tr_USCOREcheckIn->v_USCOREazimut)							printf("azimut = %s\r\n",			utf8_encode(*senddata_psd->message->tr_USCOREcheckIn->v_USCOREazimut).c_str());
#endif
	if (senddata_psd->message->tr_USCOREcheckIn->v_USCOREgps_USCOREx)						printf("longitude = %s\r\n", utf8_encode(*senddata_psd->message->tr_USCOREcheckIn->v_USCOREgps_USCOREx).c_str());
	if (senddata_psd->message->tr_USCOREcheckIn->v_USCOREgps_USCOREy)						printf("latitude = %s\r\n", utf8_encode(*senddata_psd->message->tr_USCOREcheckIn->v_USCOREgps_USCOREy).c_str());
	if (senddata_psd->message->tr_USCOREcheckIn->v_USCOREdirection)							printf("direction = %s\r\n", utf8_encode(*senddata_psd->message->tr_USCOREcheckIn->v_USCOREdirection).c_str());
	if (senddata_psd->message->tr_USCOREcheckIn->v_USCOREspeed_USCORElimit)					printf("speed limit = %d\r\n", *senddata_psd->message->tr_USCOREcheckIn->v_USCOREspeed_USCORElimit);
	if (senddata_psd->message->tr_USCOREcheckIn->v_USCOREspeed)								printf("speed = %s\r\n", utf8_encode(*senddata_psd->message->tr_USCOREcheckIn->v_USCOREspeed).c_str());
	if (senddata_psd->message->tr_USCOREcheckIn->v_USCOREregno_USCOREcountry_USCOREid)		printf("country id = %s\r\n", utf8_encode(*senddata_psd->message->tr_USCOREcheckIn->v_USCOREregno_USCOREcountry_USCOREid).c_str());
	if (senddata_psd->message->tr_USCOREcheckIn->v_USCOREregno_USCOREcolor_USCOREid)		printf("color id = %d\r\n", *senddata_psd->message->tr_USCOREcheckIn->v_USCOREregno_USCOREcolor_USCOREid);
	if (senddata_psd->message->tr_USCOREcheckIn->v_USCORErecognition_USCOREaccuracy)		printf("accuracy = %s\r\n", utf8_encode(*senddata_psd->message->tr_USCOREcheckIn->v_USCORErecognition_USCOREaccuracy).c_str());
	if (senddata_psd->message->tr_USCOREcheckIn->v_USCOREregno)								printf("recogn = %s\r\n", utf8_encode(*senddata_psd->message->tr_USCOREcheckIn->v_USCOREregno).c_str());
	if (senddata_psd->message->tr_USCOREcheckIn->v_USCOREpr_USCOREviol.size()>0)			printf("code viola = %d\r\n", senddata_psd->message->tr_USCOREcheckIn->v_USCOREpr_USCOREviol[0]);
	if (senddata_psd->message->tr_USCOREcheckIn->v_USCOREparking_USCOREnum)					printf("parking num = %d\r\n", *senddata_psd->message->tr_USCOREcheckIn->v_USCOREparking_USCOREnum);
	if (senddata_psd->message->tr_USCOREcheckIn->v_USCOREparking_USCOREzone)				printf("parking zone = %d\r\n", *senddata_psd->message->tr_USCOREcheckIn->v_USCOREparking_USCOREzone);
	if (senddata_psd->message->tr_USCOREcheckIn->v_USCORElane_USCOREnum)					printf("lane num = %d\r\n", *senddata_psd->message->tr_USCOREcheckIn->v_USCORElane_USCOREnum);
	if (senddata_psd->message->tr_USCOREcheckIn->v_USCOREcamera_USCOREplace)				printf("place = %s\r\n", utf8_encode(*senddata_psd->message->tr_USCOREcheckIn->v_USCOREcamera_USCOREplace).c_str());

	//char tempTime[256] = { 0 };
	tm * timeinfo;
	timeinfo = gmtime((&senddata_psd->message->tr_USCOREcheckIn->v_USCOREtime_USCOREcheck));
	printf("gmtime %d\r\n", senddata_psd->message->tr_USCOREcheckIn->v_USCOREtime_USCOREcheck);
	printf("date-time = %d-%02d-%02d %02d:%02d:%02d\r\n", timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
	//sprintf(tempTime,"date-time = %d-%02d-%02d %02d:%02d:%02d\r\n", timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);

	short int hourOffset = 0;
	short int minOffset = 0;

	hourOffset = violaData.pViolaTimeOffset / (3600.0 * 1000);
	minOffset = ((std::abs(violaData.pViolaTimeOffset) - std::abs(hourOffset * 3600 * 1000)) / 1000) / 60;

	printf("time offset %02d:%02d\r\n", hourOffset, minOffset);

#ifndef DEBUG_PROC
	if (hourOffset > 0)
		sprintf(objDuplo->soap->tzOffsetHour, "+%02d", hourOffset);
	else
		sprintf(objDuplo->soap->tzOffsetHour, "-%02d", hourOffset);

	sprintf(objDuplo->soap->tzOffsetMins, "%02d", minOffset);


	//sprintf(tempTime, "xml-time is %s:%s", objDuplo->soap->tzOffsetHour, objDuplo->soap->tzOffsetMins);
	//addErrorLogMessage(tempTime);

	if(violaData.pViolaMs.size() <= 4)
	sprintf(objDuplo->soap->tzMillisec, "%s", violaData.pViolaMs.c_str());

	printf("-------------------------------------------------\r\n");

	printf("photo_USCOREextra = %d\r\n", senddata_psd->message->photo_USCOREextra.size());
	if (senddata_psd->message->photo_USCOREextra.size() > 0)
	{
		if (senddata_psd->message->photo_USCOREextra.at(0)->v_USCOREframe_USCOREdatetime)
		{
			timeinfo = gmtime((const time_t*)(senddata_psd->message->photo_USCOREextra.at(0)->v_USCOREframe_USCOREdatetime));
			printf("gmtime %d\r\n", *senddata_psd->message->photo_USCOREextra.at(0)->v_USCOREframe_USCOREdatetime);
			printf("date-time = %d-%02d-%02d %02d:%02d:%02d\r\n", timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
		}

		if (senddata_psd->message->photo_USCOREextra.at(0)->v_USCOREtype_USCOREphoto)		printf("type photo = %s\r\n", utf8_encode(*senddata_psd->message->photo_USCOREextra.at(0)->v_USCOREtype_USCOREphoto).c_str());

		//senddata_psd->message->photo_USCOREextra.reserve(2);
	}

	printf("-------------------------------------------------\r\n");



	objDuplo->soap->tcp_keep_alive = 1;
	objDuplo->soap->keep_alive = 1;
#endif

	int resProc = 0;
	int res = 0;

	try
	{
		printf("transmit...\r\n");
		addErrorLogMessage("process-start...");
		SYSTEMTIME time_start;
		SYSTEMTIME time_end;

//#ifdef DEBUG_PROC
//		obj->soap->buf;
//		FILE *f = _wfopen(L"c:\\xml.log", L"at");
//		size_t size = fwrite(obj->soap->buf, 1, obj->soap->buflen , f);
//		fclose(f);//_wfopen(path_of_log_error_file [saveLog]
//#endif

		GetSystemTime(&time_start);
		resProc = obj->process(senddata_psd, senddata_psdr);
		GetSystemTime(&time_end);
		middle_cnt_send++;


//#ifdef DEBUG_PROC
		//FILE *f = _wfopen(L"c:\\xml.log", L"at");
		//int size = fwrite(obj->soap->buf, 1, obj->soap->buflen, f);
		//fclose(f);//_wfopen(path_of_log_error_file [saveLog]
//#endif

		//int diff_time_send = 
		//	(time_end.wYear - time_start.wYear) + 
		//	(time_end.wMonth - time_start.wMonth) + 
		//	(time_end.wDay - time_start.wDay) * 24 * 60 * 60 * 1000 +
		//	(time_end.wHour - time_start.wHour) * 60 * 60 * 1000 +
		//	(time_end.wMinute - time_start.wMinute)*60*1000 +
		//	(time_end.wSecond - time_start.wSecond)*1000 +
		//	time_end.wMilliseconds - time_start.wMilliseconds;

		time_t sec_diff = TimeFromSystemTime(&time_end) - TimeFromSystemTime(&time_start);

		int diff_time_send = (int)(sec_diff) * 1000 + abs(time_end.wMilliseconds - time_start.wMilliseconds);

		middle_time_send += diff_time_send;
		double hg = (double)(middle_time_send / (double)middle_cnt_send);

		char sendTimeErr[256];
		sprintf(sendTimeErr, "time to send[%d ms][middle time = %7.3f][cnt=%d]", diff_time_send, hg, middle_cnt_send);
		addErrorLogMessage(sendTimeErr);

		if (time_start.wDay != _tempDay)
		{
			nerrCntFillData = 0;
			_tempDay = time_start.wDay;
		}


		time_t diffTime = senddata_psd->message->tr_USCOREcheckIn->v_USCOREtime_USCOREcheck - timeTemp;
		timeTemp = senddata_psd->message->tr_USCOREcheckIn->v_USCOREtime_USCOREcheck;

		//calc size
		int extraSize = 0;
		for (int i = 0; i < senddata_psd->message->photo_USCOREextra.size(); i++)
			extraSize += senddata_psd->message->photo_USCOREextra.at(i)->v_USCOREphoto_USCOREextra->__size;

		extraSize += senddata_psd->message->tr_USCOREcheckIn->v_USCOREphoto_USCOREgrz->__size;
		extraSize += senddata_psd->message->tr_USCOREcheckIn->v_USCOREphoto_USCOREts->__size;
		extraSize += sizeof(ns1__trCheckIn);//senddata_psd->message->tr_USCOREcheckIn;

		setPipeParams(
			/*скорость формирования штук в секунду*/diffTime/2,
			/*скорость отправки штук в секунду*/1.0 / (hg*0.001),
			/*скорость отправки кб в сек*/(extraSize/1024)/(diff_time_send*0.001),
			/*количество ошибок в сутки*/nerrCntFillData,
			/*очередь*/ temp_diff_message_lws,
			/*качество связи*/diff_time_send);

		//char sendErr[256];
		//sprintf(sendErr, "buflen = %d extraSize = %d[%3.3f MB][%3.3f MB]", objDuplo->soap->buflen, extraSize, (double)((double)extraSize / ((double)(1024 * 1024))), (double)((double)extraSize*1.35 / ((double)(1024 * 1024))));
		//addErrorLogMessage(sendErr);

		if (resProc == SOAP_TCP_ERROR)
		{
			checkFault();
			printf("SOAP_TCP_ERROR\r\n");

			//char sendErr[256];
			//sprintf(sendErr, "SOAP_TCP_ERROR", resProc);
			addErrorLogMessage("SOAP_TCP_ERROR");

			return UNUSUAL_ERROR;
		}

	}
	catch (...)
	{
		printf("error!!\r\n");
	}

	

	if (senddata_psdr.return_ && resProc == SOAP_OK)
	{

//#ifdef DEBUG_PROC
//		char OkFill[256];
//		sprintf(OkFill, "senddata is OK\r\nokSend = %d", ++nOkCntFillData);
//		addErrorLogMessage(OkFill);
//#endif

		printf("transmit finished[SOAP_OK]\r\n");
		return 0;
	}
	else
	{
		res = checkFault();

		char sendErr[256];
		sprintf(sendErr, "checkFault = %d", res);
		addErrorLogMessage(sendErr);
	}

	printf("checkFault = %d resProc = %d\r\n", res, resProc);

	if ((res == 0) && (resProc != SOAP_OK))
	{
		//todo
		char sendErrUnusal[256];
		sprintf(sendErrUnusal, "UNUSUAL_ERROR = %d", resProc);
		addErrorLogMessage(sendErrUnusal);

		res = UNUSUAL_ERROR;
	}

	return res;
}
int COLSO::getVersion(MessengerServiceSoapBindingProxy *obj)
{
	ns1__getVersion gv;
	ns1__getVersionResponse gvr;

	obj->getVersion(&gv, gvr);

	if (gvr.return_ != nullptr)
		if(gvr.return_->buildVer)
			printf("soap version: %s\r\n", utf8_encode(*gvr.return_->buildVer).c_str());
	else
	{
		addErrorLogMessage("cant get version");
		return -1;
	}

	//int res = checkFault();

	return 0;
}
int COLSO::getInfo(MessengerServiceSoapBindingProxy *obj, std::string *camera_id)
{

	printf("/////////////*************Get Device Info************//////////////////\r\n");
	ns1__processGetInfo pgi;
	ns1__processGetInfoResponse pgir;
	//(*camera_id).append(2, 0);
	std::wstring id = utf8_decode(*camera_id);
	std::string s_id = "";
	
	if (modeRegion == mode::MOSKVA)
	{
		if (set.c_type.compare("p") == 0)
			s_id = *camera_id;
		else
		{
			if (set.prefix)s_id.append("SK");

			s_id.append(*camera_id); //utf8_decode(strCameraId);
			addErrorLogMessage("CameraId is");
			addErrorLogMessage((char*)s_id.c_str(), false);
		}
	}

	if (modeRegion == mode::MOSKVA2)
	{
		if (set.c_type.compare("p") == 0)
			s_id = *camera_id;
		else
		{
			if (set.prefix)s_id.append("SK");

			s_id.append(*camera_id); //utf8_decode(strCameraId);
			addErrorLogMessage("CameraId is");
			addErrorLogMessage((char*)s_id.c_str(), false);
		}
	}
	id = utf8_decode(s_id);
	addErrorLogMessage((char*)utf8_encode(id).c_str(),false);
	
	pgi.camera = &id;
	obj->processGetInfo(&pgi, pgir);

	if (pgir.return_ != nullptr)
	{
		char regInfo[4096] = { 0 };
		char tempInfo[4096] = { 0 };

		if (pgir.return_->azimut)				{ sprintf(tempInfo, "azimut = %s\r\n", utf8_encode(*pgir.return_->azimut).c_str());				strcat(regInfo, tempInfo); }
		if (pgir.return_->camera)				{ sprintf(tempInfo, "camera = %s\r\n", utf8_encode(*pgir.return_->camera).c_str());				strcat(regInfo, tempInfo); }
		if (pgir.return_->camera_USCOREid)		{ sprintf(tempInfo, "id = %d\r\n", *pgir.return_->camera_USCOREid);								strcat(regInfo, tempInfo); }
		if (pgir.return_->camera_USCOREmodel)	{ sprintf(tempInfo, "model = %s\r\n", utf8_encode(*pgir.return_->camera_USCOREmodel).c_str());	strcat(regInfo, tempInfo); }
		if (pgir.return_->camera_USCOREplace)	{ sprintf(tempInfo, "place = %s\r\n", utf8_encode(*pgir.return_->camera_USCOREplace).c_str());	strcat(regInfo, tempInfo); }
		if (pgir.return_->gps_USCOREx)			{ sprintf(tempInfo, "x = %s\r\n", utf8_encode(*pgir.return_->gps_USCOREx).c_str());				strcat(regInfo, tempInfo); }
		if (pgir.return_->gps_USCOREy)			{ sprintf(tempInfo, "y = %s\r\n", utf8_encode(*pgir.return_->gps_USCOREy).c_str());				strcat(regInfo, tempInfo); }
		if (pgir.return_->lane_USCOREnum)		{ sprintf(tempInfo, "num = %d\r\n", *pgir.return_->lane_USCOREnum);								strcat(regInfo, tempInfo); }
		if (pgir.return_->print_USCOREname)		{ sprintf(tempInfo, "name = %s\r\n", utf8_encode(*pgir.return_->print_USCOREname).c_str());		strcat(regInfo, tempInfo); }
		if (pgir.return_->priz_USCOREarh)		{ sprintf(tempInfo, "arh = %s\r\n", utf8_encode(*pgir.return_->priz_USCOREarh).c_str());		strcat(regInfo, tempInfo); }
		if (pgir.return_->p_USCOREnode)			{ sprintf(tempInfo, "node = %d\r\n", *pgir.return_->p_USCOREnode);								strcat(regInfo, tempInfo); }
		if (pgir.return_->serial_USCOREno)		{ sprintf(tempInfo, "no = %s\r\n", utf8_encode(*pgir.return_->serial_USCOREno).c_str());		strcat(regInfo, tempInfo); }

		addErrorLogMessage(regInfo);

		//if (pgir.return_->azimut) wprintf(L"azimut = %s\r\n", pgir.return_->azimut);
		//if (pgir.return_->camera) wprintf(L"camera = %s\r\n", pgir.return_->camera);
		//if (pgir.return_->camera_USCOREid) printf("id = %d\r\n", *pgir.return_->camera_USCOREid);
		//if (pgir.return_->camera_USCOREmodel) wprintf(L"model = %s\r\n", pgir.return_->camera_USCOREmodel);
		//if (pgir.return_->camera_USCOREplace) wprintf(L"place = %s\r\n", pgir.return_->camera_USCOREplace);
		//if (pgir.return_->gps_USCOREx) wprintf(L"x = %s\r\n", pgir.return_->gps_USCOREx);
		//if (pgir.return_->gps_USCOREy) wprintf(L"y = %s\r\n", pgir.return_->gps_USCOREy);
		//if (pgir.return_->lane_USCOREnum) printf("num = %d\r\n", *pgir.return_->lane_USCOREnum);
		//if (pgir.return_->print_USCOREname) wprintf(L"name = %s\r\n", pgir.return_->print_USCOREname);
		//if (pgir.return_->priz_USCOREarh) wprintf(L"arh = %s\r\n", pgir.return_->priz_USCOREarh);
		//if (pgir.return_->p_USCOREnode) printf("node = %d\r\n", *pgir.return_->p_USCOREnode);
		//if (pgir.return_->serial_USCOREno) wprintf(L"no = %s\r\n", pgir.return_->serial_USCOREno);
	}
	else
	{
		//todo
		char gg[512];
		sprintf(gg, "device %s is not register", s_id.c_str());
		addErrorLogMessage(gg);
		checkFault();
		return -1;
	}

	printf("getInfo\r\n");

	//int res = checkFault();

	return 0;
}
std::string COLSO::base64_encode(std::string *strBase64, unsigned char* bytes_to_encode, unsigned int in_len)
{
	strBase64->clear();

	int i = 0;
	int j = 0;
	unsigned char char_array_3[3];
	unsigned char char_array_4[4];

	while ((in_len--) && bRunning)
	{
		char_array_3[i++] = *(bytes_to_encode++);
		if (i == 3)
		{
			char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
			char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
			char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
			char_array_4[3] = char_array_3[2] & 0x3f;

			for (i = 0; (i < 4); i++)
				*strBase64 += base64_chars[char_array_4[i]];
			i = 0;
		}
	}

	if (i)
	{
		for (j = i; j < 3; j++)
			char_array_3[j] = '\0';

		char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
		char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
		char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
		char_array_4[3] = char_array_3[2] & 0x3f;

		for (j = 0; (j < i + 1); j++)
			*strBase64 += base64_chars[char_array_4[j]];

		while ((i++ < 3) && bRunning)
			*strBase64 += '=';

	}

	return *strBase64;

}
std::wstring COLSO::charToWString(const char* text)
{

	const size_t size = std::strlen(text);
	std::wstring wstr;
	if (size > 0) {
		wstr.resize(size);
		std::mbstowcs(&wstr[0], text, size);
	}
	return wstr;
}
std::wstring COLSO::utf8_decode(std::string str)
{
	int size_needed = (int)str.size()+2;// MultiByteToWideChar(CP_ACP, 0, &str[0], (int)str.size(), NULL, 0);
	std::wstring wstrTo(size_needed, 0);
	int yy = MultiByteToWideChar(CP_ACP, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
	return wstrTo;
}
std::wstring COLSO::utf8_decode_2(const std::string &str)
{
	int size_needed = MultiByteToWideChar(1251, 0, &str[0], (int)str.size(), NULL, 0);
	std::wstring wstrTo(size_needed, 0);
	MultiByteToWideChar(1251, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
	return wstrTo;
}
std::wstring COLSO::utf8_decode_3(const std::string &str)
{
	int size_needed = MultiByteToWideChar(65001, 0, &str[0], (int)str.size(), NULL, 0);
	std::wstring wstrTo(size_needed, 0);
	MultiByteToWideChar(65001, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
	return wstrTo;
}
std::string COLSO::utf8_encode(std::wstring wstr)
{
	if (wstr.length() == 0)
		return "";

	if (wstr.empty()) return std::string();
	int size_needed = (int)wstr.size()+2;// WideCharToMultiByte(1251, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
	std::string strTo(size_needed, 0);
	int yy = WideCharToMultiByte(1251, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
	return strTo;
}
std::string COLSO::utf8_encode2(std::wstring wstr)
{
	if (wstr.length() == 0)
		return "";

	if (wstr.empty()) return std::string();
	int size_needed = (int)wstr.size() + 2;// WideCharToMultiByte(1251, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
	std::string strTo(size_needed, 0);
	int yy = WideCharToMultiByte(65001, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
	return strTo;
}
bool COLSO::load_settings()
{
	memset(path_id, 0, sizeof(wchar_t)*MAX_PATH);
	wcscat(path_id, path_root);
	wcscat(path_id, L"soap.json");

	wprintf(L"id txt is: %s\r\n", path_id);

	std::wstring path = path_id;// L"c:\\soap.json";

	std::ifstream ifs(path);
	rapidjson::IStreamWrapper isw(ifs);
	rapidjson::EncodedInputStream<rapidjson::UTF8<>, rapidjson::IStreamWrapper> eis(isw);
	rapidjson::Document doc;
	doc.ParseStream(eis);
	//    

	/*

	"last_data": 2018-01-24,
	"last_time": 18:17:01
	*/

	auto tp = doc.GetType();
	if (!doc.IsObject())
		return false;

	if (!doc.HasMember("start_date"))
	{
		//bool res = doc["start_date"].IsString();
		addErrorLogMessage("soap.json: [start_date] is wrong!!!");
		return false;
	}

	if (!doc.HasMember("server"))
	{
		addErrorLogMessage("soap.json: [server] is wrong!!!");
		//bool res = doc["server"].IsString();
		return false;
	}

	if (!doc.HasMember("region"))
	{
		addErrorLogMessage("soap.json: [region] is wrong!!!");
		//bool res = doc["region"].IsString();
		return false;
	}

	if (!doc.HasMember("prefix"))
	{
		set.prefix = true;
		addErrorLogMessage("soap.json: [region] set is true!!!");
		//bool res = doc["prefix"].IsBool();
		//return false;
	}
	else
		set.prefix = doc["prefix"].GetBool();

	if (!doc.HasMember("jpeg_only"))
	{
		addErrorLogMessage("soap.json: [jpeg_only] set is true!!!");
		set.jpeg_only = true;
	}
	else
		set.jpeg_only = doc["jpeg_only"].GetBool();



	if (!doc.HasMember("accept_timeout"))
	{
		//addErrorLogMessage("soap.json: [accept_timeout] set is absent!!!");
		set.accept_timeout = 60;
	}
	else
		set.accept_timeout = doc["accept_timeout"].GetInt();


	if (!doc.HasMember("connect_timeout"))
	{
		//addErrorLogMessage("soap.json: [connect_timeout] set is absent!!!");
		set.connect_timeout = 60;
	}
	else
		set.connect_timeout = doc["connect_timeout"].GetInt();


	if (!doc.HasMember("recv_timeout"))
	{
		//addErrorLogMessage("soap.json: [recv_timeout] set is absent!!!");
		set.recv_timeout = 60;
	}
	else
		set.recv_timeout = doc["recv_timeout"].GetInt();

	if (!doc.HasMember("send_timeout"))
	{
		//addErrorLogMessage("soap.json: [send_timeout] set is absent!!!");
		set.send_timeout = 120;
	}
	else
		set.send_timeout = doc["send_timeout"].GetInt();

	if (!doc.HasMember("transfer_timeout"))
	{
		//addErrorLogMessage("soap.json: [transfer_timeout] set is absent!!!");
		set.transfer_timeout = 120;
	}
	else
		set.transfer_timeout = doc["transfer_timeout"].GetInt();

	if (!doc.HasMember("cntBackup"))
		set.cntBackup = 600;
	else
		set.cntBackup = doc["cntBackup"].GetInt();

	if (!doc.HasMember("cntTemp"))
		set.cntTemp = 100;
	else
		set.cntTemp = doc["cntTemp"].GetInt();

	if (!doc.HasMember("azimut"))
		set.azimut = "0";
	else
		set.azimut = doc["azimut"].GetString();

	char tempTimeout[1024] = { 0 };
	sprintf(tempTimeout,"accept=%d connect=%d recv=%d send=%d transfer=%d cntBackup=%d cntTemp=%d", set.accept_timeout,set.connect_timeout,set.recv_timeout,set.send_timeout,set.transfer_timeout, set.cntBackup, set.cntTemp);
	addErrorLogMessage(tempTimeout);

	std::string str = doc["server"].GetString();

	strcpy(set.serverPath, str.c_str());

	set.camera_id = getIdDevice();// doc["reg_num"].GetString();

	set.start_date = doc["start_date"].GetString();

	set.region = doc["region"].GetString();

	set.c_type = getComplexType();
	
	addErrorLogMessage((char*)set.c_type.c_str());

	return true;
}
int COLSO::gettimeofday(timeval *tv, timezone *tz)
{
	FILETIME ft;
	unsigned __int64 tmpres = 0;
	static int tzflag;

	if (NULL != tv) {
		GetSystemTimeAsFileTime(&ft);

		tmpres |= ft.dwHighDateTime;
		tmpres <<= 32;
		tmpres |= ft.dwLowDateTime;

		/*converting file time to unix epoch*/
		tmpres /= 10;  /*convert into microseconds*/
		tmpres -= DELTA_EPOCH_IN_MICROSECS;
		tv->tv_sec = (long)(tmpres / 1000000UL);
		tv->tv_usec = (long)(tmpres % 1000000UL);
	}

	if (NULL != tz) {
		if (!tzflag) {
			_tzset();
			tzflag++;
		}
		tz->tz_minuteswest = _timezone / 60;
		tz->tz_dsttime = _daylight;
	}

	return 0;
}
std::string COLSO::getIdDevice()
{
	bool res;
	std::ifstream ifs("D:\\conf\\mconfig.json");
	rapidjson::IStreamWrapper isw(ifs);
	rapidjson::EncodedInputStream<rapidjson::UTF8<>, rapidjson::IStreamWrapper> eis(isw);
	rapidjson::Document doc;
	doc.ParseStream(eis);

	const auto name_it = doc.FindMember("olympus_mconfig");

	res = (name_it != doc.MemberEnd() && name_it->value.IsObject());
	if (!res) return "";

	bool kk = doc["olympus_mconfig"].HasMember("serial_number");

	if (kk)
	return doc["olympus_mconfig"]["serial_number"].GetString();
	else return "";
}
const char* COLSO::getComplexType()
{
	bool res;
	std::ifstream ifs("D:\\conf\\uconfig.json");
	rapidjson::IStreamWrapper isw(ifs);
	rapidjson::EncodedInputStream<rapidjson::UTF8<>, rapidjson::IStreamWrapper> eis(isw);
	rapidjson::Document doc;
	doc.ParseStream(eis);

	const auto name_it = doc.FindMember("olympus_uconfig");

	res = (name_it != doc.MemberEnd() && name_it->value.IsObject());
	if (!res) return "";

	bool kk = doc["olympus_uconfig"].HasMember("complex_type");

	if (kk)
		return doc["olympus_uconfig"]["complex_type"].GetString();
	else return "";
}
int COLSO::setPipeParams(int fixation_freq, int fixation_freq_send, int fixation_size_freq_send, int err_trans_count, int send_queue, int quality_connection)
{
	if (bPipeConnect)
	{

		//int fixation_freq = (double)((double)std::rand() / (double)RAND_MAX) * 50;
		//int fixation_freq_send = (double)((double)std::rand() / (double)RAND_MAX) * 50;
		//int fixation_size_freq_send = (double)((double)std::rand() / (double)RAND_MAX) * 500;
		//int err_trans_count = (double)((double)std::rand() / (double)RAND_MAX) * 100;
		//int send_queue = (int)((double)((double)std::rand() / (double)RAND_MAX) * 100);
		//int quality_connection = (double)((double)std::rand() / (double)RAND_MAX) * 10000;

		std::string str_fixation_freq = "fixation_freq:" + std::to_string(fixation_freq);//Скорость формирования фиксаций, шт в сек 
		std::string str_fixation_freq_send = "fixation_freq_send:" + std::to_string(fixation_freq_send);//Скорость отправки, шт в сек 
		std::string str_fixation_size_freq_send = "fixation_size_freq_send:" + std::to_string(fixation_size_freq_send);//Скорость отправки, кб в сек 
		std::string str_err_trans_count = "err_trans_count:" + std::to_string(err_trans_count);//Количество ошибок при отправке (проблемы между клиентом и сервером), в сутки 
		std::string str_send_queue = "send_queue:" + std::to_string(send_queue);//Очередь отправки  
		std::string str_quality_connection = "quality_connection:" + std::to_string(quality_connection);//Качество связи 

		//СМОТЕРЬТ НА ВОЗРАТ РЕЗУЛЬТАТА
		//И ДЕЛАТЬ РЕКОННЕКТ
		serv->send_work_stats(std::array<std::string, 6>{str_fixation_freq, str_fixation_freq_send, str_fixation_size_freq_send, str_err_trans_count, str_send_queue, str_quality_connection});
	}

	return 0;
}
int COLSO::getCoordAndPlace(char *coord, char *place)
{
	bool res;
	std::ifstream ifs("D:\\conf\\uconfig.json");
	rapidjson::IStreamWrapper isw(ifs);
	rapidjson::EncodedInputStream<rapidjson::UTF8<>, rapidjson::IStreamWrapper> eis(isw);
	rapidjson::Document doc;
	doc.ParseStream(eis);

	const auto name_it = doc.FindMember("olympus_uconfig");

	res = (name_it != doc.MemberEnd() && name_it->value.IsObject());
	if (!res)
	{
		coord = "";
		//workmode = "";

		return -1;
	}

	bool bEW = false;
	bool bNS = false;
	bool blong = false;
	bool blatit = false;

	bEW = doc["olympus_uconfig"].HasMember("place_EW");
	bNS = doc["olympus_uconfig"].HasMember("place_NS");
	blatit = doc["olympus_uconfig"].HasMember("place_latitude");
	blong = doc["olympus_uconfig"].HasMember("place_longitude");

	if (bEW && bNS && blatit && blong)
	{
		std::string res =
			std::string(doc["olympus_uconfig"]["place_NS"].GetString()) +
			std::to_string(doc["olympus_uconfig"]["place_latitude"].GetDouble()) + " " +
			std::string(doc["olympus_uconfig"]["place_EW"].GetString()) +
			std::to_string(doc["olympus_uconfig"]["place_longitude"].GetDouble());

		strcpy(coord, (char*)res.c_str());
	}
	else
		strcpy(coord, "");

	bool kk = doc["olympus_uconfig"].HasMember("place_location");
	if (kk)
	{
		wchar_t *kll = (wchar_t*)doc["olympus_uconfig"]["place_location"].GetString();

		//std::wstring gg = utf8_decode_3(kll);
		std::string gg2 = utf8_encode2(kll);
		int jj=0;
		//strcpy(place, (char*)gg.c_str());
	}
	else
		strcpy(place, "");

	return 0;
}
time_t COLSO::TimeFromSystemTime(const SYSTEMTIME * pTime)
{
	struct tm tm;
	memset(&tm, 0, sizeof(tm));

	tm.tm_year = pTime->wYear - 1900; // EDIT 2 : 1900 Offset as per comment
	tm.tm_mon = pTime->wMonth - 1;
	tm.tm_mday = pTime->wDay;

	tm.tm_hour = pTime->wHour;
	tm.tm_min = pTime->wMinute;
	tm.tm_sec = pTime->wSecond;
	tm.tm_isdst = -1; // Edit 2: Added as per comment

	return mktime(&tm);
}
int COLSO::dotCount(std::wstring str)
{
	int res = 0;

	std::wstring::size_type found = str.find_first_of(L".");
	while (found != std::wstring::npos)
	{
		res++;
		found = str.find_first_of(L".", found + 1);
		if (found != std::wstring::npos)
		{
			res++;
			break;
		}
	}

	return res;
}
int COLSO::getAviVideo(wchar_t *path)
{
	//addErrorLogMessage("getMkvVideo");
	int readFromFile = 0;
	memset(avi_video, 0, MAX_SIZE_MAIN_AVI);

	memset(video_wpath, 0, MAX_PATH);
	wcsncpy(video_wpath, path, wcslen(path) - 4);
	wcscat(video_wpath, L".avi");
	
	FILE *f = nullptr;
	f = _wfopen(video_wpath, L"rb");
	if (f == 0)
	{
		addErrorLogMessage("if (f == 0)");
		printf("[video_wpath]_wfopen error\r\n");
		return -1;
	}

	addErrorLogMessage((char*)utf8_encode(std::wstring(video_wpath)).c_str());

	struct _stat buffer;
	_wstat(video_wpath, &buffer);

	if (buffer.st_size < MAX_SIZE_MAIN_AVI)
		readFromFile = fread((void*)avi_video, sizeof(char), buffer.st_size, f);
	else
	{
		addErrorLogMessage("if (buffer.st_size < MAX_SIZE_MAIN_BMP)");
		printf("[video_wpath]buffer.st_size < MAX_SIZE_MAIN_BMP[%d]\r\n", buffer.st_size);
		fclose(f);
		return -1;
	}

	if (avi_video[0] == 0)
	{
		addErrorLogMessage("avi_video[0] == 0");
		fclose(f);
		return -1;
	}

	if (buffer.st_size != readFromFile)
	{
		addErrorLogMessage("if (buffer.st_size != readFromFile)");
		printf("[video_wpath]buffer.st_size != readFromFile[%d != %d]\r\n", buffer.st_size, readFromFile);
		fclose(f);
		return -1;
	}
	fclose(f);

	return readFromFile;
}
int COLSO::getMkvVideo(wchar_t *path)
{
	//addErrorLogMessage("getMkvVideo");
	int readFromFile = 0;
	memset(mkv_video, 0, MAX_SIZE_MAIN_BMP);

	memset(video_wpath, 0, MAX_PATH);
	wcsncpy(video_wpath, path, wcslen(path) - 4);
	wcscat(video_wpath, L"_video.mkv");
	addErrorLogMessage((char*)utf8_encode(std::wstring(video_wpath)).c_str());

	FILE *f = nullptr;
	f = _wfopen(video_wpath, L"rb");
	if (f == 0)
	{
		addErrorLogMessage("if (f == 0)");
		printf("[video_wpath]_wfopen error\r\n");
		return -1;
	}

	struct _stat buffer;
	_wstat(video_wpath, &buffer);

	if (buffer.st_size < MAX_SIZE_MAIN_BMP)
		readFromFile = fread((void*)mkv_video, sizeof(char), buffer.st_size, f);
	else
	{
		addErrorLogMessage("if (buffer.st_size < MAX_SIZE_MAIN_BMP)");
		printf("[video_wpath]buffer.st_size < MAX_SIZE_MAIN_BMP[%d]\r\n", buffer.st_size);
		fclose(f);
		return -1;
	}

	if (mkv_video[0] == 0)
	{
		addErrorLogMessage("mkv_video[0] == 0");
		fclose(f);
		return -1;
	}

	if (buffer.st_size != readFromFile)
	{
		addErrorLogMessage("if (buffer.st_size != readFromFile)");
		printf("[video_wpath]buffer.st_size != readFromFile[%d != %d]\r\n", buffer.st_size, readFromFile);
		fclose(f);
		return -1;
	}
	fclose(f);

	return readFromFile;
}

int COLSO::errMySqlShutdowner()
{
	nCountMySqlError++;
	if (nCountMySqlError > 10)
	{
		addErrorLogMessage("mysql shutdown");
		errMySqlZero();
		quick_exit(1);
	}

	return nCountMySqlError;
}

void COLSO::errMySqlZero()
{
	nCountMySqlError = 0;
}

//int save_settings()
//{
//	std::wstring path = L"c:\\soap.json";
//
//	rapidjson::Document doc;
//	doc.SetObject();
//
//	rapidjson::Value conf_num(rapidjson::kStringType);
//	rapidjson::Value conf_id(rapidjson::kNumberType);
//
//	conf_num.SetString("15060199", doc.GetAllocator());
//	conf_id.SetInt(37);
//
//	//for (auto par : parameters)
//	//{
//	//	rapidjson::Value val;
//	//	if (par.second->setgetjson(false, &val, &doc))
//	//	{
//	//		rapidjson::Value vMemberName(rapidjson::kStringType);
//	//		vMemberName.SetString(par.first.c_str(), doc.GetAllocator());
//	//		olympus_configuration.AddMember(vMemberName, val, doc.GetAllocator());
//	//	}
//	//}
//	//doc.AddMember(rapidjson::StringRef(this->name.c_str()), olympus_configuration, doc.GetAllocator());
//
//	std::ofstream ofs(path);
//	rapidjson::OStreamWrapper osw(ofs);
//	rapidjson::PrettyWriter<rapidjson::OStreamWrapper> writer(osw);
//	doc.Accept(writer);
//	ofs.close();
//
//	//rapidjson::StringBuffer buffer;
//	//rapidjson::Writer<rapidjson::StringBuffer> swriter(buffer);
//	//doc.Accept(swriter);
//	//MD5 md5;
//	//std::ofstream(path + L".md5") << md5(buffer.GetString());
//
//	return 0;
//}