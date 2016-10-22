# SmartHome: Domotic house  ESP8266 based on MQTT protocol.

The project is divided into nodes: shutter nodes, temperature nodes and switch nodes.  
Each node communicates through MQTT protocol with the broker, which can be local (LAN) or remote (Internet). To interact with individual nodes, you have to send specific commands to the node (identified by a unique MQTT topic).  
By sending commands to the node (topic), you'll be interacting with it, making him do the operations or questioning. The node will respond on the topic "ack".  
In the "Android" folder is an app (still in beta) from where the can handle the various nodes.

## SmartHome tapparella V 1.0

The "shutter" node is used to control blinds or shutters.
2 GPIO are used for control of 2 relays (1 enable and 1 reversing) for the movement of the shutter.
2 GPIO are used as physical inputs from buttons to directly control the movement of the shutter.

Commands to be sent to the topic "Tapparella_Topic":

    su -> UP command  
    giu -> DOWN command  
    stop -> STOP command  
    t=XX o T=XX -> XX Indicates how long the shutter can remain in action (in sec.)  
    stato -> returns the ACK topic relay status and how long the shutter can remain in action (in sec.)  
    reset -> cleans the EEPROM memory and resets the ESP  

## SmartHome temperatura V 1.0

The "temperature" node is used to control equipment for heating.
1 GPIO is used for the temperature and humidity probe (DHT22).
1 GPIO is used to control the thermostat relay (if set to AUTO, it works as a normal thermostat, when set to MAN, you can switch it manually).
1 GPIO is used to control a relay freely manageable by the user.
2 GPIO are used to interface to an I2C display (SSD1306).

Commands to be sent to the topic "Temperatura_Topic":

    man -> Set the thermostat in the "manual"  
    Auto -> Set the thermostat in the "auto"  
    t=XX o T=XX -> Set the thermostat to the temperature XX  
    1on -> ON 1 command  
    1off -> command OFF 1  
    2on -> ON 2 command  
    2off -> OFF 2 command  
    stato -> returns the ACK topic relay status and thermostat  
    read -> reads temperature  
    reset -> cleans the EEPROM memory and resets the ESP  

## SmartHome interruttore V 1.0

Commands to be sent to the topic "Interruttore_Topic":

The "switch" node is used to control lights or sockets.
2 GPIO are used for controlling 2 relay freely managed by the user.

    1on -> ON 1 command  
    1off -> command OFF 1  
    2on -> ON 2 command  
    2off -> OFF 2 command  
    stato -> returns the ACK topic relay status   
    reset -> cleans the EEPROM memory and resets the ESP  
