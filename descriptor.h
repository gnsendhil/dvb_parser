/*
  *	File	: Descriptor.h
  *	Author : Sendhilkumar
  *	Description : Contains structures for section descriptors and prototypes .
  */

#include<stdio.h>

typedef struct descriptor
{
	UINT8 tag;
	UINT8 len;
	UINT8 *pbuffer;
	FILE *fp_output;
}Descriptor_t;

/*Parser function pointers*/
typedef void (*fp_DescParser)(Descriptor_t *);

void CA_descriptor(Descriptor_t *p_desc);
void ISO_639_language_descriptor(Descriptor_t *p_desc);
void network_name_descriptor(Descriptor_t *p_desc);
void service_list_descriptor(Descriptor_t *p_desc);   
void stuffing_descriptor(Descriptor_t *p_desc);
void satellite_delivery_system_descriptor(Descriptor_t *p_desc);
void cable_delivery_system_descriptor(Descriptor_t *p_desc);
void linkage_descriptor(Descriptor_t *p_desc);
void service_descriptor(Descriptor_t *p_desc);
void short_event_descriptor(Descriptor_t *p_desc);
void subtitling_descriptor(Descriptor_t *p_desc);
void extended_event_descriptor(Descriptor_t *p_desc);
void bouquet_name_descriptor(Descriptor_t *p_desc);
void CA_identifier_descriptor(Descriptor_t *p_desc);
void content_descriptor(Descriptor_t *p_desc);
void parental_rating_descriptor(Descriptor_t *p_desc);
void local_time_offset_descriptor(Descriptor_t *p_desc);
void frequency_list_descriptor(Descriptor_t *p_desc);
void multilingual_service_name_descriptor(Descriptor_t *p_desc);

/* NDS Specific Descriptor */
void logical_channel_descriptor(Descriptor_t *p_desc);
void Service_genre_descriptor(Descriptor_t *p_desc);

