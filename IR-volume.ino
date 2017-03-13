//	This code is distributed under a Creative Commons license
//	This code was written by thecheese429@gmail.com with heavy reference to Adafruit
//	Code version 1.003

#include <Wire.h>
#include <IRLib.h>
template<class T> inline Print &operator <<(Print &obj, T arg) { obj.print(arg); return obj; } 

#define DOWN 	0xE0E0D02F	
#define UP	 	0xE0E0E01F
#define MUTE	0xE0E0F00F
#define POWER	0xE0E040BF


#define DEFAULTVOLUME -30	// default volume used when power key is detected, or when mute is exited using a volume key
#define	HOLDDELAY 400		// amount of time delayed between volume increments when volume up/down is held
#define IRPIN 11			// pin used for the IR receiver
// TO DO: #define MINVOLUME -45		// minimum volume before volume jumps to -78dB


double volume = DEFAULTVOLUME;
int volumeAtMute = DEFAULTVOLUME;
long Previous[2] = {0,0};
unsigned long timeHoldStarted = 0;
unsigned long buttonPressTime[2] = {0,0};
unsigned long timeHeld = 0;
unsigned long timeOfMute = 0;
boolean mute = 0;
boolean keyHeld = 0;
IRrecv IR(IRPIN);
IRdecode decoder;

void setup()
{
	Serial.begin(9600);
	IR.enableIRIn();
	sendVolume();
	
	while(false)	//used for finding the values of the keys
	{
		if(IR.GetResults(&decoder))
		{
			decoder.decode();
			decoder.DumpResults();
			IR.resume();
		}
	}
	// pinMode(13, OUTPUT);
	// digitalWrite(13,LOW);
	
}

double increment()		// determine how fast the volume changes based on how long a key has been pressed
{
	if(keyHeld == 1)
	{
		timeHeld = millis() - timeHoldStarted; 
		// Serial << "button held for " << timeHeld << "mS\n";
	}
	else
	{
		timeHeld = 0;
	}
	
	if(timeHeld > 1800)
	{
		return 2;
	}
	else if(timeHeld > 750)
	{
		return 1;
	}
	else
	{
		return 0.5;
	}
}

void sendVolume()
{
	
	int tens = volume / -10;
	int ones = -1 * (volume + tens * 10) ;
	
	// Serial << "Setting volume to " << volume << "dB. Tens: " << tens << ", Ones: " << ones << "\n";
	// Serial << "\ntens command: ";
	// Serial.print(tens + B11100000, BIN);
	// Serial << " ";
	// Serial.println(ones + B11010000, BIN);
	// Serial.println();
	
	Serial << "Vol: " << volume << " increment: " << increment() << " command: ";
	Serial.print(B10001000, BIN);
	Serial.print(" ");
	Serial.print( tens + B11100000, BIN);
	Serial.print(" ");
	Serial.println( ones + B11010000, BIN);
	
	/*
	Wire.beginTransmission(B10001000);
	Wire.write( tens + B11100000);
	Wire.write( ones + B11010000);
	Wire.endTransmission();
	*/
}

void loop()
{
	
	if(IR.GetResults(&decoder))
	{
		buttonPressTime[1] = buttonPressTime[0];
		buttonPressTime[0] = millis();
		
		decoder.decode();
		// Serial.println(decoder.value, HEX);
		
		
		if( ( Previous[0] == decoder.value || Previous[1] == decoder.value ) && ( buttonPressTime[0] - buttonPressTime[1] < HOLDDELAY )	)	//same key hit twice in a row and interval between same key presses is less than X milliseconds = key is held
		{
			if(keyHeld == 0)
			{
				timeHoldStarted = buttonPressTime[0];
				keyHeld = 1;
			}
		}
		else
		{
			Previous[1] = Previous[0];
			Previous[0] = decoder.value;
			keyHeld = 0;
		}
		
		
		
		
		if(mute == 0)	//not in mute state
		{
			if(decoder.value == MUTE)
			{
				if(millis() - timeOfMute > HOLDDELAY)
				{
					timeOfMute = millis();
					volumeAtMute = volume;
					mute = 1;
					volume = -78;
					Serial << "Entering mute state                               ";
					sendVolume();
				}
				
			}
			else if(decoder.value == UP)
			{
				if(volume < 0)
				{
					if(volume + increment() <= 0)
					{
						volume += increment();
					}
					else
					{
						volume = 0;
					}
					Serial << "Volume UP   detected                              ";
					sendVolume();
				}	
			}
			else if(decoder.value == DOWN)
			{
				if(volume > -79)
				{
					if(volume - increment() >= -79)
					{
						volume -= increment();
					}
					else
					{
						volume = -79;
					}
					Serial << "Volume DOWN detected                              ";
					sendVolume();
				}
			}
			else if(decoder.value == POWER)	//every time the power key is detected, revert to default volume
			{
				volume = DEFAULTVOLUME;
				sendVolume();
			}
		}
		else	// muted 
		{
			if( decoder.value == MUTE) //if the mute button is used to exit the mute state, return to the same volume as before muting
			{
				if(millis() - timeOfMute > HOLDDELAY)
				{
					mute = 0;
					timeOfMute = millis();
					volume = volumeAtMute;
					Serial << "Exiting mute state using MUTE  key                ";
					sendVolume();
				}
			}	
			else if(decoder.value == UP || decoder.value == DOWN || decoder.value == POWER)	//if any other key is used to exit mute, revert to the default volume
			{
				mute = 0;
				volume = DEFAULTVOLUME;
				Serial << "Exiting mute state using other key; default volume";
				sendVolume();
			}
		}
		IR.resume();
		//Serial << "Volume: " << (int)volume << " increment: " << increment() << "\n";
	}
	
	
}