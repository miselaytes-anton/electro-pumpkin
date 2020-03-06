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
#include <libraries/WriteFile/WriteFile.h>

WriteFile file1;
WriteFile file2;

bool setup(BelaContext *context, void *userData)
{
	if(!context->flags & BELA_FLAG_INTERLEAVED)
	{
		fprintf(stderr, "Error: this example requires that we use interleaved buffers\n");
		return false;
	}

	if(context->analogInChannels < 5)
	{
		fprintf(stderr, "Error: this example requires at least 5 analog inputs to be enabled\n");
		return false;
	}

	file1.setup("out.bin"); //set the file name to write to
	file1.setEchoInterval(10000); // only print to the console every 10000 calls to log
	file1.setFileType(kBinary);
	// set the format that you want to use for your output.
	// Please use %f only (with modifiers).
	// When in binary mode, this is used only for echoing to console
	file1.setFormat("binary: %.4f %.4f\n");

	file2.setup("out.m"); //set the file name to write to
	file2.setHeader("myvar=[\n"); //set one or more lines to be printed at the beginning of the file
	file2.setFooter("];\n"); //set one or more lines to be printed at the end of the file
	file2.setFormat("%.4f %.4f\n"); // set the format that you want to use for your output. Please use %f only (with modifiers)
	file2.setFileType(kText);
	file2.setEchoInterval(10000); // only print to the console 1 line every other 10000

	return true;
}

void render(BelaContext *context, void *userData)
{
	static int count = 0;
	for(unsigned int n = 0; n < context->analogFrames; ++n) {
		// We are accessing the analogIn buffers directly instead of using the analogRead() function
		// This way we can get a pointer to it and pass it to `WriteFile::log()` directly
		file1.log(&(context->analogIn[n*context->analogFrames]), 4); // log an array of values: the first 4 channels from analog input
	}
	file2.log(count);
	file2.log(analogRead(context, 0, 0)); // log a single value from channel 4
	count += context->analogFrames;
}

void cleanup(BelaContext *context, void *userData)
{
}


/**
\example logging-sensors/render.cpp

Logging Sensor Data
---------------------------

This sketch demonstrates how to log sensor data for later processing or analysis.
The file can be written as either a binary file (using 32-bit floats) or
as a formatted text file (again, only using floats).
An optional header and footer can be added to text files, to make your file
immediately ready to be opened in your favourite data analysis program.

Make sure you do not fill up your disk while logging.
Current disk usage can be obtained by typing at the console:

    $ df -h .

While the space occupied by a given file can be obtained with

    $ ls -lh path/to/filename

To learn how to obtain more space on your filesystem, check out the [wiki](https://github.com/BelaPlatform/Bela/wiki/Manage-your-SD-card#resizing-filesystems-on-an-existing-sd-card).

If you write text files, it is a good idea keep your lines short,
as this usually makes it faster for other programs to process the file.
The text file in this example breaks a line after each sample is logged (note the
`\n` at the end of the format string).

If you plan to write large amount of data, binary files are preferrable as they
take less space on disk and are therefore less resource-consuming to write.
They are also generally faster to read and parse than text files.

It is safe to call WriteFile::log() from the audio thread, as the data is added
to a buffer which is then processed in the background and written to disk from
a lower-priority non-realtime task.

The files generated by this program can be opened, e.g.: in GNU Octave or Matlab, with

```
filename = 'out.bin';
numChannels = 4; % num of channels in the binary file
fid=fopen([filename],'r');
A = fread(fid, 'float'); % load in A the values logged to the binary file
fclose(fid);
A = vec2mat(A, numChannels); % transform the array A into a matrix with numChannels columns

run('out.m'); % this will create a variable myar with the data logged to the text file
T =  myvar(:,1); % the first column of the text file is timestamp
plot(1:size(A, 1), A, T, myvar(:,2));
legend('analog0','analog1', 'analog2', 'analog3', 'analog4')
```

*/
