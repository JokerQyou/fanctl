/*
Copyright (c) 2012, Broadcom Europe Ltd
All rights reserved.
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the copyright holder nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*
This file is based on the skeleton of https://git.io/JfiMl , AKA the source of `vsgencmd` tool in Raspbian.
*/

/* ---- Include Files ---------------------------------------------------- */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

// https://github.com/raspberrypi/userland
#include "interface/vmcs_host/vc_vchi_gencmd.h"
#include "interface/vmcs_host/vc_gencmd_defs.h"
// https://github.com/eclipse/mraa
#include "mraa/gpio.h"

// Fan will be turned on when temperature is above THRESHOLD_ON, and turned off when it drops below THRESHOLD_OFF.
#define THRESHOLD_ON  60
#define THRESHOLD_OFF 45
// Fan starts if FAN_VCC_PIN is high, stops if it's low.
#define FAN_VCC_PIN    8  // FIXME
#define SLEEP_INTERVAL 5

// The program stops once flag is set to 0.
volatile sig_atomic_t flag = 1;

void signal_handler(int signum)
{
	if (signum == SIGINT) {
		puts("\nSIGINT received");
		flag = 0;
	}
}

int main( int argc, char **argv )
{
	mraa_gpio_context fan_vcc;

	signal(SIGINT, signal_handler);

	VCHI_INSTANCE_T vchi_instance;
	VCHI_CONNECTION_T *vchi_connection = NULL;

	vcos_init();

	if ( vchi_initialise( &vchi_instance ) != 0)
	{
		fprintf( stderr, "VCHI initialization failed\n" );
		return -1;
	}

	//create a vchi connection
	if ( vchi_connect( NULL, 0, vchi_instance ) != 0)
	{
		fprintf( stderr, "VCHI connection failed\n" );
		return -1;
	}

	vc_vchi_gencmd_init(vchi_instance, &vchi_connection, 1 );

	mraa_init();

	fan_vcc = mraa_gpio_init(FAN_VCC_PIN);
	if (fan_vcc == NULL)
	{
		fprintf(stderr, "Failed to initialize GPIO %d\n", FAN_VCC_PIN);
		goto err_exit;
	}

	if (mraa_gpio_dir(fan_vcc, MRAA_GPIO_OUT) != MRAA_SUCCESS)
	{
		fprintf(stderr, "Failed to set GPIO %d to output\n", FAN_VCC_PIN);
		goto err_exit;
	}


	while(flag) {
		char buffer[ GENCMDSERVICE_MSGFIFO_SIZE ];
		size_t buffer_offset = 0;
		int ret;
		float temperature;

		//reset the string
		buffer[0] = '\0';
		buffer_offset = vcos_safe_strcpy(buffer, "measure_temp", sizeof(buffer), buffer_offset);
		buffer_offset = vcos_safe_strcpy(buffer, " ", sizeof(buffer), buffer_offset);

		//send the gencmd for the argument
		if (( ret = vc_gencmd_send( "%s", buffer )) != 0 )
		{
			printf( "vc_gencmd_send returned %d\n", ret );
		}

		//get + print out the response!
		if (( ret = vc_gencmd_read_response( buffer, sizeof( buffer ) )) != 0 )
		{
			printf( "vc_gencmd_read_response returned %d\n", ret );
		}
		buffer[ sizeof(buffer) - 1 ] = 0;

		if ( buffer[0] != '\0' )
		{
			if ( buffer[ strlen( buffer) - 1] == '\n' )
			{
				// Remove the trailing linebreak
				buffer[strlen(buffer) - 1] = 0;
			}

			// The response is like this: "temp=53.0'C"
			if (strncmp(buffer, "temp=", 5) == 0)
			{
				// Convert to float
				char temp_chars [strlen(buffer) - 5 - 2];
				int fan_on;
				strncpy(temp_chars, buffer+5, sizeof temp_chars);
				temperature = atof(temp_chars);

				fan_on = mraa_gpio_read(fan_vcc);
				if (fan_on == -1)
				{
					fprintf(stderr, "Failed to detect fan state");
					continue;
				}

				// Turn on the fan
				// FIXME
				if (temperature >= THRESHOLD_ON && fan_on == 0)
				{
					fprintf (stdout, "temp=%2.2f°C, fan start\n", temperature);
					if (mraa_gpio_write(fan_vcc, 1) != MRAA_SUCCESS)
					{
						fprintf(stderr, "Failed to start fan\n");
					}
				}
				// Turn off the fan
				if (temperature <= THRESHOLD_OFF && fan_on == 1)
				{
					fprintf (stdout, "temp=%2.2f°C, fan stop\n", temperature);
					if (mraa_gpio_write(fan_vcc, 0) != MRAA_SUCCESS)
					{
						fprintf(stderr, "Failed to stop fan\n");
					}
				}
			}
			else
			{
				fprintf (stderr, "%s\n", buffer);
				return -2;
			}
		}
		sleep(SLEEP_INTERVAL);
	}

	if (mraa_gpio_close(fan_vcc) != MRAA_SUCCESS)
	{
		fprintf(stderr, "Failed to close GPIO %d\n", FAN_VCC_PIN);
		goto err_exit;
	}

	mraa_deinit();
	vc_gencmd_stop();

	//close the vchi connection
	if ( vchi_disconnect( vchi_instance ) != 0)
	{
		fprintf( stderr, "VCHI disconnect failed\n" );
		return -1;
	}
	puts("Exit");
	return 0;

err_exit:
	mraa_deinit();
	vc_gencmd_stop();

	//close the vchi connection
	if ( vchi_disconnect( vchi_instance ) != 0)
	{
		fprintf( stderr, "VCHI disconnect failed\n" );
		return -1;
	}
	return EXIT_FAILURE;
}
