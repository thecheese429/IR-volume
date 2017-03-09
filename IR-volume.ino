//	This code is distributed under a Creative Commons license
//	This code was written by thecheese429@gmail.com with heavy reference to Adafruit
//	Code version 1.001

#include <Wire.h>
#include <IRLib.h>
template<class T> inline Print &operator <<(Print &obj, T arg) { obj.print(arg); return obj; } 

#define DOWN 	0xfd48b7
#define UP	 	0xfd48b6
#define MUTE	0xfd48b5
#define POWER	0xfd48b4


#define DEFAULTVOLUME -28	// the default volume used when power key is detected, or when mute is exited using a volume key
#define	HOLDDELAY 200		//the amount of time delayed between volume increments when volume up/down is held

int volume = DEFAULTVOLUME;
int volumeAtMute = DEFAULTVOLUME;
long Previous;
long timeHoldStarted = 0;
long upOrDownPressTime = 0;
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
	
	while(true)
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
	}
	else
	{
		timeHeld = 0;
	}
	
	if(timeHeld > 4000)
	{
		return 6;
	}
	else if(timeHeld > 1400)
	{
		return 4;
	}
	else
	{
		return 2;
	}
}

void sendVolume()
{
	
	int tens = -10 * (int)(volume/10);
	int ones = -1 * (volume - tens * 10) ;
	
	Serial << "Setting volume to " << volume << "dB. Tens: " << tens << ", Ones: " << ones << "\n";
	Serial << "tens command: ";
	Serial.println(tens + B1110000, BIN);
	Serial << "ones command: ";
	Serial.println(ones + B1101000, BIN);
	
	/*
	Wire.beginTransmission(10001000);
	Wire.write( tens + B1110000);
	Wire.write( ones + B1101000);
	Wire.endTransmission();
	*/
}

void loop()
{
	while(mute == 0)
	{
		if(IR.GetResults(&decoder))
		{
			decoder.decode();
			Serial << "Received " << decoder.value << ", of type: " << decoder.decode_type << "\n";
			if( decoder.decode_type == NEC)
			{
				if(decoder.value == 0xFFFFFFFF)
				{
					keyHeld = 1;
					timeHoldStarted = millis();
					decoder.value = Previous;
				}
				else
				{
					keyHeld = 0;
					Previous = decoder.value;
				}
				
				if(decoder.value == MUTE)
				{
					volumeAtMute = volume;
					mute = 1;
					volume = -78;
				}
				else if(decoder.value == UP)
				{
					upOrDownPressTime = millis();
					if(volume < 0)
					{
						volume += increment();
					}	
				}
				else if(decoder.value == DOWN)
				{
					upOrDownPressTime = millis();
					if(volume > -78)
					{
						volume -= increment();
					}
				}
				else if(decoder.value == POWER)
				{
					volume = DEFAULTVOLUME;
				}
				sendVolume();
			}
		}
	}
	
	while(mute == 1)
	{
		if(IR.GetResults(&decoder) == 1)
		{
			if(decoder.value == MUTE);
			{
				volume = volumeAtMute;
				mute = 0;
				sendVolume();
			}
			if(decoder.value == UP || decoder.value == DOWN || decoder.value == POWER)			{
				volume = DEFAULTVOLUME;
				mute = 0;
				sendVolume();
			}
		}
	}
}