//	This code is distributed under a Creative Commons license
//	This code was written by thecheese429@gmail.com with heavy reference to Adafruit
//	Code version 1.001

#include <Wire.h>
#include <IRLib.h>
template<class T> inline Print &operator <<(Print &obj, T arg) { obj.print(arg); return obj; } 

#define DOWN 	0xE0E0D02F
#define UP	 	0xE0E0E01F
#define MUTE	0xE0E0F00F
#define POWER	0xE0E040BF


#define DEFAULTVOLUME -28	// the default volume used when power key is detected, or when mute is exited using a volume key
#define	HOLDDELAY 400		//the amount of time delayed between volume increments when volume up/down is held

int volume = DEFAULTVOLUME;
int volumeAtMute = DEFAULTVOLUME;
long Previous[2] = {0,0};
long timeHoldStarted = 0;
long buttonPressTime[2] = {0,0};
long timeHeld = 0;
boolean mute = 0;
boolean keyHeld = 0;
IRrecv IR(11);
IRdecode decoder;

void setup()
{
	Serial.begin(9600);
	IR.enableIRIn();
	sendVolume();
	
	while(false)
	{
		if(IR.GetResults(&decoder))
		{
			decoder.decode();
			decoder.DumpResults();
			IR.resume();
		}
	}
	
}

int increment()
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
		return 3;
	}
	else if(timeHeld > 1000)
	{
		return 2;
	}
	else
	{
		return 1;
	}
}

void sendVolume()
{
	
	int tens = volume / -10;
	int ones = -1 * (volume + tens * 10) ;
	
	//Serial << "Setting volume to " << volume << "dB. Tens: " << tens << ", Ones: " << ones << "\n";
	// Serial << "\ntens command: ";
	// Serial.println(tens + B11100000, BIN);
	// Serial << "ones command: ";
	// Serial.println(ones + B11010000, BIN);
	// Serial.println();
	
	/*
	Wire.beginTransmission(10001000);
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
		
		
		if(( Previous[0] == decoder.value || Previous[1] == decoder.value ) && ( buttonPressTime[0] - buttonPressTime[1] < HOLDDELAY )	)	//same key hit twice in a row and interval between same key presses is less than X milliseconds = key is held
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
				volumeAtMute = volume;
				mute = 1;
				volume = -78;
				sendVolume();
			}
			else if(decoder.value == UP)
			{
				// upOrDownPressTime = millis();
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
					sendVolume();
				}	
			}
			else if(decoder.value == DOWN)
			{
				// upOrDownPressTime = millis();
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
					sendVolume();
				}
			}
			else if(decoder.value == POWER)
			{
				volume = DEFAULTVOLUME;
				sendVolume();
			}
		}
		else
		{
			if(decoder.value == MUTE);
			{
				volume = volumeAtMute;
				sendVolume();
			}
			if(decoder.value == UP || decoder.value == DOWN || decoder.value == POWER)
			{
				volume = DEFAULTVOLUME;
				sendVolume();
			}
			mute = 0;
		}
		IR.resume();
		Serial << "Volume: " << volume << " increment: " << increment() << "\n";
	}
	
	
}