#define ACTUATOR '0'
#define SENSOR '1'
#define QUEUE 4444
#define ACK "ACK"
#define STOP "STOP"
#define IS_REGISTERED 0
#define IS_NOT_REGISTERED 1
#define BUFFSIZE 100
#define FIFO_NAME "/tmp/my_fifo"

int program_running;


struct private_info{
	pid_t pid;
	char name[25];
	char device_type;
	int is_new_device; //a flag if the device has been resgisterd or not
	int threshold; //actuators have a threshold value of 0, and sensors after they have been registered have a value of 0
	int sensing_data; //if a device is being registered, sensing data is 0
};

//information to register each device
struct device_info{
	long int my_message_type;
	struct private_info info;
};

//short messages to and from the controller
struct message_from_controller{
	long int my_message_type;
	char message[BUFFSIZE];
};

//lists of sensors and actuators
struct device_info sensors[5];
struct device_info actuators[5];
