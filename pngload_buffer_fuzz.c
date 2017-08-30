#include <vips/vips.h>

int 
LLVMFuzzerTestOneInput( const guint8 *data, size_t size ) 
{
	VipsImage *image;

	if( !vips_pngload_buffer( (void *) data, size, &image, NULL ) ) {
		double d;

		if( vips_avg( image, &d, NULL ) ) {
			g_object_unref( image ); 

			/* No error return allowed by libfuzzer.
			 */
			return( 0 );
		}

		g_object_unref( image ); 
	}

	return( 0 );
}
