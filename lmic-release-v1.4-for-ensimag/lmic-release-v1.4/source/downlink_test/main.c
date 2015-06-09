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

// CONSTANTS

#define TEST_FUNCTION() testCodingRate()

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

static void receive_packet(enum _cr_t newErrcr, u4_t newFreq, enum _sf_t newSF , enum _bw_t newBw , enum _dr_eu868_t newDr){ 
  
    LMIC.dn2Freq = newFreq;
    LMIC.errcr = newErrcr;
    LMIC.dn2Dr = newDr;
    LMIC.rps = setSf(LMIC.rps, newSF);
    LMIC.rps = setBw(LMIC.rps, newBw);   
    LMIC_setRxData();
    
}

static void blinkfunc (osjob_t* j) {
    // toggle LED
    ledstate = !ledstate;
    debug_led(ledstate);
    // reschedule blink job
    os_setTimedCallback(j, os_getTime()+ms2osticks(100), blinkfunc);
}


//////////////////////////////////////////////////
// LMIC EVENT CALLBACK
//////////////////////////////////////////////////

void onEvent (ev_t ev) {
  

    switch(ev) {
        // starting to join network
        case EV_JOINING:
            debug_event(ev);
            debug_str("Started joining.\r\n");
            // start blinking
            blinkfunc(&blinkjob);
            break;

        case EV_JOINED:
            debug_event(ev);
            // join complete
            debug_str("Joined, sending first message.\r\n");
            debug_led(1);
            os_clearCallback(&blinkjob); 
            receive_packet(CR_4_7,868300000, SF7, BW125, DR_SF7);
            break;

        // network joined, session established
        case EV_RXCOMPLETE:
            receive_packet(CR_4_7,868300000, SF7, BW125,DR_SF7);
            break;
    }
}