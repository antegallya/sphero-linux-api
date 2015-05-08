/*************************************************************************
	Sphero  -  Wrapper implementing all sphero-linked features
								(like packet creation, emission, reception)
							 -------------------
	started                : 07/03/2015
 ************************************************************************/

//-------------------------------------------------------- System includes

#include <endian.h>
#include <sys/socket.h>
#include <pthread.h>
#include <unistd.h>

//--------------------------------------------------------- Local includes

#include "Sphero.hpp"
#include "packets/SpheroPacket.hpp"
#include "packets/Constants.hpp"


//-------------------------------------------------------- Private methods

void* Sphero::monitorStream(void* sphero_ptr)
{
	Sphero* sphero = (Sphero*) sphero_ptr;
	int _bt_sock = sphero->_bt_socket;
	uint8_t buf;
	SpheroPacket* packet_ptr;

	for(;;)
	{
		if(recv(_bt_sock, &buf, sizeof(buf), MSG_PEEK) <= 0)
		{
			sphero->disconnect();
		}
		else
		{
			if(SpheroPacket::extractPacket(_bt_sock, sphero, &packet_ptr))
			{
				packet_ptr->packetAction();
			}
		}
	}

    return NULL;
}//END monitorStream

void Sphero::reportCollision(CollisionStruct* infos)
{
	_collision_handler.reportAction(infos);
}

//------------------------------------------------ Constructors/Destructor


/**
 * @param btaddr : Device address (format : "XX:XX:XX:XX:XX:XX")
 * @param btcon : A pointer to the bluetooth connector
 */
Sphero::Sphero(char const* const btaddr, bluetooth_connector* btcon):
	_connected(false), _bt_adapter(btcon), _address(btaddr)
{}


Sphero::~Sphero()
{
	disconnect();
	delete _bt_adapter;
}//END destructor


//--------------------------------------------------------- Public methods

/**
 * @brief connect : Initializes the bluetooth connection to the sphero instance
 * @return true if the connection was successful, false otherwise
 */
bool Sphero::connect()
{
	disconnect();

	size_t i = 0;
	while((_bt_socket = _bt_adapter->connection(_address.c_str())) == -1 && 
			i++ < MAX_CONNECT_ATTEMPT)
	{
	}

	if(_bt_socket != -1)
	{
		pthread_create(&monitor, NULL, monitorStream, this);

		_connected = true;
		_connect_handler.reportAction();

		return true;
	}

	return false;
}//END connect


/**
 * @brief disconnect : Disconnects the current Sphero
 */
void Sphero::disconnect()
{
#ifdef MAP
	fprintf(stderr, "Logging out\n");
#endif

	if(_connected)
	{
		_connected = false;
		pthread_cancel(monitor);
		_bt_adapter->disconnect();
		
		_disconnect_handler.reportAction();
	}
}//END disconnect


/**
 * @brief sendPacket : Send the specified packet to the Sphero
 * @param packet : The packet to send to the Sphero
 */
void Sphero::sendPacket(ClientCommandPacket& packet)
{
	ssize_t retour;
	if((retour = send(_bt_socket, packet.toByteArray(),  packet.getSize(), 0)) <= 0)
	{
		disconnect();
	}

	fsync(_bt_socket);

}//END sendPacket


/**
 * @brief ping : Creates a ping request to the Sphero
 */
void Sphero::ping()
{
	ClientCommandPacket packet = ClientCommandPacket(
		DID::core,
		CID::ping,
		flags::notNeeded,
		0x01,
		NULL,
		true,
		true
	);
	sendPacket(packet);	
}//END ping


/**
 * @brief setColor : Changes the Sphero light color
 * @param red : level of red (between 0x00 and 0xFF)
 * @param green : level of green (between 0x00 and 0xFF)
 * @param blue : level of blue (between 0x00 and 0xFF)
 * @param persist : true if you want the color to be defined as user color
 */
void Sphero::setColor(uint8_t red, uint8_t green, uint8_t blue, bool persist)
{
	uint8_t data_payload[4];
	data_payload[0] = red;
	data_payload[1] = green;
	data_payload[2] = blue;
	if (persist)
		data_payload[3] = 1;
	else
		data_payload[3] = 0;

	ClientCommandPacket packet (
			DID::sphero,
			CID::setRGBLEDOutput,
			flags::notNeeded,
			0x05,
			data_payload, 
			waitConfirm, 
			resetTimer
		);
	sendPacket(packet);
}//END setColor


/**
 * @brief setBackLedOutput : Lights the back led(used to calibrate
 * 							 the spero direction) with the given power
 * @param power : The power the LED will receive
 */
void Sphero::setBackLedOutput(uint8_t power)
{
	ClientCommandPacket packet(
			DID::sphero,
			CID::setBackLEDOutput,
			flags::notNeeded,
			0x02,
			&power, 
			waitConfirm, 
			resetTimer
		);
	sendPacket(packet);
}//END setBackLedOutput


/**
 * @brief setHeading : Change the heading angle
 * @param heading : the new angle, in ° (range from 0 to 359)
 */
void Sphero::setHeading(uint16_t heading)
{
	uint8_t data[2];
	uint8_t* ptr = (uint8_t*) &heading;
	data[0] = ptr[1];
	data[1] = ptr[0];

	ClientCommandPacket packet(
			DID::sphero,
			CID::setHeading,
			flags::notNeeded,
			0x03,
			data, 
			waitConfirm, 
			resetTimer
		); 

	sendPacket(packet);
}//END setHeading


/**
 * @brief setStabilization : Enable or disable stabilization
 * @param on : enables if true, disables otherwise
 */
void Sphero::setStabilization(bool on)
{
	uint8_t state;
	state = on ? 1 : 0;

	ClientCommandPacket packet (
			DID::sphero,
			CID::setStabilization,
			flags::notNeeded,
			0x02,
			&state, 
			waitConfirm, 
			resetTimer
		); 
	sendPacket(packet);
}//END setStabilization


/**
 * @brief setRotationRate : Change the rotation speed
 * @param angspeed : The new rotation speed (new speed will be angspeed*0.784 degrees/sec)
 *		Warning :=: high value may become really uncontrollable
 */
void Sphero::setRotationRate(uint8_t angspeed)
{
	ClientCommandPacket packet(
			DID::sphero,
			CID::setRotationRate,
			flags::notNeeded,
			0x02,
			&angspeed, 
			waitConfirm, 
			resetTimer
		); 
	sendPacket(packet);
}//END setRotationRate


/**
* @brief setSelfLevel : This command controls the self level routine.
*			The self level routine attempts to achieve a horizontal orientation
*			where pitch and roll angles are less than the provided Angle Limit
*		After both angle limits are satisfied, option bits control sleep, final
*			angle (heading), and control system on/off. An asynchronous message
*			is returned when the self level routine completes (only when started
*			by API call)
*
* @param options : Flags to control the routine behavior. Disponible flags are:
*			Start/stop bit (only one of them can be used at a time):
*				START_ROUTINE : starts the routine
*				ABORT_ROUTINE : aborts the routine if in progress
*
*			Final angle bit:
*				CHECK_ANGLE : rotates to heading equal to beginning heading
*
*			Sleep bit:
*				SLEEP_AFTER_FINISH : The Sphero goes to sleep after leveling if used
*
*			Control System bit:
*				KEEP_CONTROL_SYSTEM : leaves control system on after leveling
*
* @param angle_limit : The max angle for completion (in degrees).
*			0 : Use the default value
*			Bounds : Between 1 and 90
*
* @param timeout : Set maximum seconds to run the routine
*			0 : Use the default value
*			Bounds : Between 1 and 255
*
* @param trueTime : Set the required “test for levelness” time to
*					10*<True Time> (in milliseconds)
*			0 : Use the default value
*			Bounds : Between 1 and 255
*/
void Sphero::setSelfLevel(uint8_t options, uint8_t angle_limit,
						  uint8_t timeout, uint8_t trueTime)
{
	uint8_t data_payload[4];
	data_payload[0] = options;
	data_payload[1] = angle_limit;
	data_payload[2] = timeout;
	data_payload[3] = trueTime;

	ClientCommandPacket packet(
			DID::sphero,
			CID::selfLevel,
			flags::notNeeded,
			0x05,
			data_payload, 
			waitConfirm, 
			resetTimer
		);
	sendPacket(packet);
}//END setSelfLevel


//void Sphero::setDataStreaming(uint16_t N, uint16_t M,uint32_t MASK, uint8_t 
//pcnt, uint32_t MASK2 = 0); not used yet


/**
 * @brief enableCollisionDetection : Enables the onBoard collision detector
 * @param Xt : An 8-bit settable threshold for the X (left/right) axis of Sphero
 *			A value of 0x00 disables the contribution of this axis
 *
 * @param Xspd : An 8-bit settable speed value for the X axis. This setting is
 *			ranged by the speed, then added to Xt to generate the final threshold value.
 *
 * @param Yt : An 8-bit settable threshold for the Y (front/back) axis of Sphero
 *			A value of 0x00 disables the contribution of this axis
 *
 * @param Yspd : An 8-bit settable speed value for the Y axis. This setting is
 *			ranged by the speed, then added to Yt to generate the final threshold value.
 *
 * @param Dead : An 8-bit post-collision dead time to prevent retriggering; specified in
 *			10ms increments.
 */
void Sphero::enableCollisionDetection(uint8_t Xt, uint8_t Xspd,
									  uint8_t Yt,  uint8_t Yspd,  uint8_t Dead)
{
	uint8_t data_payload[6];
	data_payload[0] = 0x01;
	data_payload[1] = Xt;
	data_payload[2] = Xspd;
	data_payload[3] = Yt;
	data_payload[4] = Yspd;
	data_payload[5] = Dead;

	ClientCommandPacket packet(
			DID::sphero,
			CID::configureCollisionDetection,
			flags::notNeeded,
			0x07,
			data_payload, 
			waitConfirm, 
			resetTimer
		);
	sendPacket(packet);
}//END enableCollisionDetection


/**
 * @brief disableCollisionDetection : Disables the onBoard collision detector
 */
void Sphero::disableCollisionDetection()
{
	uint8_t data_payload[6];
	data_payload[0] = 0x00;
	data_payload[1] = 0;
	data_payload[2] = 0;
	data_payload[3] = 0;
	data_payload[4] = 0;
	data_payload[5] = 0;

	ClientCommandPacket packet(
			DID::sphero,
			CID::configureCollisionDetection,
			flags::notNeeded,
			0x07,
			data_payload,
			waitConfirm,
			resetTimer);
	sendPacket(packet);
}//END disableCollisionDetection


/**
 * @brief isConnected : Checks the Sphero's connection state
 * @return true if it is connected.
 */
bool Sphero::isConnected()
{
	return _bt_adapter->isConnected();
}//END isConnected


/**
 * @brief configureLocator : Configure sphero's internal location
 * 							 calculation unit offsets
 * @param flags
 *			FLOATING_Y_AXIS : the Y axis won't be memorized, so heading 0 will do nothing
 * @param X : The current position on X axis of Sphero on the ground plane (in centimeters)
 * @param Y : The current position on Y axis of Sphero on the ground plane (in centimeters)
 * @param yaw : (yaw tare) Controls how the X,Y-plane is aligned with Sphero’s heading coordinate system.
 *			When this parameter is set to zero, it means that having yaw = 0 corresponds to facing down the Y-axis
 *																						 in the positive direction
 *			The value will be interpreted in the range 0-359 inclusive.
 */
void Sphero::configureLocator(uint8_t flags, uint16_t X, uint16_t Y, uint16_t yaw)
{
	uint8_t XA = (uint8_t)((X & 0xFF00) >> 8);
	uint8_t XB = (uint8_t)(X & 0x00FF);
	uint8_t YA = (uint8_t)((Y & 0xFF00) >> 8);
	uint8_t YB = (uint8_t)(Y & 0x00FF);
	uint8_t yawA = (uint8_t)((yaw & 0xFF00) >> 8);
	uint8_t yawB = (uint8_t)(yaw & 0x00FF);
	uint8_t data_payload[7];
	data_payload[0] = flags;
	data_payload[1] = XA;
	data_payload[2] = XB;
	data_payload[3] = YA;
	data_payload[4] = YB;
	data_payload[5] = yawA;
	data_payload[6] = yawB;

	ClientCommandPacket packet(
			DID::sphero,
			CID::configureLocator,
			flags::notNeeded,
			0x08,
			data_payload,
			waitConfirm,
			resetTimer);
	sendPacket(packet);
}//END configureLocator

//getLocator : we'll have to discuss this...
//getRGDLed : same


/**
 * @brief setAccelerometerRange : change sphero's accelerometer range,
 * 								warning : may cause strange behaviors
 * @param range : 	The accelerometer range. Use one (and only one!) of
 * 				  	the next flags to set the right range
 *					Any other value will have indeterminate consequences
 *					for driving and collision detection
 *
 *				ACC_2G : ±2Gs
 *				ACC_4G : ±4Gs
 *				ACC_8G : ±8Gs (default)
 *				ACC_16G : ±16Gs
 */
void Sphero::setAccelerometerRange(uint8_t range)
{
	ClientCommandPacket packet(
			DID::sphero,
			CID::setAccelerometerRange,
			flags::notNeeded,
			0x02,
			&range, 
			waitConfirm, 
			resetTimer
		); 
	sendPacket(packet);
}//END setAccelerometerRange


/**
 * @brief roll : Defines a new heading angle and a new rotation speed
 * @param speed : The new rotation speed (new speed will be speed*0.784 degrees/sec)
 * @param heading : the new angle, in ° (range from 0 to 359)
 * @param state : In the CES firmware, this was used to gate the
 *			control system to either obey the roll vector or ignore it and apply optimal
 *			braking to zero speed
 */
void Sphero::roll(uint8_t speed, uint16_t heading, uint8_t state)
{
	uint8_t msb = (uint8_t)((heading & 0xFF00) >> 8);
	uint8_t lsb = (uint8_t)(heading & 0x00FF);
	uint8_t data_payload[4];
	data_payload[0] = speed;
	data_payload[1] = msb;
	data_payload[2] = lsb;
	data_payload[3] = state;

	ClientCommandPacket packet(
			DID::sphero,
			CID::roll,
			flags::notNeeded,
			0x05,
			data_payload, 
			waitConfirm, 
			resetTimer
		);
	sendPacket(packet);
}//END roll


/**
 * @brief setInactivityTimeout :To save battery power, Sphero normally goes to sleep after a period of inactivity.
 * @param timeout : Time before Sphero goes to sleep (when nothing happens), in seconds
 *			From the factory this value is set to 600 seconds (10 minutes)
 *			The inactivity timer is reset every time an API command is received over Bluetooth or a shell
 *																			command is executed in User Hack mode.
 *			In addition, the timer is continually reset when a macro is running unless the MF_STEALTH
 *										flag is set, and the same for orbBasic unless the BF_STEALTH flag is set.
 */
void Sphero::setInactivityTimeout(uint16_t timeout)
{
	if(timeout < 60)
	{
		timeout = 60;
	}
	timeout = htobe16(timeout);
	uint8_t* data = (uint8_t*) &timeout;

	ClientCommandPacket packet(
			DID::core,
			CID::setInactivityTimeout,
			flags::notNeeded,
			0x03,
			data,
			waitConfirm,
			resetTimer
		);
	sendPacket(packet);
}//END setInactivityTimeout



/**
 * @brief sleep : This command puts Sphero to sleep immediately
 * @param time : The number of seconds for Sphero to sleep for and then automatically reawaken.
 *			Zero does not program a wakeup interval, so he sleeps forever.
 *			0xFFFF attempts to put him into deep sleep (if supported in hardware)
 *									and returns an error if the hardware does not support it.
 * @param macro : If non-zero, Sphero will attempt to run this macro ID upon wakeup.
 * @param orbbasic : If non-zero, Sphero will attempt to run an orbBasic program in Flash from this line number.
 */
void Sphero::sleep(uint16_t time, uint8_t macro, uint16_t orbbasic)
{
	uint8_t msbTime = (uint8_t)((time & 0xFF00) >> 8);
	uint8_t lsbTime = (uint8_t)(time & 0x00FF);
	uint8_t msbOrb = (uint8_t)((orbbasic & 0xFF00) >> 8);
	uint8_t lsbOrb = (uint8_t)(orbbasic & 0x00FF);
	uint8_t data_payload[5];
	data_payload[0] = msbTime;
	data_payload[1] = lsbTime;
	data_payload[2] = macro;
	data_payload[3] = msbOrb;
	data_payload[4] = lsbOrb;

	ClientCommandPacket packet(
			DID::core,
			CID::sleep,
			flags::notNeeded,
			0x06,
			data_payload, 
			waitConfirm, 
			resetTimer
		);
	sendPacket(packet);
	
}//END sleep

/*
//setRawMotorValue : not needed ?

void Sphero::setMotionTimeout(uint16_t time);

void Sphero::setPermOptFlags(uint32_t flags);

//getPermOptFlags : we'll see

void Sphero::setTmpOptFlags(uint32_t flags);

//getTmpOptFlags : we'll see

void Sphero::setDeviceMode(uint8_t value = 0);
//01h will set to user hack mode,
// will implement when we will know the usage of this mode

//getDeviceMode

void Sphero::runMacro(uint8_t id);

//void saveMacro(Macro macro);
*/

/**
 * @brief onConnect : Event thrown on Sphero connection
 * @param callback : The callback function to assign to this event
 *			Return type : void
 *			Parameters : none (void)
 */
void Sphero::onConnect(callback_connect_t callback)
{
	_connect_handler.addActionListener(callback);
}//END onConnect


/**
 * @brief onDisconnect : Event thrown on Sphero disconnection
 * @param callback : The callback function to assign to this event
 *			Return type : void
 *			Parameters : none (void)
 */
void Sphero::onDisconnect(callback_disconnect_t callback)
{
	_disconnect_handler.addActionListener(callback);
}//END onDisconnect


/**
 * @brief onCollision : Event thrown when the Sphero detects a collision
 * @param callback : The callback function to assign to this event
 *			Return type : void
 *			Parameters : spherocoord_t xCoord, spherocoord_t yCoord
 */
void Sphero::onCollision(callback_collision_t callback)
{
	_collision_handler.addActionListener(callback);
}//END onCollision
