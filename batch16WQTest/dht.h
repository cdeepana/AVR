#define F_CPU 1000000UL

#ifndef DHT_H_
#define DHT_H_
//functions
extern int8_t dht_gettemperature(int8_t *temperature);
extern int8_t dht_gethumidity(int8_t *humidity);
extern int8_t dht_gettemperaturehumidity(int8_t *temperature, int8_t *humidity);
#endif

#include <stdio.h>
#include <string.h>
#include <avr/io.h>
#include <util/delay.h>

//get data from sensor
int8_t dht_getdata(int8_t *temperature, int8_t *humidity) {
	uint8_t bits[5];
	uint8_t i,j = 0;

	memset(bits, 0, sizeof(bits));

	//reset port
	DDRD |= (1<<PIND6); //output
	PORTD |= (1<<PIND6); //high
	_delay_ms(100);

	//send request
	PORTD &= ~(1<<PIND6); //low
	_delay_ms(18);
	PORTD |= (1<<PIND6); //high
	DDRD &= ~(1<<PIND6); //input
	_delay_us(40);

	//check start condition 1
	if((PIND & (1<<PIND6))) {
		return -1;
	}
	_delay_us(80);
	//check start condition 2
	if(!(PIND & (1<<PIND6))) {
		return -1;
	}
	_delay_us(80);

	//read the data
	uint16_t timeoutcounter = 0;
	for (j=0; j<5; j++) { //read 5 byte
		uint8_t result=0;
		for(i=0; i<8; i++) {//read every bit
			timeoutcounter = 0;
			while(!(PIND & (1<<PIND6))) { //wait for an high input (non blocking)
				timeoutcounter++;
				if(timeoutcounter > 200) {
					return -1; //timeout
				}
			}
			_delay_us(30);
			if(PIND & (1<<PIND6)) //if input is high after 30 us, get result
				result |= (1<<(7-i));
			timeoutcounter = 0;
			while(PIND & (1<<PIND6)) { //wait until input get low (non blocking)
				timeoutcounter++;
				if(timeoutcounter > 200) {
					return -1; //timeout
				}
			}
		}
		bits[j] = result;
	}

	//reset port
	DDRD |= (1<<PIND6); //output
	PORTD |= (1<<PIND6); //low
	_delay_ms(100);

	//check checksum
	if ((uint8_t)(bits[0] + bits[1] + bits[2] + bits[3]) == bits[4]) {
		//return temperature and humidity
		*temperature = bits[2];
		*humidity = bits[0];
		return 0;
	}
	return -1;
}

// get temperature
int8_t dht_gettemperature(int8_t *temperature) {
	int8_t humidity = 0;
	return dht_getdata(temperature, &humidity);
}

// get humidity
int8_t dht_gethumidity(int8_t *humidity) {
	int8_t temperature = 0;
	return dht_getdata(&temperature, humidity);
}

//get temperature and humidity
	int8_t dht_gettemperaturehumidity(int8_t *temperature, int8_t *humidity) {
	return dht_getdata(temperature, humidity);
}


