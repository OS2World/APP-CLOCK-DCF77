#include <stdio.h>
#include <conio.h>
#include <dos.h>


int volatile test_timeset;
int volatile test_hour, test_minute;
int volatile test_date, test_month, test_year, test_weekday;



/* --------------------------------------------------------------------------

  dcf77_settime()
  ~~~~~~~~~~~~~~~
  This is function is called from the timer interrupt when a valid time
  has been received from DCF77.

----------------------------------------------------------------------------*/

#pragma argsused

void dcf77_settime (int hour,		/* 0-23 */
		    int minute,		/* 0-59 */
		    int date,		/* 1-31 */
		    int month,		/* 1-12 */
		    int year,		/* 0-99 */
		    int weekday)	/* 1-7, 1=Monday */
{
  /* Insert your code here :-) */
  test_hour    = hour;
  test_minute  = minute;
  test_date    = date;
  test_month   = month;
  test_year    = year;
  test_weekday = weekday;
  test_timeset = 1;
}




/* --------------------------------------------------------------------------

  DCF77 Outer state machine
  ~~~~~~~~~~~~~~~~~~~~~~~~~
  The outer state machine receives '0's, '1's, and minute synchronisation
  events from the inner state machine and assembles this information to a
  valid time and date.  When a valid time and date has been received, the
  function dcf77_settime() is called with the received time and date.

--------------------------------------------------------------------------- */

/* These events are sent from the inner to the outer state machine */
#define EVENT_ZEROBIT		0x0000
#define EVENT_ONEBIT		0xFFFF
#define EVENT_MINUTESYNC	0x0001


/* Outer state machine states.  For state 0-58, the state numbers correspond
   to the bit number that is expected next. */
#define OSM_START	0	/* Startbit				*/
#define OSM_FUTURE01	1	/* For future use, currently '0'	*/
#define OSM_FUTURE02	2	/* For future use, currently '0'	*/
#define OSM_FUTURE03	3	/* For future use, currently '0'	*/
#define OSM_FUTURE04	4	/* For future use, currently '0'	*/
#define OSM_FUTURE05	5	/* For future use, currently '0'	*/
#define OSM_FUTURE06	6	/* For future use, currently '0'	*/
#define OSM_FUTURE07	7	/* For future use, currently '0'	*/
#define OSM_FUTURE08	8	/* For future use, currently '0'	*/
#define OSM_FUTURE09	9	/* For future use, currently '0'	*/
#define OSM_FUTURE10	10	/* For future use, currently '0'	*/
#define OSM_FUTURE11	11	/* For future use, currently '0'	*/
#define OSM_FUTURE12	12	/* For future use, currently '0'	*/
#define OSM_FUTURE13	13	/* For future use, currently '0'	*/
#define OSM_FUTURE14	14	/* For future use, currently '0'	*/
#define OSM_ANTENNA	15	/* Using backup antenna			*/
#define OSM_BEFORE_DST	16	/* Is set one hour before switching between normal/DST */
#define OSM_DST		17	/* Daylight saving time active (summer)	*/
#define OSM_NO_DST	18	/* Daylight saving time not active (winter) */
#define OSM_LEAP_SECOND	19	/* Announcing leap second		*/
#define OSM_TIMESTART	20	/* Start bit for time info (always 1)	*/
#define OSM_MIN01	21	/* Minute bit 0				*/
#define OSM_MIN02	22	/* Minute bit 1				*/
#define OSM_MIN04	23	/* Minute bit 2				*/
#define OSM_MIN08	24	/* Minute bit 3				*/
#define OSM_MIN10	25	/* Minute bit 4				*/
#define OSM_MIN20	26	/* Minute bit 5				*/
#define OSM_MIN40	27	/* Minute bit 6				*/
#define OSM_PARITY1	28	/* Parity of bit 21-27			*/
#define OSM_HOUR01	29	/* Hour bit 0				*/
#define OSM_HOUR02	30	/* Hour bit 1				*/
#define OSM_HOUR04	31	/* Hour bit 2				*/
#define OSM_HOUR08	32	/* Hour bit 3				*/
#define OSM_HOUR10	33	/* Hour bit 4				*/
#define OSM_HOUR20	34	/* Hour bit 5				*/
#define OSM_PARITY2	35	/* Parity of bit 29-34			*/
#define OSM_DATE01	36	/* Date bit 0				*/
#define OSM_DATE02	37	/* Date bit 1				*/
#define OSM_DATE04	38	/* Date bit 2				*/
#define OSM_DATE08	39	/* Date bit 3				*/
#define OSM_DATE10	40	/* Date bit 4				*/
#define OSM_DATE20	41	/* Date bit 5				*/
#define OSM_WEEKDAY01	42	/* Weekday bit 0			*/
#define OSM_WEEKDAY02	43	/* Weekday bit 1			*/
#define OSM_WEEKDAY04	44	/* Weekday bit 2			*/
#define OSM_MONTH01	45	/* Month bit 0				*/
#define OSM_MONTH02	46	/* Month bit 1				*/
#define OSM_MONTH04	47	/* Month bit 2				*/
#define OSM_MONTH08	48	/* Month bit 3				*/
#define OSM_MONTH10	49	/* Month bit 4				*/
#define OSM_YEAR01	50	/* Year bit 0				*/
#define OSM_YEAR02	51	/* Year bit 1				*/
#define OSM_YEAR04	52	/* Year bit 2				*/
#define OSM_YEAR08	53	/* Year bit 3				*/
#define OSM_YEAR10	54	/* Year bit 4				*/
#define OSM_YEAR20	55	/* Year bit 5				*/
#define OSM_YEAR40	56	/* Year bit 6				*/
#define OSM_YEAR80	57	/* Year bit 7				*/
#define OSM_PARITY3	58	/* Parity of bit 36-57			*/
#define OSM_MINUTESYNC	59	/* Awaiting minute sync after correctly received time */
#define OSM_RESYNC	60	/* Awaiting minute sync after an error	*/

static int osm_state;		/* one of the above states */



static void dcf77_osm (unsigned int event)
{
  static unsigned int minute;
  static unsigned int hour;
  static unsigned int date;
  static unsigned int weekday;
  static unsigned int month;
  static unsigned int year;
  static unsigned int parity;

  switch (osm_state)
  {
    case OSM_START:
    case OSM_FUTURE01:
    case OSM_FUTURE02:
    case OSM_FUTURE03:
    case OSM_FUTURE04:
    case OSM_FUTURE05:
    case OSM_FUTURE06:
    case OSM_FUTURE07:
    case OSM_FUTURE08:
    case OSM_FUTURE09:
    case OSM_FUTURE10:
    case OSM_FUTURE11:
    case OSM_FUTURE12:
    case OSM_FUTURE13:
    case OSM_FUTURE14:
    case OSM_ANTENNA:
    case OSM_BEFORE_DST:
    case OSM_DST:
    case OSM_NO_DST:
    case OSM_LEAP_SECOND:
	   /* These bits aren't used */
	   break;


    case OSM_TIMESTART:
	   /* This bit must be '1' */
	   minute  = 0;
	   hour    = 0;
	   date    = 0;
	   weekday = 0;
	   month   = 0;
	   year    = 0;
	   if (event == EVENT_ONEBIT)
	     osm_state = OSM_MIN01;		/* startbit OK, continue */
	   else if (event == EVENT_MINUTESYNC)
	     osm_state = OSM_START;		/* unexpected minute sync */
	   else
	     osm_state = OSM_RESYNC;		/* startbit bad */
	   return;


    case OSM_MIN01:  parity = 0;
		     minute += event & 1;   break;
    case OSM_MIN02:  minute += event & 2;   break;
    case OSM_MIN04:  minute += event & 4;   break;
    case OSM_MIN08:  minute += event & 8;   break;
    case OSM_MIN10:  minute += event & 10;  break;
    case OSM_MIN20:  minute += event & 20;  break;
    case OSM_MIN40:  minute += event & 40;  break;


    case OSM_PARITY1:
	   if (event == parity)
	     osm_state = OSM_HOUR01;		/* parity OK, continue */
	   else if (event == EVENT_MINUTESYNC)
	     osm_state = OSM_START;		/* unexpected minute sync */
	   else
	     osm_state = OSM_RESYNC;		/* parity bad */
	   return;


    case OSM_HOUR01:  parity = 0;
		      hour += event & 1;   break;
    case OSM_HOUR02:  hour += event & 2;   break;
    case OSM_HOUR04:  hour += event & 4;   break;
    case OSM_HOUR08:  hour += event & 8;   break;
    case OSM_HOUR10:  hour += event & 10;  break;
    case OSM_HOUR20:  hour += event & 20;  break;


    case OSM_PARITY2:
	   if (event == parity)
	     osm_state = OSM_DATE01;		/* parity OK, continue */
	   else if (event == EVENT_MINUTESYNC)
	     osm_state = OSM_START;		/* unexpected minute sync */
	   else
	     osm_state = OSM_RESYNC;		/* parity bad */
	   return;


    case OSM_DATE01   :  parity = 0;
			 date   += event & 1;   break;
    case OSM_DATE02   :  date   += event & 2;   break;
    case OSM_DATE04   :  date   += event & 4;   break;
    case OSM_DATE08   :  date   += event & 8;   break;
    case OSM_DATE10   :  date   += event & 10;  break;
    case OSM_DATE20   :  date   += event & 20;  break;
    case OSM_WEEKDAY01:  weekday+= event & 1;   break;
    case OSM_WEEKDAY02:  weekday+= event & 2;   break;
    case OSM_WEEKDAY04:  weekday+= event & 4;   break;
    case OSM_MONTH01  :  month  += event & 1;   break;
    case OSM_MONTH02  :  month  += event & 2;   break;
    case OSM_MONTH04  :  month  += event & 4;   break;
    case OSM_MONTH08  :  month  += event & 8;   break;
    case OSM_MONTH10  :  month  += event & 10;  break;
    case OSM_YEAR01   :  year   += event & 1;   break;
    case OSM_YEAR02   :  year   += event & 2;   break;
    case OSM_YEAR04   :  year   += event & 4;   break;
    case OSM_YEAR08   :  year   += event & 8;   break;
    case OSM_YEAR10   :  year   += event & 10;  break;
    case OSM_YEAR20   :  year   += event & 20;  break;
    case OSM_YEAR40   :  year   += event & 40;  break;
    case OSM_YEAR80   :  year   += event & 80;  break;


    case OSM_PARITY3:
	   if (event == parity)
	     osm_state = OSM_MINUTESYNC;	/* parity OK, continue */
	   else if (event == EVENT_MINUTESYNC)
	     osm_state = OSM_START;		/* unexpected minute sync */
	   else
	     osm_state = OSM_RESYNC;		/* parity bad */
	   return;


    case OSM_MINUTESYNC:
	   if (event == EVENT_MINUTESYNC)
	   {
	     /* Hooray, correct time followed by a minute sync */
	     dcf77_settime (hour, minute, date, month, year, weekday);
	     osm_state = OSM_START;
	   }
	   else
	   {
	     /* Too bad.  What seemed to be a valid time was received, but
		we didn't get the two-second pause after it. */
	     osm_state = OSM_RESYNC;
	   }
	   return;


    case OSM_RESYNC:
	   /* Error condition, try to find the start of a new frame */
	   if (event == EVENT_MINUTESYNC)
	     osm_state = OSM_START;
	   return;
  }

  /* This default processing is performed for all cases that don't return.
     If a minute sync unexpectedly happens in the middle of a frame, the
     state machine is reset to await the start of a new frame.  Otherwise
     (a zerobit or onebit is received), the state machine is advanced to
     await the next bit. */
  parity ^= event;
  if (event == EVENT_MINUTESYNC)
    osm_state = OSM_START;
  else
    osm_state++;
}




/* --------------------------------------------------------------------------

  DCF77 Inner state machine
  ~~~~~~~~~~~~~~~~~~~~~~~~~
  The inner state machine picks bits out of the received DCF77 signal and
  sends them to the outer state machine.
  dcf77_ism() is called from the timer interrupt at 200 Hz.  The parameter
  dcfbit is the value of the received DCF77 signal (zero or non-zero).

  An incoming pulse of 10-125 msec (2-25 samples) sends a '0' to the
  outer state machine.  A pulse of 130-300 msec (26-60 samples) sends a
  '1'.  Pulses shorter than 2 samples or longer than 60 samples cause an
  inner state machine resynchronisation.

  Ideally, a pulse comes either 1 second (normal bit) or 2 seconds (minute
  synchronisation) after the previous pulse.

  If the pause between the start of a pulse and the start of the next one
  is 800-1200 msec (160-240 ticks), nothing special happens.  If the pause
  is 1800-2200 msec (360-440 ticks), a minute synch is sent to the outer
  state machine.  If the pause isn't within these intervals, it is an error,
  and the inner state machine is resynchronized.

  Resynchronizing the inner state machine means waiting for a 0-to-1
  transition of the DCF77 signal, i.e. waiting for the signal being 0,
  and then waiting for it to be 1.

--------------------------------------------------------------------------- */

/* Inner state machine states */
#define ISM_RESYNC0	0
#define ISM_RESYNC1	1
#define ISM_ACTIVE	2
#define ISM_INACTIVE	3

static int ism_state;		/* one of the above states */



static void dcf77_ism (char dcfbit)
{
  static unsigned int ism_ticks; /* ticks from the beginning of the pulse */

  switch (ism_state)
  {
    case ISM_RESYNC0:		/* Resynching - waiting for '0' */
	   if (!dcfbit)
	     ism_state = ISM_RESYNC1;
	   break;


    case ISM_RESYNC1:		/* Resynching - waiting for '1' */
	   if (dcfbit)
	   {
	     /* 0-to-1 edge detected */
	     ism_ticks = 0;
	     ism_state = ISM_ACTIVE;
	   }
	   break;


    case ISM_ACTIVE:		/* '1' is being received */
	   ism_ticks++;
	   if (dcfbit)
	   {
	     /* Still '1' coming in */
	     if (ism_ticks > 60)
	     {
	       /* Incoming '1' for more than 60 ticks!  Something is wrong. */
	       ism_state = ISM_RESYNC0;
	     }
	   }
	   else
	   {
	     /* 1-to-0 edge detected */
	     if (ism_ticks < 2)
	     {
	       /* Pulse of only 1 tick!  Something is wrong. */
	       ism_state = ISM_RESYNC0;
	     }
	     else if (ism_ticks <= 25)
	     {
	       /* Ordinary short pulse.  This is a '0'. */
	       dcf77_osm (EVENT_ZEROBIT);
	       ism_state = ISM_INACTIVE;
	     }
	     else
	     {
	       /* Ordinary long pulse.  This is a '1'. */
	       dcf77_osm (EVENT_ONEBIT);
	       ism_state = ISM_INACTIVE;
	     }
	   }
	   break;


    case ISM_INACTIVE:		/* '0' is being received */
	   ism_ticks++;
	   if (dcfbit)
	   {
	     /* Beginning of a new pulse (0-to-1 edge) detected.  ism_ticks
		is the number of ticks between the beginning of the old
		pulse and the beginning of the new one. */
	     if (ism_ticks < 160)
	     {
	       /* 1-159 ticks, this is too close */
	       ism_state = ISM_RESYNC0;
	     }
	     else if (ism_ticks <= 240)
	     {
	       /* 160-240 ticks, normal interval */
	       ism_ticks = 0;
	       ism_state = ISM_ACTIVE;
	     }
	     else if (ism_ticks < 360)
	     {
	       /* 241-359 ticks, this is illegal */
	       ism_state = ISM_RESYNC0;
	     }
	     else
	     {
	       /* 360-440 ticks, end of minute sync interval */
	       ism_ticks = 0;
	       ism_state = ISM_ACTIVE;
	       dcf77_osm (EVENT_MINUTESYNC);
	     }
	   }
	   else
	   {
	     /* Still '0' coming in */
	     if (ism_ticks >= 440)
	     {
	       /* 440 ticks have passed without seeing a new pulse, the
		  receiver is probably dead */
	       ism_state = ISM_RESYNC0;
	     }
	   }
	   break;
  }
}




void dcf77_init (void)
{
  ism_state = ISM_RESYNC0;
  osm_state = OSM_RESYNC;
}




#define PORTBASE  0x2F8

/* Timer interrupt, called at 200 Hz */

static void interrupt timerintr (void)
{
  dcf77_ism (inportb (PORTBASE+6) & 0x80);
  outportb (0x20, 0x20);

  /* If this was a real program, we would sometimes call the original
     interrupt handler, so that it would end up being called 18.2 times
     per second, as usual. */
}




int main (void)
{
  void interrupt (*oldtimer) (void);
  static char *dayname[7] = { "Monday", "Tuesday", "Wednesday", "Thursday",
			      "Friday", "Saturday", "Sunday" };
  /* Turn on DCF77 receiver */
  outportb (PORTBASE+4, 0x01);

  dcf77_init();

  /* Reprogram timer interrupt to 199.997 Hz */
  disable();
  outportb (0x43, 0x36);
  outportb (0x40, 0x4E);
  outportb (0x40, 0x17);
  enable();

  /* Install our timer interrupt handler */
  oldtimer = getvect (0x08);
  setvect (0x08, timerintr);

  while (!kbhit())
  {
    printf ("ism=%-2d    osm=%-2d\r", ism_state, osm_state);
    if (test_timeset)
    {
      printf ("\n\nReceived Time=%02d.%02d   Date=%02d%02d%02d   Day=%s\n\n",
	      test_hour, test_minute, test_year, test_month, test_date,
	      dayname[test_weekday-1]);
      test_timeset = 0;
    }
  }

  /* Timer interrupt back to 18.2 Hz */
  disable();
  outportb (0x43, 0x36);
  outportb (0x40, 0x00);
  outportb (0x40, 0x00);
  enable();
  setvect (0x08, oldtimer);

  /* Turn off DCF77 receiver */
  outportb (PORTBASE+4, 0x00);

  return 0;
}
