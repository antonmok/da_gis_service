#include <cstdio>
#include <sys/ipc.h>
#include <sys/time.h>
#include <sys/shm.h>
#include <iostream>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <pthread.h>
//#include <synch.h>

#include <fcntl.h> 
#include <stdio.h>
#include <thread>

#include <string>
#include <ctime>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <time.h>

#include "../salmon/common/cmn.h"

#include <boost/filesystem.hpp>
#include <boost/chrono.hpp>
#include <boost/ratio.hpp>
#include <boost/chrono.hpp>
#include <boost/thread/thread.hpp> 

#include <experimental/filesystem>

#include <libwebsockets.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <string.h>
#include <signal.h>

#include <jpeglib.h>
#include "rapidjson/rapidjson.h"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/istreamwrapper.h"
#include "rapidjson/ostreamwrapper.h"
#include "rapidjson/prettywriter.h"

#include "base64.h"

#include "sys/stat.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgproc/imgproc_c.h>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/features2d.hpp>

#define BUFFRECVSIZE 1024

#define LWS_MESSAGE 4096
#define LWS_RECV_MESSAGE 4096




#define SIFT_IS_NOT_FINISHED -1// задержка распознавнаия символов
#define SIFT_IS_IMPOSSIBLE 99999// СИФТ НИЧЕГ НЕ НАШЕЛ

using namespace std;
using namespace cv;

int mutexFramesInit();
int mutexIdsInit();
uint64_t getTimeMcs();

shm_mutex_array *img_buffer = nullptr;
shm_ids_mutex   *ids_buffer = nullptr;
unsigned int g_nCountSalmon = 0;
//uint64_t _lastTime = 0;
uint64_t _lastImage = 0;
int fps_cap_cnt = 0;
int fps_cnt = 0;

enum MODE_VERIFY {
    VERIFY_TIME,
    VERIFY_COORD,
    VERIFY_DIST,
    MODE_TIME
};

struct nmea
{
    time_t xtime;
    time_t time;
    uint64_t frame;
    double speed;
    double lon;
    double lat;
    double lat_disp;
    double lon_disp;
};

int addNmeaLog(std::string path);

std::vector<nmea> nmeaLog;
nmea getField(std::string str);
double getCalcTime(char* time_date);
double getCalcTimeNs(char* time_date);
uint64_t getTimeMcsFromArray(int file_id);

enum class VRF_CMD{
    vrf_null,
    vrf_setBrightness,
    vrf_setMode,
    vrf_setSettings
};

void thrVerifyServer();

int sct_vrf, sct_vrf_new;
void reacceptVerify();
int accept_connectionVerify();

int recvVerifyMessage(VRF_CMD *cmd,std::string *params);
VRF_CMD recvVerifyParsing(int size,std::string *params);

int sendVerifyAnswer(VRF_CMD cmd,std::string params);
int prepareVerifyAnswer(VRF_CMD cmd,std::string params);

char bufRecvVrf[BUFFRECVSIZE];

lws_context *lwscontext = nullptr;
lws *web_socket = nullptr;
lws_client_connect_info ccinfo = { 0 };
unsigned char *lws_message = new unsigned char[LWS_MESSAGE];
unsigned char *lws_recv_message = new unsigned char[LWS_RECV_MESSAGE];

__time_t lws_old;
	lws_context_creation_info lws_cinfo;

int lwsInit();
int lwsMain();

bool parse_settings();

void jpegInit();
int jpegProc(unsigned char *outbuf, int allSize, unsigned char *image_buffer);
void jpegRelease();

struct jpeg_compress_struct cinfo;
struct jpeg_error_mgr jerr;

int ws_callback(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len);

lws_protocols protocols[] =
{
	{
		"",				/* name */
		ws_callback,		/* callback */
		0,0
	},
	{
		nullptr, nullptr, 0,0	/* End of list */
	}
};

// static const struct lws_protocol_vhost_options pvo_opt = {
//         NULL,
//         NULL,
//         "default",
//         "1"
// };
// static const struct lws_protocol_vhost_options pvo = {
//         NULL,
//         &pvo_opt,
//         "my-protocol",
//         ""
// };

struct msg 
{
	void *payload; /* is malloc'd */
	size_t len;
};

void __minimal_destroy_message(msg *_msg);
int callback_minimal(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len);

struct per_session_data__minimal 
{
	struct per_session_data__minimal *pss_list;
	struct lws *wsi;
	int last; /* the last message number we sent */
};

struct per_vhost_data__minimal 
{
	struct lws_context *context;
	struct lws_vhost *vhost;
	const struct lws_protocols *protocol;

	struct per_session_data__minimal *pss_list; /* linked-list of live pss*/

	struct msg amsg; /* the one pending message... */
	int current; /* the current message number we are caching */
};

// static struct lws_protocols protocols[] = {
// 	{ "http", lws_callback_http_dummy, 0, 0 },
// 	{ "lws-minimal", callback_minimal, sizeof(struct per_session_data__minimal), 128 , 0 , nullptr , 0 },
// 	{ NULL, NULL, 0, 0 } /* terminator */
// };

static int interrupted;

void sigint_handler(int sig)
{
	interrupted = 1;
}

using namespace rapidjson;

unsigned char left_jpg[1024*500];        
unsigned char right_jpg[1024*500];        

unsigned char left_b64[1024*700];        
unsigned char right_b64[1024*700];        

unsigned char send_json[1024*1500];        

int convertToBase64(unsigned char *buf_out,unsigned char *buf_in,int data_length);

int save_settings();
void readAndCompress(MODE_VERIFY mode);
int readPartFrame(box bx,void *pic);

struct srv_to_cl
{
    std::string lat;
    std::string lon;
    uint64_t    time;
    double      dist;
    unsigned int brightness_gain;
    unsigned int brightness_shutter;
    unsigned int brightness_gamma;
};

srv_to_cl sendToClient;
int readFrame(uint64_t *id,uint64_t *time,double *lat,double *lon);

pthread_mutex_t ipc_mutex[SALMON_SIZE] = {0};
pthread_mutex_t ids_mutex = {0};

//shm_mutex_array *img_buffer = nullptr;
//shm_ids_mutex *ids_buffer = nullptr;

uint64_t _lastTime = 0;
uint64_t _last_index = 0;

void *imgsum = nullptr;

void *dst_jpg_mem1;
void *dst_jpg_mem2;

int init_pipe_rd(char *fdn);

pthread_mutex_t gps_mutex;
gns_actualy gps_from_pipe = {0};

void savePassage(char *left_fact,char *right_fact,uint64_t ttime,char *number,cv::Mat *retLeft,cv::Mat *retRight);

struct sf_match
{
  double dist;
  int index;
};

double calcsift(box left,box right,int un,uint64_t id);
bool siftSort(sf_match a,sf_match b){return a.dist < b.dist;};


double calcClassicZ(double pxDiff,double dl,double dr,double cpx);
double calcDist(double pxDiff1);

int main(int argc, char* argv[])
{


    /*std::thread t_gns = std::thread([&]()
    {
        gps_mutex = PTHREAD_MUTEX_INITIALIZER;
        int res = pthread_mutex_init(&gps_mutex,nullptr);
        gns_actualy wrt_s2b;
    
        int fd_salmon2blabber = init_pipe_rd(PIPE_SALMON_TO_BLABBER);
        do
        {
            do
            {
                ssize_t read_size = 0;

                if(fd_salmon2blabber > 0)
                {
                    read_size = read(fd_salmon2blabber,&wrt_s2b,sizeof(gns_actualy));

                    if(read_size < 0)
                    {
                        printf("read error %d\r\n",errno);
                        sleep(2);
                        continue;
                    }                                
                    
                    if(read_size == sizeof(gns_actualy))
                    {
                        //coord verify, get time and coord from pipe
                        while(true)
                        {
                            if(pthread_mutex_trylock(&gps_mutex) == 0)
                            {
                                gps_from_pipe = wrt_s2b;
                                printf("time = %lld\r\n",wrt_s2b.time);
                                pthread_mutex_unlock(&gps_mutex);
                                break;
                            }
                            else 
                                printf("t_gns: gps mutex busy!!\r\n");
                        }
                    }
                }

                if(fd_salmon2blabber < 0)
                {
                    printf("init pipe error %d\r\n",errno);
                    usleep(10000);
                    fd_salmon2blabber = init_pipe_rd(PIPE_SALMON_TO_BLABBER);
                    continue;
                }

                if(read_size >= 0)
                {
                    usleep(10000);
                    continue;
                }

            } while(fd_salmon2blabber < 0);

            usleep(10000);

        } while(true);

        close(fd_salmon2blabber);
        std::cout << " t_gns exit \n";
    });*/

    std::thread *thr2 = new std::thread(&thrVerifyServer);

    // if (thr->joinable()) 
    // thr->join();

    while(true)
    {
        sleep(1);
    };

    int res = 0;

    return 0;
}
int mutexFramesInit()
{
    int shm_id = shm_open(SALMON_PATH,O_RDONLY,S_IRUSR | S_IWUSR);

    if(shm_id < 0)
        {printf("mutexFramesInit: shm_open[%d]\r\n",errno);return -1;}

    img_buffer = (shm_mutex_array*)mmap(nullptr,sizeof(shm_mutex)*SALMON_SIZE,PROT_READ,MAP_SHARED,shm_id,0);
    if(img_buffer == MAP_FAILED)
        {printf("mutexFramesInit: mmap\r\n");return -1;}

    for(int i = 0;i < SALMON_SIZE;i++)
        ipc_mutex[i] = img_buffer->frame[i].ipc_mutex;

    return 0; 
}
int mutexIdsInit()
{
    int shm_id = shm_open(SALMON_IDS_PATH,O_RDONLY,S_IRUSR | S_IWUSR);

    if(shm_id < 0)
        {printf("mutexIdsInit: shm_open[%d]\r\n",errno);return -1;}

    ids_buffer = (shm_ids_mutex*)mmap(nullptr,sizeof(shm_ids_mutex),PROT_READ,MAP_SHARED,shm_id,0);
    if(ids_buffer == MAP_FAILED)
        {printf("mutexIdsInit: mmap\r\n");return -1;}

    ids_mutex = ids_buffer->ipc_mutex;

    return 0; 
}
uint64_t getTimeMcs()
{
    struct timeval mtime;
    int64_t fulltime;

    gettimeofday(&mtime,nullptr);
    fulltime = mtime.tv_sec*1000000 + mtime.tv_usec;

    time_t sec = fulltime/1000000;
    uint64_t mcssec = fulltime - sec*1000000;

    std::tm* tt = gmtime(&sec);
    //printf("[%d] %d-%02d-%02d %02d:%02d:%02d.%06d\r\n",g_nCountSalmon,tt->tm_year + 1900,tt->tm_mon + 1,tt->tm_mday,tt->tm_hour,tt->tm_min,tt->tm_sec,mcssec);

    return fulltime; 
}
uint64_t getTimeMcsFromArray(int file_id)
{
    int64_t fulltime;
    for(std::vector<nmea>::iterator it = nmeaLog.begin();it != nmeaLog.end();it++)
    {
        if(it->frame == file_id)
        {
            fulltime = it->xtime;
            break;
        }
    }

    return fulltime;
}
int addNmeaLog(std::string path)
{
    path.append("/gps-balka-stat.log");
    std::string line;

    ifstream logfile(path.c_str());
    if(logfile.is_open())
    {
        while(std::getline(logfile,line))
        {
            if(!line.empty())
            {
                nmea data;
                data = getField(line);
                nmeaLog.push_back(data);
            }
        }
        logfile.close();
        printf("size nmea log = %d\r\n",(int)nmeaLog.size());
    }

    return 0;
}
nmea getField(std::string str)
{
    nmea data = {0};
    
    char strtemp[20] = {0};

    std::string::size_type sz;
    int k = 0;
    char *token = std::strtok((char*)str.c_str(), " ");
    while (token != NULL) 
    {
        if(k == 0)//frame
        {
            //sscanf(token,"%s%d",strtemp,&data.frame);
            data.frame = atoi(token+6);
        }

        if(k == 1)//speed
        {
            //sscanf(token,"speed=%.2f ",&data.speed);
            data.speed = atof(token+6);
        }

        if(k == 2)//time
        {
            data.time = getCalcTime(token+5);
        }

        if(k == 3)//xtime
        {
            data.xtime = getCalcTimeNs(token+6);
        }

        if(k == 4)//lat + disp
        {
            //int lat1,lat2,latd1,latd2;
            //sscanf(token,"lat=%d.%d[%d.%d] ",&lat1,&lat2,&latd1,&latd2);
            data.lat=60.0;
        }

        if(k == 5)//lon + disp
        {
            int lon1,lon2,lond1,lond2;
            sscanf(token,"lon=%d.%d[%d.%d]",&lon1,&lon2,&lond1,&lond2);
            data.lon=30.0;
        }

        k++;
        token = std::strtok(NULL, " ");
    }

    return data;

}
double getCalcTime(char* time_date)
{
    //%d-%d-%d_%d:%d:%d.%d ",&year,&month,&day,&hour,&min,&sec,&mcsec

    char day[2] = {0};
    char month[2] = {0};
    char year[4] = {0};

    char hour[2] = {0};
    char min[2] = {0};
    char sec[2] = {0};
    char msec[3] = {0};

    memcpy(year,time_date,4);
    memcpy(month,time_date+5,2);
    memcpy(day,time_date+8,2);

    memcpy(hour,time_date+11,2);
    memcpy(min,time_date+14,2);
    memcpy(sec,time_date+17,2);
    memcpy(msec,time_date+20,3);


    //std::time_t t = std::time(0);   // get time now
    std::tm now = {0};// = std::localtime(&t);

    now.tm_year = stoi(std::string(year))-1900;
    now.tm_mon = stoi(std::string(month))-1;
    now.tm_mday = stoi(std::string(day));

    now.tm_hour = stoi(std::string(hour));
    now.tm_min = stoi(std::string(min));
    now.tm_sec = stoi(std::string(sec));

    time_t tt = mktime(&now);
    //tt += 3600*3;
    tt = tt *1000;
    tt += stoi(std::string(msec));
    //*ms = stoi(msec)*10;
    // tt += ms;
    tt *= 1000;

    return tt;
}
double getCalcTimeNs(char* time_date)
{
    //%d-%d-%d_%d:%d:%d.%d ",&year,&month,&day,&hour,&min,&sec,&mcsec

    char day[2] = {0};
    char month[2] = {0};
    char year[4] = {0};

    char hour[2] = {0};
    char min[2] = {0};
    char sec[2] = {0};
    char nsec[9] = {0};

    memcpy(year,time_date,4);
    memcpy(month,time_date+5,2);
    memcpy(day,time_date+8,2);

    memcpy(hour,time_date+11,2);
    memcpy(min,time_date+14,2);
    memcpy(sec,time_date+17,2);
    memcpy(nsec,time_date+20,9);


    //std::time_t t = std::time(0);   // get time now
    std::tm now = {0};// = std::localtime(&t);

    now.tm_year = stoi(std::string(year))-1900;
    now.tm_mon = stoi(std::string(month))-1;
    now.tm_mday = stoi(std::string(day));

    now.tm_hour = stoi(std::string(hour));
    now.tm_min = stoi(std::string(min));
    now.tm_sec = stoi(std::string(sec));

    time_t tt = mktime(&now);
    //tt += 3600*3;
    tt = tt *1000000;
    int nansec = stoi(std::string(nsec))/1000;
    tt += nansec;
    //*ms = stoi(msec)*10;
    // tt += ms;
    //tt *= 1000;

    return tt;
}

void thrVerifyServer()
{
    jpegInit();

    imgsum = malloc(IMAGE_HEIGHT*2*IMAGE_WIDTH);
    dst_jpg_mem1 = malloc(IMAGE_HEIGHT*IMAGE_WIDTH);
    dst_jpg_mem2 = malloc(IMAGE_HEIGHT*IMAGE_WIDTH);

    //init mutex & init shm
    mutexIdsInit();
    mutexFramesInit();

    bool work = true;
    VRF_CMD cmd = VRF_CMD::vrf_null;
    std::string params = "";

  	// struct stat left_st;
	// int res = stat("/mnt/m2/blabber/left.jpg", &left_st);

    // FILE *fl = fopen("/mnt/m2/blabber/left.jpg","rb");
    // fread(left_jpg,sizeof(char),left_st.st_size,fl);
    // fclose(fl); 

  	// struct stat right_st;
	// res = stat("/mnt/m2/blabber/right.jpg", &right_st);

    // FILE *fr = fopen("/mnt/m2/blabber/right.jpg","rb");
    // fread(right_jpg,sizeof(char),right_st.st_size,fr);
    // fclose(fr); 

    //int leftRes = convertToBase64(left_b64,left_jpg,left_st.st_size);
    //int rightRes = convertToBase64(right_b64,right_jpg,right_st.st_size);

    accept_connectionVerify();

    //GET FROM RECEIVE
    MODE_VERIFY mode;
    mode = MODE_VERIFY::MODE_TIME;

	while (work)
	{
        readAndCompress(mode);
        if(sendVerifyAnswer(cmd,params) < 0)
        {
             printf("Error: sendAnswer\r\n");
		     reacceptVerify();
        }

		// if (recvVerifyMessage(&cmd,&params) > 0)
		// {
        //     if(sendVerifyAnswer(cmd,params) < 0)
        //     {
        //         printf("Error: sendAnswer\r\n");
		// 	    reacceptVerify();
        //     }
		// }
        // else
		// { 
		// 	printf("Error: recvMessage\r\n");
		// 	reacceptVerify();
		// }

		usleep(200000);
	}
}
void reacceptVerify()
{
    printf("reaccept\r\n");
	if(sct_vrf_new > 0)close(sct_vrf_new);
	if(sct_vrf > 0)close(sct_vrf);
	accept_connectionVerify();
}
int accept_connectionVerify()
{
    int portno;
    portno = 8090;
    socklen_t clilen;

    struct sockaddr_in serv_addr, cli_addr;

    sct_vrf = socket(AF_INET, SOCK_STREAM, 0);
    if (sct_vrf < 0) 
    {
        perror("ERROR opening socket");
        return -1;
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(portno);

    if (bind(sct_vrf, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
    {
        perror("ERROR on binding");
        system("fuser -k 8082/tcp");//fuser -k 8082/tcp
        sleep(2);
        return -2;
    }

    int res = listen(sct_vrf,5);
    if (res < 0) 
    {
        perror("ERROR on listen");
        return -4;
    }

    clilen = sizeof(cli_addr);
    sct_vrf_new = accept(sct_vrf, (struct sockaddr *) &cli_addr, &clilen);

    if (sct_vrf_new < 0) 
    {
        perror("ERROR on accept");
        return -3;
    }

	return 0;
}
int recvVerifyMessage(VRF_CMD *cmd,std::string *params)
{
    bzero(bufRecvVrf,BUFFRECVSIZE);
    int n = recv(sct_vrf_new,bufRecvVrf,BUFFRECVSIZE,0);

    if (n < 0) 
    {
        perror("ERROR reading from socket");
        return -1;
    }

    printf("Here is the message:\r\n%s\r\n",bufRecvVrf);

    *cmd = recvVerifyParsing(n,params);

    return n;
}
VRF_CMD recvVerifyParsing(int size,std::string *params)
{
    bufRecvVrf;
    *params = "name=name&";
    return VRF_CMD::vrf_setMode;
}
int prepareVerifyAnswer(VRF_CMD cmd,std::string params)
{

    /* client to server

        {
            "mode": 
            {
                "type": "time",      //time,gns,dist,calibr
                "state": "ON",       //ON,OFF
                "videoState": "ON",  //ON,OFF
                "videoQ": 30         //10-100
            },
            "coord":
            {
                "XL": 5,
                "YL": 5,
                "WL": 5,
                "HL": 5,
                "XR": 5,
                "YR": 5,
                "WR": 5,
                "HR": 5      
            },
            "brightness":
            {
                "gain": 30,
                "shutter": 60,
                "gamma": 5
            }
        }

        struct cl_to_srv
        {
            srd::string mode_type;      //time,gns,dist,calibr
            srd::string mode_state;     //ON,OFF
            srd::string mode_videoState;//ON,OFF
            unsigned int mode_videoQ;   //10-100

            unsigned int coord_XL;   //0-1920
            unsigned int coord_YL;   //0-1920
            unsigned int coord_WL;   //0-1920
            unsigned int coord_HL;   //0-1920
            unsigned int coord_XR;   //0-1920
            unsigned int coord_YR;   //0-1920
            unsigned int coord_WR;   //0-1920
            unsigned int coord_HR;   //0-1920

            unsigned int brightness_gain;   //0-1920
            unsigned int brightness_shutter;   //0-1920
            unsigned int brightness_gamma;   //0-1920
        }
    */

    std::string strJson;
    strJson.append("[\r");
    strJson.append("{\r");
            strJson.append("\"videoL\":\"");strJson.append((char*)left_b64);strJson.append("\",\r");
            strJson.append("\"videoR\":\"");strJson.append((char*)right_b64);strJson.append("\",\r");
            //strJson.append("\"videoL\":\"");strJson.append("qwerty");strJson.append("\",\r");
            //strJson.append("\"videoR\":\"");strJson.append("qwerty");strJson.append("\",\r");
            strJson.append("\"lat\":\"");strJson.append(sendToClient.lat);strJson.append("\",\r");
            strJson.append("\"lon\":\"");strJson.append(sendToClient.lon);strJson.append("\",\r");
            strJson.append("\"time\":");strJson.append(std::to_string(sendToClient.time));strJson.append(",\r");
            strJson.append("\"dist\":");strJson.append(std::to_string(sendToClient.dist));strJson.append(",\r");
            strJson.append("\"brightness\":\r");
            strJson.append("[\r");
            strJson.append("{\r");
            strJson.append("\"gain\":");strJson.append(std::to_string(sendToClient.brightness_gain));strJson.append(",\r");
            strJson.append("\"shutter\":");strJson.append(std::to_string(sendToClient.brightness_shutter));strJson.append(",\r");
            strJson.append("\"gamma\":");strJson.append(std::to_string(sendToClient.brightness_gamma));strJson.append("\r");
            strJson.append("}\r");
            strJson.append("]\r");
    strJson.append("}\r");
    strJson.append("]");

    int size = strJson.size();
    memcpy((void*)send_json,(void*)&size,sizeof(int));
    memcpy((void*)(send_json+sizeof(int)),(void*)(strJson.c_str()),size);

    return size+sizeof(int);

}
int sendVerifyAnswer(VRF_CMD cmd,std::string params)
{
    int answer_size = prepareVerifyAnswer(cmd,params);

    int n = 0;

    try
    {
        int err_code = 0;
        socklen_t err_code_sz = 0;
        getsockopt(sct_vrf_new,SOL_SOCKET,SO_ERROR,&err_code,&err_code_sz);
        printf("err_code1 = %d\r\n",err_code);
        if(err_code == 0)
            n = send(sct_vrf_new,send_json,answer_size,0);
        else
        {
            perror("ERROR writing to socket");
            return -2;
        }

        getsockopt(sct_vrf_new,SOL_SOCKET,SO_ERROR,&err_code,&err_code_sz);
        printf("err_code2 = %d\r\n",err_code);
        if(err_code != 0)
        {
            perror("ERROR writing to socket");
            return -4;
        }
    }
    catch(...)
    {
        //std::cerr << e.what() << '\n';
        printf("send ERROR!\r\n");
        return -3;
    }    

    printf("send = %d\r\n",n);

    if (n <= 0) 
    {
        perror("ERROR writing to socket");
        return -1;
    }

    return n;
}


int lwsInit()
{
	//memset(&info, 0, sizeof(lws_context_creation_info));
	lws_cinfo.port = 8083;//CONTEXT_PORT_NO_LISTEN;
	lws_cinfo.protocols = protocols;
	lws_cinfo.gid = -1;
	lws_cinfo.uid = -1;
    //lws_cinfo.pvo = &pvo;
    lws_cinfo.server_string = "tandem";
    lws_cinfo.vhost_name = "localhost";

    //lws_cinfo.timeout_secs = 5;
    //lws_cinfo.timeout_secs_ah_idle = 0;
    //info.connect_timeout_secs;
    //lws_cinfo.ka_interval = 1;
    //lws_cinfo.ka_probes = 5;
    //lws_cinfo.ka_time = 20;
    //lws_cinfo.keepalive_timeout = 20;
    //lws_cinfo.log_filepath = "/mnt/m2/blabber/log.log";
    //lws_cinfo.pt_serv_buf_size = 32*1024;

    //lws_cinfo.options = LWS_SERVER_OPTION_HTTP_HEADERS_SECURITY_BEST_PRACTICES_ENFORCE;
    //lws_cinfo.options |= LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
    lws_cinfo.options = 0;

    lws_cinfo.ssl_cipher_list = nullptr;

    //lws_cinfo.ssl_ca_filepath = "/mnt/m2/blabber/crt/servercert.pem";           //cca-cert.pem
    //lws_cinfo.ssl_cert_filepath = "/mnt/m2/blabber/crt-end/rootCA.crt";       //servercert.pem
    //lws_cinfo.ssl_private_key_filepath = "/mnt/m2/blabber/crt-end/rootCA.key"; //serverkey.pem

	lwscontext = lws_create_context(&lws_cinfo);

    //lws_cinfo.ssl_ca_filepath = "/mnt/m2/blabber/crt/servercert.pem";           //cca-cert.pem
    //lws_cinfo.ssl_cert_filepath = "/mnt/m2/blabber/crt-end/rootCA.crt";       //servercert.pem
    //lws_cinfo.ssl_private_key_filepath = "/mnt/m2/blabber/crt-end/rootCA.key"; //serverkey.pem

	return 1;
}
int lwsMain()
{
   	// timeval lws_tv;
	// gettimeofday(&lws_tv, NULL);
	// if (!web_socket && lws_tv.tv_sec != lws_old)
	// {
	// 	memset(&ccinfo, 0, sizeof(lws_client_connect_info));
	// 	ccinfo.context = lwscontext;
	// 	ccinfo.address = "localhost";
	// 	ccinfo.host = lws_canonical_hostname(lwscontext);
	// 	ccinfo.port = 8083;
	// 	ccinfo.path = "/";
	// 	ccinfo.protocol = protocols[0].name;
    //     ccinfo.ssl_connection = 1;
	// 	web_socket = lws_client_connect_via_info(&ccinfo);
	// }
	//  if (lws_tv.tv_sec != lws_old)
	//  {
	// // 	lws_callback_on_writable(web_socket);
	//  	lws_old = lws_tv.tv_sec;
	//  }

	int res = lws_service(lwscontext, 0);

	return 1;
}
// //callback_dumb_increment
int ws_callback(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
{
    int res = 0;
	switch (reason)
	{
        case LWS_CALLBACK_HTTP:
        {
            printf("LWS_CALLBACK_HTTP\r\n");
            //lws_callback_on_writable(wsi);
            break;
        }
        case LWS_CALLBACK_CLIENT_ESTABLISHED:
        {
            printf("LWS_CALLBACK_CLIENT_ESTABLISHED\r\n");
            //!lws_callback_on_writable(wsi);
            break;
        }
        case LWS_CALLBACK_CLIENT_RECEIVE:
        {
            printf("LWS_CALLBACK_CLIENT_RECEIVE\r\n");
            break;
        }
        case LWS_CALLBACK_HTTP_CONFIRM_UPGRADE:
        {
            //printf("LWS_CALLBACK_HTTP_CONFIRM_UPGRADE\r\n");
            break;
        }
        case LWS_CALLBACK_ADD_HEADERS:
        {
            //printf("LWS_CALLBACK_ADD_HEADERS\r\n");
            break;
        }
        case LWS_CALLBACK_ESTABLISHED:
        {
            printf("LWS_CALLBACK_ESTABLISHED\r\n");
            //lws_callback_on_writable(wsi);//wsi
            break;
        }
        case LWS_CALLBACK_ESTABLISHED_CLIENT_HTTP:
        {
            //printf("LWS_CALLBACK_ESTABLISHED_CLIENT_HTTP\r\n");
            break;
        }
        case LWS_CALLBACK_CLIENT_WRITEABLE:
        {
            printf("LWS_CALLBACK_CLIENT_WRITEABLE\r\n");
           	memset(lws_message, 0, LWS_MESSAGE);
           	int res = sprintf((char*)lws_message, "{\"type\" : \"test\", \"first\" : \"первый\",\"second\" : \"второй\"}");
           	int nbb = lws_write(wsi, lws_message, res, LWS_WRITE_TEXT);
            break;
        }
        case LWS_CALLBACK_CLOSED:
        {
            web_socket = nullptr;
            break;
        }
        case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
        {
            printf("LWS_CALLBACK_CLIENT_CONNECTION_ERROR\r\n");
            printf("[%d]%s\r\n",len,in);
            web_socket = nullptr;
            break;
        }
        case LWS_CALLBACK_PROTOCOL_INIT:
        {
            //printf("LWS_CALLBACK_PROTOCOL_INIT\r\n");
            break;
        }
        case LWS_CALLBACK_CLIENT_HTTP_BIND_PROTOCOL:
        {
            //printf("LWS_CALLBACK_CLIENT_HTTP_BIND_PROTOCOL\r\n");
            break;
        }
        case LWS_CALLBACK_GET_THREAD_ID:
        {
            //printf("LWS_CALLBACK_GET_THREAD_ID\r\n");
            break;
        }
        case LWS_CALLBACK_EVENT_WAIT_CANCELLED:
        {
            //printf("LWS_CALLBACK_EVENT_WAIT_CANCELLED\r\n");
            break;
        }
        case LWS_CALLBACK_CLIENT_HTTP_DROP_PROTOCOL:
        {
            printf("LWS_CALLBACK_CLIENT_HTTP_DROP_PROTOCOL\r\n");
            break;
        }
        case LWS_CALLBACK_WSI_DESTROY:
        {
            printf("LWS_CALLBACK_WSI_DESTROY\r\n");
            web_socket = nullptr;
            break;
        }
        case LWS_CALLBACK_FILTER_NETWORK_CONNECTION:
        {
            //printf("LWS_CALLBACK_FILTER_NETWORK_CONNECTION\r\n");
            break;
        }
        case LWS_CALLBACK_WSI_CREATE:
        {
            //printf("LWS_CALLBACK_WSI_CREATE\r\n");
            break;
        }
        case LWS_CALLBACK_CLOSED_HTTP:
        {
            printf("LWS_CALLBACK_CLOSED_HTTP\r\n");
            break;
        }
        case LWS_CALLBACK_FILTER_HTTP_CONNECTION:
        {
            printf("LWS_CALLBACK_FILTER_HTTP_CONNECTION\r\n");
            break;
        }
        case LWS_CALLBACK_HTTP_BIND_PROTOCOL:
        {
            //printf("LWS_CALLBACK_HTTP_BIND_PROTOCOL\r\n");
            break;
        }
        case LWS_CALLBACK_SERVER_NEW_CLIENT_INSTANTIATED:
        {
            printf("LWS_CALLBACK_SERVER_NEW_CLIENT_INSTANTIATED\r\n");
            //res = lws_callback_on_writable(wsi);//wsi

            break;
        }
        case LWS_CALLBACK_HTTP_WRITEABLE:
        {
            printf("LWS_CALLBACK_HTTP_WRITEABLE\r\n");
            break;
        }
        case LWS_CALLBACK_RECEIVE:
        {
            //printf("LWS_CALLBACK_RECEIVE\r\n");
            memset(lws_recv_message,0,LWS_RECV_MESSAGE);
            memcpy(lws_recv_message,in,len);
            printf("receive = %s\r\n",lws_recv_message);
            break;
        }
        default:
            break;
	}
    //if(len != 0)
    //printf("reason = %d[%d]\r\n",reason,len);
	return res;
}
void __minimal_destroy_message(msg *_msg)
{
	struct msg *__msg = _msg;

	free(__msg->payload);
	__msg->payload = NULL;
	__msg->len = 0;
}
int callback_minimal(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
{
	struct per_session_data__minimal *pss =	(struct per_session_data__minimal *)user;
	struct per_vhost_data__minimal *vhd = (struct per_vhost_data__minimal *)lws_protocol_vh_priv_get(lws_get_vhost(wsi),lws_get_protocol(wsi));
	int m;

	switch (reason) {
	case LWS_CALLBACK_PROTOCOL_INIT:
		vhd = (per_vhost_data__minimal *)lws_protocol_vh_priv_zalloc(lws_get_vhost(wsi),lws_get_protocol(wsi),sizeof(struct per_vhost_data__minimal));
		vhd->context = lws_get_context(wsi);
		vhd->protocol = lws_get_protocol(wsi);
		vhd->vhost = lws_get_vhost(wsi);
		break;

	case LWS_CALLBACK_ESTABLISHED:
		/* add ourselves to the list of live pss held in the vhd */
		lws_ll_fwd_insert(pss, pss_list, vhd->pss_list);
		pss->wsi = wsi;
		pss->last = vhd->current;
		break;

	case LWS_CALLBACK_CLOSED:
		/* remove our closing pss from the list of live pss */
		lws_ll_fwd_remove(struct per_session_data__minimal, pss_list, pss, vhd->pss_list);
		break;

	case LWS_CALLBACK_SERVER_WRITEABLE:
		if (!vhd->amsg.payload)
			break;

		if (pss->last == vhd->current)
			break;

		/* notice we allowed for LWS_PRE in the payload already */
		m = lws_write(wsi, ((unsigned char *)vhd->amsg.payload) + LWS_PRE, vhd->amsg.len, LWS_WRITE_TEXT);
		if (m < (int)vhd->amsg.len) 
        {
			lwsl_err("ERROR %d writing to ws\n", m);
			return -1;
		}

		pss->last = vhd->current;
		break;

	case LWS_CALLBACK_RECEIVE:
		if (vhd->amsg.payload)
			__minimal_destroy_message(&vhd->amsg);

		vhd->amsg.len = len;
		/* notice we over-allocate by LWS_PRE */
		vhd->amsg.payload = malloc(LWS_PRE + len);
		if (!vhd->amsg.payload) 
        {
			lwsl_user("OOM: dropping\n");
			break;
		}

		memcpy((char *)vhd->amsg.payload + LWS_PRE, in, len);
		vhd->current++;

		/*
		 * let everybody know we want to write something on them
		 * as soon as they are ready
		 */
		lws_start_foreach_llp(struct per_session_data__minimal **,ppss, vhd->pss_list) 
        {
            lws_callback_on_writable((*ppss)->wsi);
		} lws_end_foreach_llp(ppss, pss_list);
		break;

	default:
		break;
	}

	return 0;
}
bool parse_settings()
{
	// memset(path_id, 0, sizeof(wchar_t)*MAX_PATH);
	// wcscat(path_id, path_root);
	// wcscat(path_id, L"soap.json");
	// wprintf(L"id txt is: %s\r\n", path_id);

	std::wstring path = L"";// = Lpath_id;// L"c:\\soap.json";
    std::string filename;

	std::ifstream ifs(filename, std::ios::binary);
	rapidjson::IStreamWrapper isw(ifs);

	rapidjson::EncodedInputStream<rapidjson::UTF8<>, rapidjson::IStreamWrapper> eis(isw);
	rapidjson::Document doc;
	doc.ParseStream(eis);

	auto tp = doc.GetType();
	if (!doc.IsObject())
		return false;

	if (!doc.HasMember("start_date"))
	{
		//bool res = doc["start_date"].IsString();
		return false;
	}

	if (!doc.HasMember("server"))
	{
		//bool res = doc["server"].IsString();
		return false;
	}

	if (!doc.HasMember("region"))
	{
		//bool res = doc["region"].IsString();
		return false;
	}

	if (!doc.HasMember("prefix"))
	{
		//set.prefix = true;
		//bool res = doc["prefix"].IsBool();
		//return false;
	}
	//else
		//set.prefix = doc["prefix"].GetBool();

	if (!doc.HasMember("jpeg_only"))
	{
		//set.jpeg_only = true;
	}
	//else
		//set.jpeg_only = doc["jpeg_only"].GetBool();



	if (!doc.HasMember("accept_timeout"))
	{
		//set.accept_timeout = 60;
	}
	//else
		//set.accept_timeout = doc["accept_timeout"].GetInt();


	if (!doc.HasMember("connect_timeout"))
	{
		//set.connect_timeout = 60;
	}
	//else
		//set.connect_timeout = doc["connect_timeout"].GetInt();


	if (!doc.HasMember("recv_timeout"))
	{
		//set.recv_timeout = 60;
	}
	//else
		//set.recv_timeout = doc["recv_timeout"].GetInt();

	if (!doc.HasMember("send_timeout"))
	{
		//set.send_timeout = 120;
	}
	//else
		//set.send_timeout = doc["send_timeout"].GetInt();

	if (!doc.HasMember("transfer_timeout"))
	{
		//set.transfer_timeout = 120;
	}
	//else
		//set.transfer_timeout = doc["transfer_timeout"].GetInt();

	// if (!doc.HasMember("cntBackup"))
	// 	set.cntBackup = 600;
	// else
	// 	set.cntBackup = doc["cntBackup"].GetInt();

	// if (!doc.HasMember("cntTemp"))
	// 	set.cntTemp = 100;
	// else
	// 	set.cntTemp = doc["cntTemp"].GetInt();

	// if (!doc.HasMember("azimut"))
	// 	set.azimut = "0";
	// else
	// 	set.azimut = doc["azimut"].GetString();

	std::string str = doc["server"].GetString();

	// strcpy(set.serverPath, str.c_str());

	// set.camera_id = getIdDevice();// doc["reg_num"].GetString();

	// set.start_date = doc["start_date"].GetString();

	// set.region = doc["region"].GetString();

	// set.c_type = getComplexType();
	
	return true;
}
int save_settings()
{
// 	std::wstring path = L"c:\\soap.json";

// 	rapidjson::Document doc;
// 	doc.SetObject();

// 	rapidjson::Value conf_num(rapidjson::kStringType);
// 	rapidjson::Value conf_id(rapidjson::kNumberType);

// 	conf_num.SetString("15060199", doc.GetAllocator());
// 	conf_id.SetInt(37);

// 	//for (auto par : parameters)
// 	//{
// 	//	rapidjson::Value val;
// 	//	if (par.second->setgetjson(false, &val, &doc))
// 	//	{
// 	//		rapidjson::Value vMemberName(rapidjson::kStringType);
// 	//		vMemberName.SetString(par.first.c_str(), doc.GetAllocator());
// 	//		olympus_configuration.AddMember(vMemberName, val, doc.GetAllocator());
// 	//	}
// 	//}
// 	//doc.AddMember(rapidjson::StringRef(this->name.c_str()), olympus_configuration, doc.GetAllocator());

//     std::string filename;

// 	std::ofstream ofs(filename, std::ios::binary);
// 	rapidjson::OStreamWrapper osw(ofs);
// 	rapidjson::PrettyWriter<rapidjson::OStreamWrapper> writer(osw);
// 	doc.Accept(writer);
// 	ofs.close();

// 	//rapidjson::StringBuffer buffer;
// 	//rapidjson::Writer<rapidjson::StringBuffer> swriter(buffer);
// 	//doc.Accept(swriter);
// 	//MD5 md5;
// 	//std::ofstream(path + L".md5") << md5(buffer.GetString());

//     StringBuffer s;
//     PrettyWriter<StringBuffer> writer(s);
//     writer.StartObject();
// //    writer.Key("version");
// //    writer.String("1.0");
// //    writer.Key("data");
// //    writer.String(ss_date.str().c_str());
//     writer.Key("mode");
//     writer.StartArray();
//     writer.String(path.c_str());

//     writer.EndArray();
//     writer.EndObject();
//     std::ofstream of(m_indexFilePath);


	return 0;
}
int convertToBase64(unsigned char *buf_out,unsigned char *buf_in,int data_length)
{
    //char *base64_string;
    //char* data;// your initialization here
    //int data_length = 42;// your data length here
    //int encoded_data_length = Base64encode_len(data_length);
    //void* base64_string = malloc(encoded_data_length);

    int size = Base64encode((char*)buf_out, (char*)buf_in, data_length);

    return size;
}
void convertFromBase64()
{
    char *base64_string;
    char* data = NULL;
    int data_length = 0;
    int alloc_length = Base64decode_len(base64_string);
    void *some_random_data = malloc(alloc_length);
    data_length = Base64decode(data, base64_string);
}
void jpegInit()
{
	memset(&cinfo, 0, sizeof(jpeg_compress_struct));
	memset(&jerr, 0, sizeof(jpeg_error_mgr));

return;
}
int jpegProc(unsigned char *outbuf, int allSize, unsigned char *image_buffer)
{
	unsigned long outsize = allSize;

	//compress
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);

	jpeg_mem_dest(&cinfo, &outbuf, &outsize);

	cinfo.image_width = IMAGE_WIDTH;      /* image width and height, in pixels */
	cinfo.image_height = IMAGE_HEIGHT;
	cinfo.input_components = 1;     /* # of color components per pixel */
	cinfo.in_color_space = JCS_GRAYSCALE; /* colorspace of input image */
	jpeg_set_defaults(&cinfo);

	jpeg_start_compress(&cinfo, true);

	JSAMPROW row_pointer[1];        /* pointer to a single row */
	int row_stride;                 /* physical row width in buffer */
	row_stride = IMAGE_WIDTH * 1;   /* JSAMPLEs per row in image_buffer */
	while ((cinfo.next_scanline < cinfo.image_height)) {
		row_pointer[0] = &image_buffer[cinfo.next_scanline * row_stride];
		jpeg_write_scanlines(&cinfo, row_pointer, 1);
	}

	jpeg_finish_compress(&cinfo);

	jpeg_destroy_compress(&cinfo);

	return outsize;

}
void jpegRelease()
{

	jpeg_finish_compress(&cinfo);
	jpeg_destroy_compress(&cinfo);

	//free(scr);

}
void readAndCompress(MODE_VERIFY mode)
{
    uint64_t id = 0;
    uint64_t time = 0;
    double lat = 0,lon = 0;

    gns_actualy wrt = {0};

    int res = 0;

    //use mode verify
    if(mode == MODE_VERIFY::VERIFY_TIME)
    {
        //time verify, get time and ccord from shared memory
        res = readFrame(&id,&time,&lat,&lon);
    }

    if(mode == MODE_VERIFY::VERIFY_COORD)
    {
        //coord verify, get time and coord from pipe
        while(true)
        {
            if(pthread_mutex_trylock(&gps_mutex) == 0)
            {
                wrt = gps_from_pipe;
                pthread_mutex_unlock(&gps_mutex);
                break;
            }
            else 
                printf("readAndCompress: gps mutex busy!!\r\n");
        };

        time = wrt.time;
        lat = wrt.lat;
        lon = wrt.lon;
    }

    if(mode == MODE_VERIFY::VERIFY_DIST)
    {

    }

    cv::Mat retLeft;
    cv::Mat retRight;

    //ADD DRAW RECOGN FRAME
    savePassage((char*)imgsum,(char*)(imgsum + IMAGE_HEIGHT*IMAGE_WIDTH),time,"2102999",&retLeft,&retRight/*ADD BOX FRAME FOR VERIFY_DIST*/);

    int nJpegSize1 = jpegProc((unsigned char*)dst_jpg_mem1,IMAGE_HEIGHT*IMAGE_WIDTH,(unsigned char*)retLeft.data);
    int nJpegSize2 = jpegProc((unsigned char*)dst_jpg_mem2,IMAGE_HEIGHT*IMAGE_WIDTH,(unsigned char*)(retRight.data));

    int leftRes = convertToBase64(left_b64,(unsigned char*)dst_jpg_mem1,nJpegSize1);
    int rightRes = convertToBase64(right_b64,(unsigned char*)dst_jpg_mem2,nJpegSize2);

    left_b64[leftRes] = 0;
    right_b64[rightRes] = 0;

    sendToClient.time = time;//wrt
    sendToClient.lat = std::to_string(lat);//wrt
    sendToClient.lon = std::to_string(lon);//wrt

    sendToClient.dist = 234.6;
    sendToClient.brightness_gain = 7;
    sendToClient.brightness_gamma = 89;
    sendToClient.brightness_shutter = 56;
}
int readFrame(uint64_t *id,uint64_t *time,double *lat,double *lon)
{
    bool res = false;
    uint64_t last_time = 0;//mcsec
    uint64_t last_index = 0;

    do
    {
        if(pthread_mutex_trylock(&ids_mutex) == 0)
        {
            res = true;

            last_time = ids_buffer->last_time;//mcsec
            last_index = ids_buffer->last_index;

            if(_last_index != last_index)
                _last_index = last_index;
            else
            {
                res = false;
                pthread_mutex_unlock(&ids_mutex);
                break;
            }
            
            //time_t sec = last_time/1000000;
            //uint64_t mcssec = last_time - sec*1000000;

            //std::tm* tt = gmtime(&sec);
            //printf("[%d] %d-%02d-%02d %02d:%02d:%02d.%06d\r\n",last_id,tt->tm_year + 1900,tt->tm_mon + 1,tt->tm_mday,tt->tm_hour,tt->tm_min,tt->tm_sec,mcssec);

            pthread_mutex_unlock(&ids_mutex);
        }
        else 
        {
            printf("writeIds mutex busy!!\r\n");
            res = false;
            usleep(1000);
        }
    }while(!res);

    if(res)
    {
        if(pthread_mutex_trylock(&ipc_mutex[last_index]) == 0)
        {
            memcpy(imgsum,img_buffer->frame[last_index].buf,2*IMAGE_HEIGHT*IMAGE_WIDTH);
            _lastTime = img_buffer->frame[last_index].time;

            *id = img_buffer->frame[last_index].id;
            *time = img_buffer->frame[last_index].time_gns;
            *lat = img_buffer->frame[last_index].lat;
            *lon = img_buffer->frame[last_index].lon;
            pthread_mutex_unlock(&ipc_mutex[last_index]);

            //printf("cnt=%d id=%lld\r\n",last_index,*id);
        }
        else 
        {
            printf("writeFrame mutex busy!!\r\n");
            printf("last_id = [%d]\r\n",last_index);
        }
    }

    if(res)
    return 0;
    return -1;
}
int init_pipe_rd(char *fdn)
{
    printf("try init %s\r\n",fdn);

    int fd_fifo = -1;           
    do
    {
        fd_fifo = open(fdn,O_RDONLY);
        if(fd_fifo < 0)
        {
            printf("open rd error[%d] %s\r\n",errno,fdn);
            //return fd_fifo;
            sleep(5);
        }
    } while (fd_fifo < 0);

    printf("init %s OK\r\n",fdn);

    return fd_fifo;
}
void savePassage(char *left_fact,char *right_fact,uint64_t ttime,char *number,cv::Mat *retLeft,cv::Mat *retRight)
{
  int hblackString = 200;

  cv::Mat mFullLeft(IMAGE_HEIGHT + hblackString,IMAGE_WIDTH,CV_8UC1);
  cv::Mat mFullRight(IMAGE_HEIGHT + hblackString,IMAGE_WIDTH,CV_8UC1);

  cv::Mat mLeft(IMAGE_HEIGHT,IMAGE_WIDTH, CV_8UC1, (void*)left_fact);
  cv::Mat mRight(IMAGE_HEIGHT,IMAGE_WIDTH, CV_8UC1, (void*)right_fact);

  mLeft.copyTo(mFullLeft(cv::Rect(0,0,IMAGE_WIDTH,IMAGE_HEIGHT)));
  mRight.copyTo(mFullRight(cv::Rect(0,0,IMAGE_WIDTH,IMAGE_HEIGHT)));

  cv::Point p0(0,IMAGE_HEIGHT), p1(IMAGE_WIDTH, IMAGE_HEIGHT + hblackString);
  cv::rectangle(mFullLeft, p0,p1, cv::Scalar(0, 0, 0),cv::FILLED);
  cv::rectangle(mFullRight, p0,p1, cv::Scalar(0, 0, 0),cv::FILLED);

  char str_left[1024] = {0};
  char str_right[1024] = {0};

  time_t tt = ttime / 1000000;
  int tt_ms = ttime - tt*1000000;
  std::tm *ptm = std::localtime(&tt);

  sprintf(str_left,"TANDEM LEFT %s %d-%02d-%02d %02d:%02d:%02d.%06d",number,ptm->tm_year + 1900,ptm->tm_mon + 1,ptm->tm_mday,ptm->tm_hour,ptm->tm_min,ptm->tm_sec,tt_ms);

  cv::putText(mFullLeft, 
            str_left,
            cv::Point(15,IMAGE_HEIGHT + 45), // Coordinates
            cv::FONT_HERSHEY_COMPLEX_SMALL, // Font
            2.0, // Scale. 2.0 = 2x bigger
            cv::Scalar(255,255,255), // BGR Color
            3); // Line Thickness (Optional)
            //cv::CV_AA); // Anti-alias (Optional)

  sprintf(str_right,"TANDEM RIGHT %s %d-%02d-%02d %02d:%02d:%02d.%06d",number,ptm->tm_year + 1900,ptm->tm_mon + 1,ptm->tm_mday,ptm->tm_hour,ptm->tm_min,ptm->tm_sec,tt_ms);

  cv::putText(mFullRight, 
            str_right,
            cv::Point(15,IMAGE_HEIGHT + 45), // Coordinates
            cv::FONT_HERSHEY_COMPLEX_SMALL, // Font
            2.0, // Scale. 2.0 = 2x bigger
            cv::Scalar(255,255,255), // BGR Color
            3); // Line Thickness (Optional)
            //cv::CV_AA); // Anti-alias (Optional)

  //cv::imwrite("/mnt/m2/blabber/hjh.jpg",mFullLeft);
  //cv::imwrite("/mnt/m2/blabber/hjh2.jpg",mFullRight);

  *retLeft = mFullLeft.clone();
  *retRight = mFullRight.clone();

  int jj = retLeft->cols;
  int jj2 = retLeft->rows;

  return;
}
double calcsift(box left,box right,int un,uint64_t id)
{
    bool bSaveDebugPair = false;
	//const char* fileLeft = "/mnt/m2/home-develop/sift/left20m.jpg";//argv[1];
	//const char* fileRight = "/mnt/m2/home-develop/sift/right20m.jpg";//argv[2];

    std::string::size_type sz2;
    //diffAxis = std::stod(std::string(argv[3]), &sz2);

    double k = 0.3;
    box left_crop = left;
    box right_crop = right;

    left_crop.x = left_crop.x + k*left_crop.w;
    left_crop.y = left_crop.y + k*left_crop.h;
    left_crop.w = left_crop.w - 2*k*left_crop.w;
    left_crop.h = left_crop.h - 2*k*left_crop.h;

    right_crop.x = right_crop.x + k*right_crop.w;
    right_crop.y = right_crop.y + k*right_crop.h;
    right_crop.w = right_crop.w - 2*k*right_crop.w;
    right_crop.h = right_crop.h - 2*k*right_crop.h;

    box pleft = left_crop;
    box pright = right_crop;

    double contrast = 0;//std::stod(std::string(argv[4]), &sz2);//чем больше, тем меньше точек
    int edge = 0;//std::stod(std::string(argv[5]), &sz2);//чем больше, тем больше точек
    int sigma = 0;//std::stod(std::string(argv[6]), &sz2);//чем хуже изображение, тем меньше число

    //diffAxis = 350;
    contrast = 0.05;//025;
    edge = 4;
    sigma = 2;

	//printf("load file-left:%s\n", fileLeft);
	//printf("load file-right:%s\n", fileRight);

	//cv::Ptr<SIFT> sift = SIFT::create(0, 6, contrast, edge, sigma);//0, 4, 0.04, 10, 1.6//0.1 20 2

    vector<KeyPoint> keypointsLeft;
    Mat descriptorsLeft;

    vector<KeyPoint> keypointsRight;
    Mat descriptorsRight;

    //Mat origLeftImg;//from shared memory
    //Mat origRightImg;//from shared memory

    void *left_pict_ts = nullptr;
    void *right_pict_ts = nullptr;

    //void *color_left_pict_ts = nullptr;
    //void *color_right_pict_ts = nullptr;

    left_pict_ts = malloc(pleft.h*pleft.w);
    right_pict_ts = malloc(pright.h*pright.w);

    //if(bSaveDebugPair)color_left_pict_ts = malloc(pleft.h*pleft.w*3);
    //if(bSaveDebugPair)color_right_pict_ts = malloc(pright.h*pright.w*3);

    //todo!!
    readPartFrame(pleft,left_pict_ts);
    readPartFrame(pright,right_pict_ts);

    // std::string file_path_l = "";
    // file_path_l.append("/tmp/tmp-test/sift/left");
    // char pn_l[10]={0};
    // sprintf(pn_l,"%06d",un);
    // file_path_l.append(std::string(pn_l));
    // file_path_l.append(".bmp");

    // std::string file_path_r = "";
    // file_path_r.append("/tmp/tmp-test/sift/right");
    // char pn_r[10]={0};
    // sprintf(pn_r,"%06d",un);
    // file_path_r.append(std::string(pn_r));
    // file_path_r.append(".bmp");

    //  std::string file_path_big = "";
    //  if(bSaveDebugPair)file_path_big.append("/mnt/m2/records/my_result/sift");
    //  char pn_big[30]={0};
    //  if(bSaveDebugPair)sprintf(pn_big,"%06lld-%06d",id,un);
    //  if(bSaveDebugPair)file_path_big.append(std::string(pn_big));
    //  if(bSaveDebugPair)file_path_big.append(".bmp");

    //printf("[%d]lw = %d lh = %d [%d]rw = %d rh = %d\r\n",left.id,left.w,left.h,right.id,right.w,right.h);

    // cv::Mat mLeft(left.h,left.w, CV_8UC1, (void*)left_pict_ts);
    // cv::Mat mRight(right.h,right.w, CV_8UC1, (void*)right_pict_ts);

    // Mat mLeftColor(left.h,left.w, CV_8UC3, (void*)color_left_pict_ts);
    // cv::cvtColor(mLeft,mLeftColor,cv::COLOR_GRAY2RGB);

    // Mat mRightColor(right.h,right.w,CV_8UC3, (void*)color_right_pict_ts);
    // cv::cvtColor(mRight,mRightColor,cv::COLOR_GRAY2RGB);  

    // cv::Size network_size = cv::Size(std::max(left.w,right.w),std::max(left.h,right.h));
    // cv::Mat mLeft2(std::max(left.h,right.h),std::max(left.w,right.w),CV_8UC1);
    // cv::resize(mLeft,mLeft2,network_size);

    // cv::Mat mRight2(std::max(left.h,right.h),std::max(left.w,right.w),CV_8UC1);
    // cv::resize(mRight,mRight2,network_size);

    // cv::Mat bigPict(std::max(left.h,right.h),2*std::max(left.w,right.w),CV_8UC1);

    // cv::hconcat(mLeft2,mRight2,bigPict);
    // cv::imwrite(file_path_big,bigPict);

    //SaveBMP((char*)file_path_l.c_str(), left.w, left.h, (unsigned char*)left_pict_ts);
    //SaveBMP((char*)file_path_r.c_str(), right.w, right.h, (unsigned char*)right_pict_ts);

    //cv::Rect leftROI(left.x, left.y, left.w, left.h);
    //origLeftImg = origLeftImg(leftROI);

    //cv::Rect rightROI(right.x, right.y, right.w, right.h);
    //origRightImg = origRightImg(rightROI);

    //printf("left image:%d keypoints are found.\n", (int)keypointsLeft.size());
    //printf("right image:%d keypoints are found.\n", (int)keypointsRight.size());

    // Create a image for displaying mathing keypoints
    cv::Mat mLeft(pleft.h,pleft.w, CV_8UC1, (void*)left_pict_ts);
    cv::Mat mRight(pright.h,pright.w, CV_8UC1, (void*)right_pict_ts);
    
  //   //if(bSaveDebugPair)
  //   //{
  //     Mat mLeftColor(pleft.h,pleft.w, CV_8UC3, (void*)color_left_pict_ts);
  //     cv::cvtColor(mLeft,mLeftColor,cv::COLOR_GRAY2RGB);

  //     Mat mRightColor(pright.h,pright.w,CV_8UC3, (void*)color_right_pict_ts);
  //     cv::cvtColor(mRight,mRightColor,cv::COLOR_GRAY2RGB);  

  //     Size sz = Size(mLeft.size().width + mRight.size().width, mRight.size().height + mLeft.size().height);
  //     Mat matchingImage = Mat::zeros(sz, CV_8UC3);

  //   // Draw left image
  //     Mat roi1 = Mat(matchingImage, Rect(0,0, mLeft.size().width, mLeft.size().height));
  //     mLeftColor.copyTo(roi1);

  //   // Draw right image
  //     Mat roi2 = Mat(matchingImage, Rect(mLeft.size().width, mLeft.size().height, mRight.size().width, mRight.size().height));
  //     mRightColor.copyTo(roi2);
  //  //}

/****************/

    /*sift->detect(mLeft, keypointsLeft);
    sift->compute(mLeft, keypointsLeft, descriptorsLeft);

    sift->detect(mRight, keypointsRight);
    sift->compute(mRight, keypointsRight, descriptorsRight);
*/
    if(keypointsLeft.size() == 0 || (int)keypointsRight.size() == 0)
    {
      free(left_pict_ts);
      free(right_pict_ts);

      //  if(bSaveDebugPair)
      //  {
      //    free(color_left_pict_ts);
      //    free(color_right_pict_ts);
      //  }
      return -1;
    }

    // for (int i = 0; i < keypointsLeft.size(); i++)
    // {
    //     KeyPoint kp = keypointsLeft[i];
    //     //circle(matchingImage, kp.pt, cvRound(kp.size * 0.25), Scalar(255, 255, 0), 2, 8, 0);
    // }
    // for (int i = 0; i < keypointsRight.size(); i++)
    // {
    //     KeyPoint kp = keypointsRight[i];
    //     {
    //         kp.pt.x += mLeft.size().width;
    //         kp.pt.y += mLeft.size().height;
    //         //circle(matchingImage, kp.pt, cvRound(kp.size * 0.25), Scalar(255, 255, 0), 2, 8, 0);
    //     }
    // }

    // auto chronotime1 = std::chrono::high_resolution_clock::now();
    // std::chrono::microseconds mcs1 = duration_cast<std::chrono::microseconds>(chronotime1.time_since_epoch());
    // std::chrono::milliseconds ms1 = duration_cast<std::chrono::milliseconds>(chronotime1.time_since_epoch());
    // std::chrono::seconds sec1 = duration_cast<std::chrono::seconds>(ms1);

    // int mcs1c = mcs1.count();
    // int ms1c = ms1.count();
    // int sec1c = sec1.count();

    BFMatcher matcher(NORM_L2, true);//NORM_L1
    std::vector<std::vector<DMatch>> matches;
    matcher.knnMatch(descriptorsLeft, descriptorsRight, matches,1);

/****************/

    // printf("matches.size() = %d\r\n",matches.size());

    // std::vector<DMatch> match1;
    // std::vector<DMatch> match2;

    int colorCnt = 0;
    //std::vector<DeepCoord> vecDist;
    double dpx = 0;
    std::vector<sf_match> _sf_match;
    
    //if(bSaveDebugPair)
    //FILE *fp = fopen("/mnt/m2/records/my_result/sift.log","at");0
    //int size_good_res = 0;

      for(int i = 0; i < matches.size(); i++)
      {
          colorCnt++;
          if (colorCnt > 13) colorCnt = 0;

          if (matches[i].size() > 0)
          {
            Point2f leftPoint = keypointsLeft[matches[i][0].queryIdx].pt;
            Point2f rightPoint = keypointsRight[matches[i][0].trainIdx].pt;
            //printf("sift: dist = %lld\r\n",matches[i][0].distance);
            
            sf_match sfm;
            sfm.dist = matches[i][0].distance;
            sfm.index = i;

            _sf_match.push_back(sfm);

          //   if(matches[i][0].distance < 100.0)
          //   {
          //     middle_dist += std::abs((leftPoint.x + left.x) - (rightPoint.x + right.x));
          //     size_good_res++;
          //   }

          //   sprintf(sift_log,"%s[%d][%3.3f]xL=%4.1f xR=%4.1f XL=%4.1f XR=%4.1f diff=%4.1f\r ",pn_big,i,matches[i][0].distance,leftPoint.x,rightPoint.x,leftPoint.x + left.x,rightPoint.x + right.x,(leftPoint.x + left.x) - (rightPoint.x + right.x));
          //   fwrite(sift_log,sizeof(char),strlen(sift_log),fp);
              
          //   if(bSaveDebugPair)
          //   {
          //     rightPoint.x += mLeft.size().width;
          //     rightPoint.y += mLeft.size().height;
          //     line(matchingImage, leftPoint, rightPoint, colorLine(colorCnt), 1);
          //  }
          }
      }

      //fclose(fp);

      if(_sf_match.size() > 2)
      {
        // сортируем по октавам, по dist.
        // не дистанция !!!

        std::sort(_sf_match.begin(),_sf_match.end(),siftSort);
        
        Point2f lp = keypointsLeft[matches[_sf_match[0].index][0].queryIdx].pt;
        Point2f rp = keypointsRight[matches[_sf_match[0].index][0].trainIdx].pt;
        
        // это разница в пикселях
        double df = (lp.x + pleft.x) - (rp.x + pright.x);

        if(df > 0)
        {
          dpx = df;
        }
        else
          dpx = SIFT_IS_IMPOSSIBLE;

      }
      else
        dpx = SIFT_IS_IMPOSSIBLE;

    

    // if(size_good_res > 0)
    //    middle_dist = middle_dist / size_good_res;
    //  else
    //    middle_dist = 9999.9999;
    
    // if(bSaveDebugPair)
    // cv::imwrite(file_path_big,matchingImage);

    /****************/

    // auto chronotime2 = std::chrono::high_resolution_clock::now();
    // std::chrono::microseconds mcs2 = duration_cast<std::chrono::microseconds>(chronotime2.time_since_epoch());
    // std::chrono::milliseconds ms2 = duration_cast<std::chrono::milliseconds>(chronotime2.time_since_epoch());
    // std::chrono::seconds sec2 = duration_cast<std::chrono::seconds>(ms2);

    // int mcs2c = mcs2.count() - mcs1c;
    // int ms2c = ms2.count() - ms1c;
    // int sec2c = sec2.count() - sec1c;

    // printf("msec latency %d[%d-%d]\r\n",ms2c,ms2.count(),ms1.count());
    // printf("mcsec latency %d[%d-%d]\r\n",mcs2c,mcs2.count(),mcs1.count());
    //printf("mcsec %d\r\n",mcs2c);

     
    // Display mathing image
    // imshow("mywindow", matchingImage);
    // imwrite("Gray_Image.jpg", matchingImage);

    // printf("size overlap = %d \r\n", (int)vecDist.size());
    // Mat img_deep_map(origLeftImg.size().height,origLeftImg.size().width, CV_8U, Scalar(0,0,0));
    // for(int i = 0;i < vecDist.size();i++)
    // {
    //     int deep = (double)(254.0 / (double)origLeftImg.size().width)* (double)vecDist[i].deep;//0-BLACK=DALEKO,255-WHITE-BLIZKO
    //     //img_deep_map.at<uchar>(vecDist[i].y, vecDist[i].x) = deep;
        
    //     int w = 2;
    //     int h = 2;

    //     cv::Point pt1(vecDist[i].x-w/2, vecDist[i].y-h/2);
    //     cv::Point pt2(vecDist[i].x + w/2, vecDist[i].y + h/2);

    //     //if(deep < 50)
    //      //   deep = 250;

    //     cv::rectangle(img_deep_map, pt1, pt2, cv::Scalar(deep, deep, deep),FILLED);

    //     //printf("deep[px] = %d",deep);
    //     //printf(" dist = %3.3f\r\n",calcClassicZ(vecDist[i].lx,vecDist[i].rx));
    // }

    //applyColorMap(img_deep_map, img_deep_map, cv::COLORMAP_JET);//COLORMAP_JET
    //imwrite("DeepMap.jpg", img_deep_map);
    //imshow("jetmap", img_deep_map);

    //Mat draw;
    //distanceTransform(img_deep_map, draw, CV_DIST_L2, 5);//CV_DIST_L2
    //draw.convertTo(draw, CV_8U, 20);
    //applyColorMap(draw, draw, cv::COLORMAP_RAINBOW);//COLORMAP_JET
    //imshow("jetmap", draw);
    //waitKey();

      free(left_pict_ts);
      free(right_pict_ts);

      // if(bSaveDebugPair)
      // free(color_left_pict_ts);
      // free(color_right_pict_ts);

  return dpx;
}
int readPartFrame(box bx,void *pic)
{
    bool res = false;
    uint64_t last_time = 0;//mcsec
    uint64_t last_index = 0;

    for(int i = SALMON_SIZE - 1;i >= 0;i--)
    {
        if(img_buffer->frame[i].id == bx.id)
        {
          //if(pthread_mutex_trylock(&ipc_mutex[i]) == 0)
          //{
              for(int j = 0;j < bx.h;j++)
                  memcpy((void*)(pic + j*bx.w),img_buffer->frame[i].buf + bx.y*IMAGE_WIDTH+bx.x + j*IMAGE_WIDTH,bx.w);

              res = true;

          //   pthread_mutex_unlock(&ipc_mutex[i]);
               break;
          //  }
          //  else 
          //  {
          //      printf("readFrame mutex busy!! last_id = [%d]\r\n",i);
          //      //printf("last_id = [%d]\r\n",i);
          //  }
        }
    }
    //    usleep(25000);


    if(res)
    return 0;
    return -1;
}
double calcClassicZ(double pxDiff,double dl,double dr,double cpx)
{
    double pxSize = 11.3 / 1920.0;// 0.00586;//mm
    double calibrPx = cpx;// -12;// 7.5;
    double baseMaket = 1025.0;//mm
    //double baseWall = 944.0;//mm

    double diffWallLeft = dl;// 20.0;//mm
    double diffWallRight = dr;// -15.0;//mm

    double distToWall = 7326;//mm//7490

    double focus = 36.649078;//mm

    //расчет фокусного растояния по 10см линейке на стенке
    double wMatr = 11.3;
    double wWall = (1920.0 / 85.0) * 100.0 * 0.5;//сколько 10см линеек помеещается в кадре на расстоянии distToWall//118
    double tanAlpha05 = wWall / distToWall;
    focus = wMatr / (2.0 * tanAlpha05);

    // если реальная ось левее чем надо, то кол-во пикселей будет больше чем надо, 
    // то есть надо кол-во пикселей уменьшать, для этого число в знаменателе должно быть больше = +
    //diffPxLeft = diffPxLeft / (1.0 + (diffWallLeft / distToWall));

    // если реальная ось левее чем надо, то кол-во пикселей будет меньше чем надо, 
    // то есть надо кол-во пикселей увеличивать, для этого число в знаменателе должно быть меньше = -
    //diffPxRight = diffPxRight / (1.0 + (diffWallRight / distToWall));

    double diffPx = pxDiff - calibrPx;//mm
    double z = (focus * baseMaket) / (pxSize * diffPx);

    //double distReal = 13006;
    //double dcpx = (focus * baseMaket) / (pxSize * distReal);

    return z;
}

double calcDist(double pxDiff1)
{
    double width = 1920.0;
    double widthCentr = width / 2.0;

    //здесь расчет фокуса
    //*begin
    double distToWall = 7326.0;//расстояние на которо мкрепилась линейка
    double wWall = (1920.0 / 85.0) * 100.0 * 0.5;//сколько 10см линеек помеещается в кадре на расстоянии distToWall, 121 пиксель занимает 10см на расстоянии 7400мм
    double tanAlpha05 = wWall / distToWall;
    double alphaView05 = atan(tanAlpha05) * 180.0 / M_PI;
    //*end

    double angle1 = (alphaView05 / widthCentr) * abs(pxDiff1);/* - abs(widthCentr - diffPxRight1)*/ //угол в первый момент времени
    //double angle2 = (alphaView05 / widthCentr) * abs(pxDiff2);/* - abs(widthCentr - diffPxRight2)*/ //угол во второй момент времени

    double measureDist1 = 0.001*calcClassicZ(pxDiff1,0,0,-10.95);//m расстояние измеренное в первый момент времени
    //double measureDist2 = 0.001*calcClassicZ(pxDiff2,0,0,-10.95);//m расстояние во второй момент времени

    double dist1 = measureDist1*cos((angle1 / 180.0) * M_PI);//m расстояние пересчитанное с учетом угла левый
    //*dist2 = measureDist2*cos((angle2 / 180.0) * M_PI);//m расстояние пересчитанное с учетом угла правый

    //double time1 = 37.748;//время начального кадра
    //double time2 = 37.856;//время конечного кадров

    return dist1;
}