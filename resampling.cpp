
#include <stdio.h>
#include <stdlib.h>
// #include "resample/unistd.h"
#include <string.h>
#include <math.h>

#include "resample/config.h"
#include "resample/samplerate.h"

// #if (HAVE_SNDFILE)


//#include "sndfile.h"

//#define DEFAULT_CONVERTER SRC_SINC_MEDIUM_QUALITY

#define	BUFFER_LEN		4096	/*-(1<<16)-*/

/*
static void usage_exit (const char *progname) ;
static sf_count_t sample_rate_convert (SNDFILE *infile, SNDFILE *outfile, int converter, double src_ratio, int channels, double * gain) ;
static double apply_gain (float * data, long frames, int channels, double max, double gain) ;
*/

void resample_test()
{ 
	static ARES input [BUFFER_LEN] ;
	static ARES output [BUFFER_LEN] ;

	SRC_STATE	*src_state ;
	SRC_DATA	src_data ;
	int			error ;
	double		max = 0.0 ;
	
	int	output_count = 0 ;

	int converter=SRC_SINC_BEST_QUALITY;
	int channels=2;

	double src_ratio=44100.0/48000.0;

	/*
	sf_seek (infile, 0, SEEK_SET) ;
	sf_seek (outfile, 0, SEEK_SET) ;
*/

	/* Initialize the sample rate converter. */
	if ((src_state = src_new (converter, channels, &error)) == NULL)
	{	printf ("\n\nError : src_new() failed : %s.\n\n", src_strerror (error)) ;
		exit (1) ;
		} ;

	src_data.end_of_input = 0 ; /* Set this later. */

	/* Start with zero to force load in while loop. */
	src_data.input_frames = 0 ;
	src_data.data_in = input ;

	src_data.src_ratio = src_ratio ;

	src_data.data_out = output ;
	src_data.output_frames = BUFFER_LEN /channels ;

	int i=20;

	while (i--)
	{
		/*
		/* If the input buffer is empty, refill it. */
		if (src_data.input_frames == 0)
		{	
			// src_data.input_frames = sf_readf_float (infile, input, BUFFER_LEN / channels) ;
			src_data.input_frames=512;
			src_data.data_in = input ;

			/* The last read will not be a full buffer, so snd_of_input. */
			// if (src_data.input_frames < BUFFER_LEN / channels)
			//src_data.end_of_input = SF_TRUE ;
		} ;

		if ((error = src_process (src_state, &src_data)))
		{	printf ("\nError : %s\n", src_strerror (error)) ;
		exit (1) ;
		} ;

		/* Terminate if done. */
		if (src_data.end_of_input && src_data.output_frames_gen == 0)
			break ;

		//	max = apply_gain (src_data.data_out, src_data.output_frames_gen, channels, max, *gain) ;

		/* Write output. */
		//sf_writef_float (outfile, output, src_data.output_frames_gen) ;
		output_count += src_data.output_frames_gen ;

		src_data.data_in += src_data.input_frames_used * channels ;
		src_data.input_frames -= src_data.input_frames_used ;

	} ;

	src_state = src_delete (src_state) ;

	/*
	if (max > 1.0)
	{	*gain = 1.0 / max ;
		printf ("\nOutput has clipped. Restarting conversion to prevent clipping.\n\n") ;
		output_count = 0 ;
		sf_command (outfile, SFC_FILE_TRUNCATE, &output_count, sizeof (output_count)) ;
		return -1 ;
		} ;
*/

//	return output_count ;
 /* sample_rate_convert */
}
