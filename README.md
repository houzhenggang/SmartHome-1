# SmartHome

Casa domotica per ESP8266 basato sul protocollo MQTT.
Il progetto si divide in nodi: nodi tapparella, nodi temperatura e nodi interruttore.
Ogni nodo, deve avere un nome univoco (topic) per comunicare con il broker MQTT e quindi con l'app Android.
Inviando i comandi al nodo (topic), il nodo risponderà sul topic "ack"

## SmartHome tapparella V 1.0

Comandi da inviare al topic "Tapparella_Topic":

    su            -> comando SU
    giu           -> comando GIU
    stop          -> comando STOP
    t=XX o T=XX   -> XX Indica per quanto tempo la tapparella può restare in azione (in sec.)
    stato         -> restituisce sul topic ACK lo stato dei relè e per quanto tempo la tapparella può restare in azione (in sec.)
    reset         -> pulisce la memoria EEPROM e resetta l'ESP

## SmartHome temperatura V 1.0

Comandi da inviare al topic "Temperatura_Topic":

    man           -> Imposta il termostato in "manuale"
    auto          -> Imposta il termostato in "automatico"
    t=XX o T=XX   -> Imposta il termostato alla temperatura XX
    1on           -> comando ON 1
    1off          -> comando OFF 1
    2on           -> comando ON 2
    2off          -> comando OFF 2
    stato         -> restituisce sul topic ACK lo stato dei relè e per quanto tempo la tapparella può restare in azione (in sec.)
    read          -> legge la temperatura
    reset         -> pulisce la memoria EEPROM e resetta l'ESP

## SmartHome interruttore V 1.0

Comandi da inviare al topic "Interruttore_Topic":

    1on           -> comando ON 1
    1off          -> comando OFF 1
    2on           -> comando ON 2
    2off          -> comando OFF 2
    stato         -> restituisce sul topic ACK lo stato dei relè e per quanto tempo la tapparella può restare in azione (in sec.)
    reset         -> pulisce la memoria EEPROM e resetta l'ESP
