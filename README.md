# Internet of Things
SYSC 4001 Operating Systems, Fall 2015
Programming Assignment 1: Emulation of Internet of Things (IoT) Using Software Processes
Monica Ruttle
Bronwyn Skelley

Assignment 1 contains four c files and one header file. The files include cloud.c, controller.c, sensor.c, actuator.c and iotinfo.h.

The program begins by starting the controller.c file, which represents the logic of the controller. The controller begins by creating the queue used to communicate with the devices. It then forks itself, creating a parent and a child process. The parent process will wait to communicate with the cloud, and the child process will wait to communicate with the devices.

The next file to run is cloud.c, which controls the logic for the cloud. This will create the FIFO used to communicate with the parent process of the controller. The cloud with start by waiting for a message from the controller. 

The next file to execute is actuator.c, which controls the logic of the actuator. When running the actuator from the command line, it must take one parameters, it's name. The actuator will start by registering itself with the controller. The controller will print the actuator's name and indicate it has been registered. Once it registers itself, the actuator will receive acknowledgement from the controller and print this acknowledgement. It will then wait for a message from the controller. 

The next file to run is the sensor.c, which controls the logic for the sensor. When running the sensor from the command line, it must take two parameters, the first being its name, and the second being its threshold. It begins by registering itself with the controller. It's name is printed by the controller and confirmation it has been registered. The sensor will receive acknowledgement and print this. 

Subsequently, every 2 seconds, the sensor will send random data to the controller. The controller checks the threshold it saved from when registering for the device, and checks if the data is over. If it is over, the parent process sends a message to the cloud to indicate this. The cloud prints this message. The child process will send an Alert to the actuator, and it will "turn on a fan". The process repeats every 2 seconds. After a value has gone over the threshold, the next line will have a garbage value as the first character.

For every actuator that is executed, a sensor must be executed immediately after. If more than one of these pairs would like to be run, repeat the starting process for the actuator and sensor as many times as required.

After forty iterations of the child process loop, the child will kill the parent process. It will also send a STOP signal to the actuators and the sensons, which will terminate those as well. The cloud remains running as logically, a controller should not be able to end a cloud process.

[1] Matthew, Neil, and Richard Stones. Beginning Linux Programming. 4th ed. Birmingham, UK: Wrox, 2007. Print.
