#include "include/dmesg.h"
#include "include/lfb.h"
#include <smk/inttypes.h>
#include "include/multiboot.h"



static uint32_t *lfb_Ptr	= NULL;
static uint32_t lfb_Width  	= 0;
static uint32_t lfb_Height 	= 0;
static uint32_t lfb_Depth  	= 0;
static uint32_t lfb_Type 	= 0;
	



void init_lfb(  multiboot_info_t *multiboot_info )
{

	if ( (multiboot_info->flags & (1<<11)) == 0 )
	{
	    lfb_Width  	= 0;
	    lfb_Height 	= 0;
	    lfb_Depth 	= 0;
	    lfb_Type 	= 0;
	    lfb_Ptr	 	= NULL;
		dmesg( "VBE LFB information from multiboot info is not available.\n" );
	}
	else
	{
		unsigned int x,y;
		struct vbe_mode *mode_info;

	    mode_info 	= (struct vbe_mode*) multiboot_info->vbe_mode_info;
	    lfb_Width  	= mode_info->x_resolution;
	    lfb_Height 	= mode_info->y_resolution;
	    lfb_Depth 	= mode_info->bits_per_pixel;
	    lfb_Type 	= 0;
	    lfb_Ptr 	= (uint32_t*)mode_info->phys_base;
	
		dmesg( "VBE LFB information: %i x %i x %i at lfb %x\n",
					lfb_Width,
					lfb_Height,
					lfb_Depth,
					lfb_Ptr );
     
	 	for ( x = 0; x < lfb_Width; x++ )
	 		for ( y = 0; y < lfb_Height; y++ )
				lfb_Ptr[ lfb_Width * y + x ] = 0x340034;
	}
}




void 	get_lfb( uint32_t **ptr, 
					uint32_t *width, 
					uint32_t *height )
{
	*ptr 	= lfb_Ptr;
	*width 	= (uint32_t)lfb_Width;
	*height = (uint32_t)lfb_Height;
}





