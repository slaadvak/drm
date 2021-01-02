#include <xf86drm.h>
#include <xf86drmMode.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

static int device;
static uint32_t connector_id;
static drmModeModeInfo mode_info;

struct connector {
	drmModeConnector *connector;
	drmModeObjectProperties *props;
	drmModePropertyRes **props_info;
	char *name;
};

#define EXIT(msg) { fputs (msg, stderr); exit (EXIT_FAILURE); }

static void print_connector(drmModeConnector *conn)
{
    if (conn->count_modes != 0)
    {
        for(int i = 0; i < conn->count_modes; i++)
        {
            printf("    mode %2d: %-15s %dHz %ux%u clock:%u\n", i, conn->modes[i].name, conn->modes[i].vrefresh, conn->modes[i].hdisplay, conn->modes[i].vdisplay, conn->modes[i].clock);
        }
    }
    else
    {
        printf("    no valid mode for connector %u\n", conn->connector_id);
    }

    printf("    properties count: %u\n", conn->count_props);

	for(int i=0; i < conn->count_props; i++)
    {
		drmModePropertyPtr prop = drmModeGetProperty(device, conn->props[i]);
		if(!prop)
        {
			printf("    cannot retrieve DRM property (connector_id: %d, prop_id: %d): errorno: %d (%s)\n", conn->connector_id, conn->props[i], errno, strerror(errno));
			continue;
		}

        char* enum_name = NULL;
        // If enum => find enum text
        if (prop->flags & DRM_MODE_PROP_ENUM)
        {
            for (int i = 0; i < prop->count_enums; i++)
            {
                if (prop->enums[i].value == conn->prop_values[i])
                {
                    enum_name = prop->enums[i].name;
                }
            }

            if (!enum_name)
            {
                printf("#\tproperty value (%llu) not found in enum list!\n", conn->prop_values[i]);
            }
        }
        
        printf("    property (#%d):  %s = %llu", prop->prop_id, prop->name, conn->prop_values[i]);
        
        if (enum_name)
        {
            printf(" (%s)", enum_name);
        }
        printf("\n");

		drmModeFreeProperty(prop);
    }

}

static void set_connector_property(drmModeConnector *conn, char* prop_name, uint64_t value)
{
    for(int i=0; i < conn->count_props; i++)
    {
		drmModePropertyPtr prop = drmModeGetProperty(device, conn->props[i]);
		if(!prop)
        {
			printf("    cannot retrieve DRM property (connector_id: %d, prop_id: %d): errorno: %d (%s)\n", conn->connector_id, conn->props[i], errno, strerror(errno));
			continue;
		}

        if(! strcmp(prop->name, prop_name))
        {

            int ret = drmModeConnectorSetProperty(device, conn->connector_id, prop->prop_id, value);
		    printf("Setting '%s' -> %llu - ret: %d :: connector_id: %d prop_id: %d \n", prop_name, value, ret, conn->connector_id, prop->prop_id);
        }
        
        drmModeFreeProperty(prop);
    }
}

static drmModeConnector *find_connector(drmModeRes *resources)
{
	// iterate the connectors
	int i;
	for (i=0; i<resources->count_connectors; i++)
    {
		drmModeConnector *connector = drmModeGetConnector(device, resources->connectors[i]);
		// pick the first connected connector
		if (connector->connection == DRM_MODE_CONNECTED)
        {
			return connector;
		}

		drmModeFreeConnector(connector);
	}
	// no connector found
	return NULL;
}

static void find_display_configuration()
{
	drmModeRes *resources = drmModeGetResources(device);
	// find a connector
	drmModeConnector *connector = find_connector(resources);
	if (!connector) EXIT ("no connector found\n");
	// save the connector_id
	connector_id = connector->connector_id;
	// save the first mode
	mode_info = connector->modes[0];

    // print_connector(connector);

    set_connector_property(connector, "max bpc", 12);
    set_connector_property(connector, "left margin", 48);


    print_connector(connector);


	drmModeFreeConnector (connector);
	drmModeFreeResources (resources);
}

int main()
{
	device = open("/dev/dri/card0", O_RDWR|O_CLOEXEC);
	find_display_configuration();

    close(device);

    return 0;
}