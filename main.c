/* Run one of the crash files found by libFuzzer ... handy for debugging.
 *
 * Compile with:

gcc -g -Wall main.c jpegload_buffer_fuzz.c `pkg-config vips --cflags --libs`

 */

#include <vips/vips.h>

extern int LLVMFuzzerTestOneInput( const guint8 *data, size_t size );

int
main( int argc, char **argv )
{
	void *data;
	size_t size;

	if( !(data = vips__file_read_name( argv[1], ".", &size )) )
		vips_error_exit( NULL ); 

	LLVMFuzzerTestOneInput( data, size );

	g_free( data );

	return( 0 );
}
