#define ACTUATOR '0'
#define SENSOR '1'
#define QUEUE 4444
#define ACK "ACK"
#define STOP "STOP"
#define IS_REGISTERED 0
#define IS_NOT_REGISTERED 1
#define BUFFSIZE 100


struct private_info{
	pid_t pid;
	char name[25];
	char device_type;
	int is_new_device; //a flag if the device has been resgisterd or not
	int threshold; //actuators have a threshold value of 0
	int sensing_data; //if a device is being registered, sensing data is 0
};

//information to register each device
struct device_info{
	long int my_message_type;
	struct private_info info;
};

struct message_from_controller{
	long int my_message_type;
	char message[BUFFSIZE];
};

//linked list node, stores a device and info to next device
struct node{
	struct device_info device;
	struct node *next;
};

static const struct device_info empty_device;
