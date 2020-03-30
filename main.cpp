//#define MBED_CONF_RTOS_API_PRESENT 1
#include "mbed.h"
#include "rtos.h"

AnalogOut aout(DAC0_OUT);
AnalogIn ain(A0);
DigitalIn sw3(SW3);
DigitalOut rLED(LED1);
DigitalOut gLED(LED2);
BusOut ssd(D6, D7, D9, D10, D11, D5, D4, D8);

Serial pc(USBTX, USBRX, 115200);
Timer tim;
Timer timdisp;
//Timer tdebug;
Thread sampler;
Thread wavegen;

const char ssdt[10] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F};

float adc = 0;
float adcl = 0;
float adcll = 0;
float timt = 0;
float timtl = 0;
int freq = 100;

int fd = 0;
int fd1 = 0;
int fd2 = 0;
int fd3 = 0;

void thread_wavegen() {
	while(true) {
		while(!sw3) {
			int fwg = freq;
			for(float i = 0; i < 2; i += 0.05) {
				aout = 0.5 + 0.5*sin(3.14159 * i);
				wait(1.0 / (fwg * 40.0));
			}
		}
		wait(0.1);
	}
}

void thread_sampler() {
	tim.start();
	while(true) {
		adc = ain;

		if(adcll < adcl && adcl > adc) {		// peak reached
			
			timt = tim.read();
			if(timtl != 0.0) {
				freq = round(1 / (timt - timtl));
			}

			timtl = timt;
		}

		pc.printf("%1.3f\r\n", adc);
		//pc.printf("%1.3f\r\n", tdebug.read());

		adcll = adcl;
		adcl = adc;

		wait(0.001);
	}
}

int main() {
	//tdebug.start();
	sampler.start(thread_sampler);
	wavegen.start(thread_wavegen);

	while(true) {
		
		if(!sw3) {		// SW3 is pressed
			rLED = 0; gLED = 1;
			timdisp.start();
			
			if(timdisp.read() < 0.05) {
				fd = freq;

				fd1 = fd / 100;
				fd2 = (fd % 100) / 10;
				fd3 = (fd % 10);
			}

			if(fmod(timdisp.read(), 3) < 1) {
				ssd = ssdt[fd1];
			} else if(fmod(timdisp.read(), 3) < 2) {
				ssd = ssdt[fd2];
			} else {
				ssd = ssdt[fd3] | 0x80;
			}
		} else {
			rLED = 1; gLED = 0;
			timdisp.stop();
			timdisp.reset();
		}

		wait(0.1);
	}
}