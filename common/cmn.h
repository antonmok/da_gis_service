#define IMAGE_HEIGHT 1200
#define IMAGE_WIDTH 1920

#define IMAGE_WEB_HEIGHT    IMAGE_HEIGHT / 2
#define IMAGE_WEB_WIDTH     IMAGE_WIDTH / 2

inline constexpr auto& SALMON_PATH = "/salmon";
inline constexpr auto& SALMON_IDS_PATH = "/salmon_ids";

inline constexpr auto& TROUT_PATH = "/trout";
inline constexpr auto& TROUT_IDS_PATH = "/trout_ids";

inline constexpr auto& PIPE_TROUT_TO_PROC = "/tmp/trout-to-proc";    // transmit only ts frame
inline constexpr auto& PIPE_TROUT_TO_COD = "/tmp/trout-to-cod";      // transmit only grz frame
//inline constexpr auto& PIPE_TROUT_TO_PROC2 = "/tmp/trout-to-sproc";  // transmit only sign frame
inline constexpr auto& PIPE_COD_TO_PROC = "/tmp/cod-to-proc";        // transmit only grz text

inline constexpr auto& PIPE_SALMON_TO_BLABBER = "/tmp/salmon-to-blabber"; //transmit only gns

#define SALMON_SIZE 800
#define TROUT_SIZE 800

#define DESC_SIZE 64

//SALMON_IDS_PATH
struct shm_ids_mutex
{
    pthread_mutex_t ipc_mutex;
    uint64_t last_time;//mcsec
    uint64_t last_index;//id in array, index array
};

//SALMON_PATH
struct shm_mutex
{
    uint64_t id;//global from cam
    uint64_t time;//mcsec
    uint64_t time_gns;//mcsec
    double lat;// + or -, + is N
    double lon;// + or -, + is E
    pthread_mutex_t ipc_mutex;
    unsigned char buf[2*IMAGE_HEIGHT*IMAGE_WIDTH]; 
};

//SALMON_PATH
struct shm_mutex_array
{
    shm_mutex frame[SALMON_SIZE];
};

//TROUT_PATH
struct box 
{
    unsigned int x, y, w, h;       // (x,y) - top-left corner, (w, h) - width & height of bounded box
    float prob;                    // confidence - probability that the object was found correctly
    unsigned int obj_id;           // class of object - from range [0, classes-1]
    unsigned int track_id;         // tracking id for video (0 - untracked, 1 - inf - tracked object)
    uint64_t id;               // counter of frames on which the object was detected
    uint64_t time;                  // event time
    unsigned int count;            // increase counter, max value size of batch
    unsigned int size_batch;       // size of batch
    char desc[DESC_SIZE];
    short int side;                 //0-left,1-right
    uint64_t uniq;
    //uint64_t time_first;
    //uint64_t time_second;
};

struct gns_actualy
{
    double lat;
    double lon;
    uint64_t time;
};

enum side {LEFT,RIGHT};