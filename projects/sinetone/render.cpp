/*
 ____  _____ _        _
| __ )| ____| |      / \
|  _ \|  _| | |     / _ \
| |_) | |___| |___ / ___ \
|____/|_____|_____/_/   \_\

The platform for ultra-low latency audio and sensor processing

http://bela.io

A project of the Augmented Instruments Laboratory within the
Centre for Digital Music at Queen Mary University of London.
http://www.eecs.qmul.ac.uk/~andrewm

(c) 2016 Augmented Instruments Laboratory: Andrew McPherson,
	Astrid Bin, Liam Donovan, Christian Heinrichs, Robert Jack,
	Giulio Moro, Laurel Pardue, Victor Zappi. All rights reserved.

The Bela software is distributed under the GNU Lesser General Public License
(LGPL 3.0), available here: https://www.gnu.org/licenses/lgpl-3.0.txt
*/

#include <Bela.h>
#include <libraries/Oscillator/Oscillator.h>
#define NUM_TOUCH_PINS 12
float gFrequencies[NUM_TOUCH_PINS] = {261.63, 293.66, 329.63, 349.23,
                                      392.00, 440.00, 493.88, 523.25,
                                      587.33, 659.25, 698.25, 783.99};
Oscillator oscillators[NUM_TOUCH_PINS];


Oscillator osc;

bool setup(BelaContext *context, void *userData)
{
	for (unsigned int i = 0; i < NUM_TOUCH_PINS; i++) {
		oscillators[i] = Oscillator{gFrequencies[i], context->audioSampleRate, Oscillator::osc_type::square};
	}
	return true;
}

void render(BelaContext *context, void *userData)
{
	for(unsigned int n = 0; n < context->audioFrames; n++) {
	    float out = 0.0;
		for (unsigned int i = 0; i < NUM_TOUCH_PINS; i++) {
			out += oscillators[i].process();
		}
		for(unsigned int channel = 0; channel < context->audioOutChannels; channel++) {
			audioWrite(context, n, channel, out * 0.08);
		}
	}
}

void cleanup(BelaContext *context, void *userData)
{

}
