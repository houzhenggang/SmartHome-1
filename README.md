# SmartHome: Casa domotica per ESP8266 basato sul protocollo MQTT.

Il progetto si divide in nodi: nodi tapparella, nodi temperatura e nodi interruttore.  
Ogni nodo comunica attraverso il protocollo MQTT con il broker, che puo' essere locale (LAN) o remoto (internet). Per interagire con i singoli nodi bisogna mandare specifici comandi al nodo (contraddistinto da un topic MQTT univoco).  
Inviando i comandi al nodo (topic), si interagisce con esso, facendogli fare delle operazioni o interrogandolo. Il nodo risponderà sul topic "ack".  
Nella cartella "Android" è presente un'app (ancora in versione beta) dalla quale di possono gestire i vari nodi.  

## SmartHome tapparella V 1.0

Il nodo "tapparella" serve per comandare tapparelle o serrande.  
2 GPIO vengono usati per comandare 2 relè (1 di abilitazione e 1 di inversione del movimento) per il movimento della tapparella.  
2 GPIO vengono usati come ingressi fisici da pulsanti per comandare direttamente il movimento della tapparella.  

Comandi da inviare al topic "Tapparella_Topic":

    su            -> comando SU
    giu           -> comando GIU
    stop          -> comando STOP
    t=XX o T=XX   -> XX Indica per quanto tempo la tapparella può restare in azione (in sec.)
    stato         -> restituisce sul topic ACK lo stato dei relè e per quanto tempo la tapparella può restare in azione (in sec.)
    reset         -> pulisce la memoria EEPROM e resetta l'ESP

## SmartHome temperatura V 1.0

Il nodo "temperatura" serve per comandare apparecchiature per il riscaldamento.  
1 GPIO viene usato per la sonda di temperatura e umidità (DHT22).  
1 GPIO viene usato per comandare il relè termostato (se impostato in AUTO, funziona come un normale termostato, se impostato in MAN, lo si può commutare a piacere).  
1 GPIO viene usato per comandare un relè liberamente gestibile dall'utente.  
2 GPIO vengono usati per interfacciare un display I2C (SSD1306).  

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

Il nodo "interruttore" serve per comandare luci o prese.  
2 GPIO vengono usati per comandare 2 relè liberamente gestibili dall'utente.  

    1on           -> comando ON 1
    1off          -> comando OFF 1
    2on           -> comando ON 2
    2off          -> comando OFF 2
    stato         -> restituisce sul topic ACK lo stato dei relè e per quanto tempo la tapparella può restare in azione (in sec.)
    reset         -> pulisce la memoria EEPROM e resetta l'ESP
