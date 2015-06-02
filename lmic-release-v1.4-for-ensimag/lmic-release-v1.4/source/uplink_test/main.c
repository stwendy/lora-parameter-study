/*******************************************************************************
 * Copyright (c) 2014-2015 IBM Corporation.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors:
 *    IBM Zurich Research Lab - initial API, implementation and documentation
 *******************************************************************************/

#include "lmic.h"
#include "debug.h"
#include "id.h"

//////////////////////////////////////////////////
// CONFIGURATION (FOR APPLICATION CALLBACKS BELOW)
//////////////////////////////////////////////////


//////////////////////////////////////////////////
// APPLICATION CALLBACKS
//////////////////////////////////////////////////

// provide application router ID (8 bytes, LSBF)
void os_getArtEui (u1_t* buf) {
    memcpy(buf, APPEUI, 8);
}

// provide device ID (8 bytes, LSBF)
void os_getDevEui (u1_t* buf) {
    memcpy(buf, DEVEUI, 8);
}

// provide device key (16 bytes)
void os_getDevKey (u1_t* buf) {
    memcpy(buf, DEVKEY, 16);
}


//////////////////////////////////////////////////
// MAIN - INITIALIZATION AND STARTUP
//////////////////////////////////////////////////
static u1_t numberToSend = 0;
static u1_t endTest = 0;
static u1_t numberTestChange = 0;

// initial job
static void initfunc (osjob_t* j) {
    // reset MAC state
    LMIC_reset();
    // start joining
    LMIC_startJoining();
    // init done - onEvent() callback will be invoked...
}


// application entry point
int main () {
    osjob_t initjob;

    // initialize runtime env
    os_init();
    // initialize debug library
    debug_init();
    // setup initial job
    os_setCallback(&initjob, initfunc);
    // execute scheduled jobs and events
    os_runloop();
    // (not reached)
    return 0;
}


//////////////////////////////////////////////////
// UTILITY JOB
//////////////////////////////////////////////////

static osjob_t blinkjob;
static u1_t ledstate = 0;
u1_t msgData = 1;
static u1_t numTest = 0;

static void blinkfunc (osjob_t* j) {
    // toggle LED
    ledstate = !ledstate;
    debug_led(ledstate);
    // reschedule blink job
    os_setTimedCallback(j, os_getTime()+ms2osticks(100), blinkfunc);
}

void testPacketSize(){
    
    if(numberToSend <10){
        numberToSend++;
        // ICI on fixe les paramètres et les data a envoyer selon numerTestChange
        
        LMIC.message_type=TEST_MESSAGE;
        // schedule transmission (port 1, datalen 1, no ack requested)
        LMIC_setTxData2(1, &numberToSend, numberTestChange*5+1, 0);
        // (will be sent as soon as duty cycle permits)
    }else{
        LMIC.message_type=END_MESSAGE;
        numberTestChange++;
        numberToSend=0;
        // envoi des paramètres du test
        LMIC_setTxData2(1, &numberToSend, 1, 0);
        if(numberTestChange==10){
            endTest=1;
        }
  }
  
}

void testBandWidth(){
    if(numberToSend <100){
        numberToSend++;
        // ICI on fixe les paramètres et les data a envoyer selon numerTestChange
        
        LMIC.message_type=TEST_MESSAGE;
        // schedule transmission (port 1, datalen 1, no ack requested)
        LMIC_setTxData2(1, &msgData, 1, 0);
        // (will be sent as soon as duty cycle permits)
    }else{
        LMIC.message_type=END_MESSAGE;
        numberTestChange++;
        numberToSend=0;
        // envoi des paramètres du test
        LMIC_setTxData2(1, &msgData, 1, 0);
        if(numberTestChange==3){
            endTest=1;
        }
  }
}

void testPower(){
     if(numberToSend <100){
        numberToSend++;
        // ICI on fixe les paramètres et les data a envoyer selon numerTestChange
        
        LMIC.message_type=TEST_MESSAGE;
        // schedule transmission (port 1, datalen 1, no ack requested)
        LMIC_setTxData2(1, &msgData, 1, 0);
        // (will be sent as soon as duty cycle permits)
    }else{
        LMIC.message_type=END_MESSAGE;
        numberTestChange++;
        numberToSend=0;
        // envoi des paramètres du test
        LMIC_setTxData2(1, &msgData, 1, 0);
        if(numberTestChange==10){
            endTest=1;
        }
  }
}

void testSpreadingFactor(){
    if(numberToSend <100){
        numberToSend++;
        // ICI on fixe les paramètres et les data a envoyer selon numerTestChange
        
        LMIC.message_type=TEST_MESSAGE;
        // schedule transmission (port 1, datalen 1, no ack requested)
        LMIC_setTxData2(1, &msgData, 1, 0);
        // (will be sent as soon as duty cycle permits)
    }else{
        LMIC.message_type=END_MESSAGE;
        numberTestChange++;
        numberToSend=0;
        // envoi des paramètres du test
        LMIC_setTxData2(1, &msgData, 1, 0);
        if(numberTestChange==6){
            endTest=1;
        }
  }
}

void testCodingRate(){
    if(numberToSend <100){
        numberToSend++;
        // ICI on fixe les paramètres et les data a envoyer selon numerTestChange

        LMIC.message_type=TEST_MESSAGE;
        // schedule transmission (port 1, datalen 1, no ack requested)
        LMIC_setTxData2(1, &msgData, 1, 0);
        // (will be sent as soon as duty cycle permits)
    }else{
        LMIC.message_type=END_MESSAGE;
        numberTestChange++;
        numberToSend=0;
        // envoi des paramètres du test
        LMIC_setTxData2(1, &msgData, 1, 0);
        if(numberTestChange==4){
            endTest=1;
        }
  }  
  
}

void sendNextMessage() {
   // test choice
    switch(numTest){
        case 0 :
            testPacketSize();
            break;
        case 1 :
            testBandWidth();
            break;
        case 2 :
            testPower();
            break;
        case 3 :
            testSpreadingFactor();
            break;
        case 4 :
            testCodingRate();
            break;  
  }
}

//////////////////////////////////////////////////
// LMIC EVENT CALLBACK
//////////////////////////////////////////////////

void onEvent (ev_t ev) {
    debug_event(ev);

    switch(ev) {
        // starting to join network
        case EV_JOINING:
            debug_str("Started joining.\r\n");
            // start blinking
            blinkfunc(&blinkjob);

            break;

        case EV_JOINED:
            // join complete
            debug_str("Joined, sending first message.\r\n");
            debug_led(1);
            os_clearCallback(&blinkjob);
            sendNextMessage();

            break;

        // network joined, session established
        case EV_TXCOMPLETE:
            if (endTest==1){
                break;
            }
            if (LMIC.dataLen) { // data received in rx slot after tx
                debug_str("Received response message:\r\n");
                debug_buf(LMIC.frame+LMIC.dataBeg, LMIC.dataLen);
                //debug_str("Sending message.\r\n");
                //sendNextMessage();
            } else {
                // nothing received after sending something
                debug_str("No message received after sending data, waiting.\r\n");
            }
            sendNextMessage();            
            break;
    }
}
