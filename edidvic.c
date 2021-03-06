#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>

#include"vic_timings.h"

enum MODE {
    MODE_UNKNOWN=0,
    MODE_VIC=1,
    MODE_DMT=2
};

const char *program_name;


void help(const char *section)
{
    if(section==NULL)
    {
        fprintf(stderr, "Usage: %s <command> [arguments]\n", program_name);
        fprintf(stderr, "  Commands: vic <vic id>\n");
    }
    else if(strcmp(section,"vic")==0)
    {
        fprintf(stderr, "Usage: %s vic <vic id>\n", program_name);
        fprintf(stderr, "  Print the CEA-861 mode information for a given VIC id\n");
    }
    else if(strcmp(section,"dmt")==0)
    {
        fprintf(stderr, "Usage: %s dmt <dmt id>\n", program_name);
        fprintf(stderr, "  Print the VESA DMT mode information for a given DMT id\n");
    }
}

void print_display_mode(const struct drm_display_mode *dm)
{
    float framerate = (dm->clock * 1000.0) / (dm->htotal * dm->vtotal);
    if(dm->flags & DRM_MODE_FLAG_DBLCLK)
    {
        printf("  Pixel Clock: %d.%d MHz\n", dm->clock/1000, dm->clock % 1000);
        printf("   Frame Rate: %f Hz\n", framerate);
    }
    else
    {
        printf("  Pixel Clock: %d.%d MHz (*2 = %d.%d MHz)\n", 
            dm->clock/1000, dm->clock%1000,
            dm->clock*2/1000, dm->clock*2%1000);
        printf("   Frame Rate: %f Hz (*2 = %f Hz)\n", framerate, framerate*2.0);
    }
	printf("     H Active: %d\n", dm->hdisplay);
	printf("H Front Porch: %d\n", dm->hsync_start-dm->hdisplay);
	printf("       H Sync: %d\n", dm->hsync_end-dm->hsync_start);
	printf(" H Back Porch: %d\n", dm->htotal-dm->hsync_end);
    printf("      H Total: %d\n", dm->htotal);
	printf("     V Active: %d\n", dm->vdisplay);
	printf("V Front Porch: %d\n", dm->vsync_start-dm->vdisplay);
	printf("       V Sync: %d\n", dm->vsync_end-dm->vsync_start);
	printf(" V Back Porch: %d\n", dm->vtotal-dm->vsync_end);
    printf("      V Total: %d\n", dm->vtotal);
    printf("        Flags:%s%s%s%s%s%s\n",
        dm->flags & DRM_MODE_FLAG_PHSYNC ? " +hsync" : " -hsync",
        dm->flags & DRM_MODE_FLAG_PVSYNC ? " +vsync" : " -vsync",
        dm->flags & DRM_MODE_FLAG_INTERLACE ? " +interlace" : "",
        dm->flags & DRM_MODE_FLAG_DBLSCAN ? " +dblscan" : "",
        dm->flags & DRM_MODE_FLAG_DBLCLK ? " +dblclk" : "",
        dm->flags & DRM_MODE_FLAG_CSYNC ? (dm->flags & DRM_MODE_FLAG_PCSYNC ? " +pcsync" : " -pcsync" ) : ""
    );
    printf("Active Pixels: %d\n", dm->hdisplay * dm->vdisplay);
    printf(" Total Pixels: %d\n", dm->htotal * dm->vtotal);
    printf("   8b10b Rate: %d.%d MHz (/25 = %d.%d MHz)\n", (dm->clock * 10) / 1000 , (dm->clock * 10) % 1000, (dm->clock * 10 / 25) / 1000, (dm->clock * 10 / 25) % 1000);
}

int convert_number(const char *arg)
{
    char *endptr=NULL;
    if(strlen(arg)>2 && arg[0]=='0' && (arg[1]=='x' || arg[1]=='X'))
    {
        return (int)strtol(arg+2,&endptr,16);
    }
    return (int)strtol(arg,&endptr,10);
}


int command_dmt(int argc, char **argv)
{
    if(argc != 1)
    {
        help("dmt");
        return 1;
    }
    int dmtid = convert_number(argv[0]);

    const struct drm_display_mode *dm=NULL;
    if(dmtid>0 && dmtid<=88)
    {
        dm = drm_dmt_modes+dmtid;
    }
    else
    {
        printf("DMT ID out of range (1-88)\n");
        return 1;
    }

    print_display_mode(dm);

}
int command_vic(int argc, char **argv)
{
    if(argc != 1)
    {
        help("vic");
        return 1;
    }
    int vic = convert_number(argv[0]);

    const struct drm_display_mode *dm=NULL;
    if(vic>0 && vic<=127)
    {
        dm = edid_cea_modes+vic;
    }
    else if (vic>=193 && vic<=219)
    {
        dm = edid_cea_modes_193+(vic-193);
    }
    else
    {
        fprintf(stderr,"VIC ID out of range (1-127, 193-219)\n");
        return 1;
    }

    print_display_mode(dm);

    return 0;
}

int main(int argc, char **argv)
{
    int opt, vic;
    enum MODE mode = MODE_UNKNOWN;
    program_name = argv[0];

    if(argc<=1)
    {
        help(NULL);
        return 1;
    }

    if(strcmp(argv[1],"vic")==0)
    {
        mode = MODE_VIC;
    }
    else if(strcmp(argv[1],"dmt")==0)
    {
        mode = MODE_DMT;
    }

    switch(mode)
    {
    case MODE_VIC:
        return command_vic(argc-2, argv+2);

    case MODE_DMT:
        return command_dmt(argc-2, argv+2);

    default:
        help(NULL);
    }
    return 1;
}