
/*
 * Author: Armen Karamian <absoboll@gmail.com>
 * Copyright (c) 2015 Armen Karamian.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "grove.h"
#include "jhd1313m1.h"
#include "groveloudness.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <climits>
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <stdint.h>
#include <vector>
#include <cstring>
#include <pthread.h>

/*
 * Monitor audio input and count words
 *
 * Demonstrate the usage of various component types using the UPM library.
 *
 * - analog in: GroveLoudness connected to the Grove Base Shield Port A0
 * - I2C: Jhd1313m1 LCD connected to any I2C on the Grove Base Shield
 *
 * Additional linker flags: -lupm-i2clcd -lupm-grove
 */

/*
 *
 */

void sendMessage()
{
	    int sockfd, portno, n;

	    struct sockaddr_in serv_addr;
	    struct hostent *server;

	    char buffer[256] = "TALKING";
	    portno = 8083;
	    sockfd = socket(AF_INET, SOCK_STREAM, 0);
	    if (sockfd < 0)
	        std::cout << "ERROR opening socket" << std::endl;
	    server = gethostbyname("10.60.0.8");
	    if (server == NULL)
	    {
	        std::cout << "ERROR, no such host\n" << std::endl;
	        return;
	    }

	    serv_addr.sin_family = AF_INET;

	    memcpy((char *)server->h_addr,
	         (char *)&serv_addr.sin_addr.s_addr,
	         server->h_length);

	    serv_addr.sin_port = htons(portno);

	    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)
	        std::cout <<"ERROR connecting"<<std::endl;
	    std::cout << "writing: " << std::endl;

	    n = write(sockfd,buffer,strlen(buffer));

	    if (n < 0)
	    	std::cout << "Error writing to socket" << std::endl;
	    bzero(buffer,256);
	    n = read(sockfd,buffer,255);
	    if (n < 0)
	    	std::cout << "Error writing to socket" << std::endl;

	    printf("%s\n",buffer);
}

int main()
{
	// check that we are running on Galileo or Edison
	mraa_platform_t platform = mraa_get_platform_type();
	if ((platform != MRAA_INTEL_EDISON_FAB_C))
	{
		std::cerr << "Unsupported platform, exiting" << std::endl;
		return MRAA_ERROR_INVALID_PLATFORM;
	}

	// temperature sensor connected to A0 (analog in)
	upm::GroveLoudness* loudness = new upm::GroveLoudness(0);
	// LCD connected to the default I2C bus
	upm::Jhd1313m1* lcd = new upm::Jhd1313m1(0);

	//word count
	int word_count = 0;
	const int LOUDNESS_THRESHOLD = 200;
	const int BUFFER_SIZE = 1100;

	// other helper variables
	uint8_t r, g, b; // resulting LCD backlight color components [0 .. 255]
	std::stringstream row_1, row_2; // LCD rows


	// simple error checking
	if ((loudness == NULL) || (lcd == NULL))
	{
		std::cerr << "Can't create all objects, exiting" << std::endl;
		return MRAA_ERROR_UNSPECIFIED;
	}
/*
	std::cout << "sending message" << std::endl;
	sendMessage();
	std::cout << "message sent" << std::endl;
*/
	while (true)
	{
		bool word_found = false;
		long average = 0;

		while(true)
		{
			for (int i = 0; i < BUFFER_SIZE; i++)
			{
				average += loudness->value();
			}

			average /= BUFFER_SIZE;

			if (average > LOUDNESS_THRESHOLD)
			{
				word_count++;
				word_found = true;

				r = (int)(0);
				g = (int)(255);
				b = (int)(0);


				// apply the calculated result
				lcd->setColor(r, g, b);
			}

			if (average < LOUDNESS_THRESHOLD)
			{

				if (word_found == true)
				{
					word_count++;
				}

				r = 255;
				g = 0;
				b = 0;

				// apply the calculated result
				lcd->setColor(r, g, b);
			}
		}
	}

	return MRAA_SUCCESS;
}
