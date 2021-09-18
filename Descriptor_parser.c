/*
  *	File	: Table_parser.c
  *	Author : Sendhilkumar
  *	Description : descriptor parsing and logging into file
  */

#include "dvb_parser.h"
#include "descriptor.h"                                

void ConvertBCD2String(UINT32 BCD, char *Out );

void video_stream_descriptor(Descriptor_t *p_desc)
{
	INFO_PRINT((p_desc->fp_output, "------ Video stream descriptor ------- \n"));
	
	INFO_PRINT((p_desc->fp_output, "\n\n"));
}


void audio_stream_descriptor(Descriptor_t *p_desc)
{
	INFO_PRINT((p_desc->fp_output, "------ Audio Stream descriptor ------- \n"));
	INFO_PRINT((p_desc->fp_output, "\n\n"));

}

void CA_descriptor(Descriptor_t *p_desc)
{
	UINT16	CA_system_ID, CA_PID;
	UINT8 * ptr_temp;

	ptr_temp = p_desc->pbuffer;

	INFO_PRINT((p_desc->fp_output, "------ CA descriptor ------- \n"));

	CA_system_ID = (*ptr_temp++);
	CA_system_ID <<= 8;
	CA_system_ID |= *ptr_temp++; 

	CA_PID = (*ptr_temp++) & 0x1F;
	CA_PID <<= 8;
	CA_PID |= *ptr_temp++; 

	
	INFO_PRINT((p_desc->fp_output, "CA_system_ID: %d\n", CA_system_ID));
	INFO_PRINT((p_desc->fp_output, "CA_PID: %d\n", CA_PID));

	/*
		private data parsing
	*/

	INFO_PRINT((p_desc->fp_output, "\n\n"));
	
}


void Private_data_indicator_descriptor(Descriptor_t *p_desc)
{
}

void ISO_639_language_descriptor(Descriptor_t *p_desc)
{

	UINT8	i;
	UINT8 * ptr_temp;

	ptr_temp = p_desc->pbuffer;

	INFO_PRINT((p_desc->fp_output, "------ ISO 639 language descriptor ------- \n"));

	for(i=0; i<p_desc->len;)
	{	
	
		INFO_PRINT((p_desc->fp_output, "language %d:\n", i));
		INFO_PRINT((p_desc->fp_output, "%c%c%c\n",*ptr_temp, *(ptr_temp + 1), *(ptr_temp + 2)));
		ptr_temp += 3;
		INFO_PRINT((p_desc->fp_output, "Stream type 0x%x",*ptr_temp++));

		i+= 4;
	}	
	INFO_PRINT((p_desc->fp_output, "\n\n"));
		
}


void system_clock_descriptor(Descriptor_t *p_desc)
{
}


void multiplex_buffer_utilization_descriptor(Descriptor_t *p_desc)
{
}

void copyright_descriptor(Descriptor_t *p_desc)
{
}

void maximum_bitrate_descriptor(Descriptor_t *p_desc)
{
}

void smoothing_buffer_descriptor(Descriptor_t *p_desc)
{
}

void STD_descriptor(Descriptor_t *p_desc)
{
}

void ibp_descriptor(Descriptor_t *p_desc)
{
}

void MPEG4_video_descriptor(Descriptor_t *p_desc)
{
}

void MPEG4_audio_descriptor(Descriptor_t *p_desc)
{
}

void IOD_descriptor(Descriptor_t *p_desc)
{
}

void SL_descriptor(Descriptor_t *p_desc)
{
}

void FMC_descriptor(Descriptor_t *p_desc)
{
}

void Muxcode_descriptor(Descriptor_t *p_desc)
{
}

void FmxBufferSize_descriptor(Descriptor_t *p_desc)
{
}

void MultiplexBuffer_descriptor(Descriptor_t *p_desc)
{
}

void network_name_descriptor(Descriptor_t *p_desc)
{
	UINT8	i;
	UINT8 * ptr_temp;

	ptr_temp = p_desc->pbuffer;

	INFO_PRINT((p_desc->fp_output, "------ Network name descriptor ------- \n"));

	INFO_PRINT((p_desc->fp_output, "Network name :"));
	for(i=0; i<p_desc->len; i++)
	{		
		INFO_PRINT((p_desc->fp_output, "%c",*ptr_temp++));
	}	
	INFO_PRINT((p_desc->fp_output, "\n\n"));
}

void service_list_descriptor(Descriptor_t *p_desc)
{
	UINT8	i, service_type;
	UINT16	service_id;
	UINT8 * ptr_temp;

	ptr_temp = p_desc->pbuffer;

	INFO_PRINT((p_desc->fp_output, "------ Service list descriptor ------- \n"));

	INFO_PRINT((p_desc->fp_output, "Service List \n"));
	for(i=0; i<p_desc->len;)
	{		
		service_id = *ptr_temp++;
		service_id <<=8;
		service_id |= *ptr_temp++; 
		service_type = *ptr_temp++; 
		INFO_PRINT((p_desc->fp_output, "Service id: %d\t\t",service_id));
		
		if((service_type == 1)|| (service_type == 2))
		{
			INFO_PRINT((p_desc->fp_output, "Service type: %s\n", (service_type == 1)?"Television":"Radio"));
		}
		else
		{
			INFO_PRINT((p_desc->fp_output, "Service type: %d\n", service_type ));		
		}
		
		i+=3;
	}	
	INFO_PRINT((p_desc->fp_output, "\n\n"));

}

void stuffing_descriptor(Descriptor_t *p_desc)
{
	INFO_PRINT((p_desc->fp_output, "Stuffing Descriptor \n"));
}

void satellite_delivery_system_descriptor(Descriptor_t *p_desc)
{
	UINT32	frequency, symbol_rate;
	UINT8	modulation_system,modulation_type, FEC_Inner, FEC_outer, west_east_flag, polarization, temp, roll_off;
	UINT16	orbital_position;
	UINT8 * ptr_temp;

	ptr_temp = p_desc->pbuffer;
	
	INFO_PRINT((p_desc->fp_output, "------ Satellite delivery system ------- \n"));

	frequency = *((UINT32 *)ptr_temp);
	ptr_temp += 4;
	orbital_position = *((UINT16 *)ptr_temp);
	ptr_temp += 2;
	temp = *ptr_temp++;

	west_east_flag = (temp & 0x80)>>7;
	polarization = (temp & 0x60) >> 5;
	modulation_system = (temp & 0x04) >>2;
	if(modulation_system)
	{
		roll_off = (temp & 0x18) >> 3;
	}
	else
	{
		roll_off = 0;
	}

	modulation_type = (temp & 0x03);
	
	symbol_rate = (*(UINT32 *)ptr_temp);
	FEC_Inner = symbol_rate & 0xF;
	symbol_rate >>= 4;

	p_desc->pbuffer += 4;

	INFO_PRINT((p_desc->fp_output, "Frequency %d\n", frequency));
	INFO_PRINT((p_desc->fp_output, "orbital_position %d\n", orbital_position));
	INFO_PRINT((p_desc->fp_output, "polarization %d\n", polarization));
	INFO_PRINT((p_desc->fp_output, "west_east_flag %d\n", west_east_flag));
	INFO_PRINT((p_desc->fp_output, "modulation_system %d\n", modulation_system));
	INFO_PRINT((p_desc->fp_output, "roll_off %d\n", roll_off));
	INFO_PRINT((p_desc->fp_output, "modulation_type %d\n", modulation_type));
	INFO_PRINT((p_desc->fp_output, "symbol_rate %d\n", symbol_rate));
	INFO_PRINT((p_desc->fp_output, "FEC_Inner %d\n\n", FEC_Inner));
	
}

void cable_delivery_system_descriptor(Descriptor_t *p_desc)
{
	UINT8	modulation, FEC_Inner, FEC_outer;
	UINT8	*ptr_temp, *frequency, *symbol_rate;

	ptr_temp = p_desc->pbuffer;

	INFO_PRINT((p_desc->fp_output, "------ Cable delivery system ------- \n"));
	
	frequency = ptr_temp;
	ptr_temp += 4;
	ptr_temp++;
	FEC_outer = 0x0F & *ptr_temp++;
	modulation = *ptr_temp++;
	symbol_rate = ptr_temp;

	ptr_temp +=3;
	FEC_Inner = *ptr_temp & 0xF;

	ptr_temp += 4;
	
	INFO_PRINT((p_desc->fp_output, "%-25s :%.2X%.2X%.2X%.2X\n", "Frequency", *frequency, *(frequency + 1), *(frequency + 2), *(frequency + 3)));
	INFO_PRINT((p_desc->fp_output, "%-25s :%d\n", "FEC outer", FEC_outer));
	INFO_PRINT((p_desc->fp_output, "%-25s :%d\n", "modulation", modulation));
	INFO_PRINT((p_desc->fp_output, "%-25s :%.2X%.2X%.2X\n", "Symbol Rate", *symbol_rate, *(symbol_rate + 1), *(symbol_rate + 2)));
	INFO_PRINT((p_desc->fp_output, "%-25s :%d\n\n", "FEC_Inner", FEC_Inner));
	
}

void VBI_data_descriptor (Descriptor_t *p_desc)
{
}

void VBI_teletext_descriptor(Descriptor_t *p_desc)
{
}

void bouquet_name_descriptor(Descriptor_t *p_desc)
{
	UINT8	i;
	UINT8 * ptr_temp;

	ptr_temp = p_desc->pbuffer;

	INFO_PRINT((p_desc->fp_output, "------ bouquet name descriptor ------- \n"));

	INFO_PRINT((p_desc->fp_output, "Bouquet name :"));
	for(i=0; i<p_desc->len; i++)
	{		
		INFO_PRINT((p_desc->fp_output, "%c",*ptr_temp++));
	}	
	INFO_PRINT((p_desc->fp_output, "\n\n"));

}

void service_descriptor(Descriptor_t *p_desc)
{
	UINT8	i, service_type, service_provider_name_len, service_name_len;
	UINT8 * ptr_temp;

	ptr_temp = p_desc->pbuffer;

	INFO_PRINT((p_desc->fp_output, "------ service descriptor ------- \n"));

	service_type = *ptr_temp++;
	service_provider_name_len = *ptr_temp++;

	if((service_type == 1)|| (service_type == 2))
	{
		INFO_PRINT((p_desc->fp_output, "Service type: %s\n", (service_type == 1)?"Television":"Radio"));
	}
	else
	{
		INFO_PRINT((p_desc->fp_output, "Service type: %d\n", service_type ));		
	}

	INFO_PRINT((p_desc->fp_output, "Service Provider name : "));
	
	for(i=0; i<service_provider_name_len; i++)
	{		
		INFO_PRINT((p_desc->fp_output, "%c",*ptr_temp++));
	}	

	service_name_len = *ptr_temp++;

	INFO_PRINT((p_desc->fp_output, "\nService name : "));
	
	for(i=0; i<service_name_len; i++)
	{		
		INFO_PRINT((p_desc->fp_output, "%c",*ptr_temp++));
	}	

	INFO_PRINT((p_desc->fp_output, "\n\n"));

}

void country_availability_descriptor(Descriptor_t *p_desc)
{
	UINT8	i, country_avail_flag;
	UINT8	* ptr_temp;

	ptr_temp = p_desc->pbuffer;

	INFO_PRINT((p_desc->fp_output, "------ country availability descriptor ------- \n"));

	country_avail_flag = (*ptr_temp++) >> 7;

	INFO_PRINT((p_desc->fp_output, "%-25s :%dn", "Country Avail flag\n", country_avail_flag));

	for(i=0; i<p_desc->len - 1;)
	{				
		INFO_PRINT((p_desc->fp_output, "%-25s :%c%c%c\n", "Country Code", *ptr_temp,*(ptr_temp + 1), *(ptr_temp + 2) ));

		ptr_temp += 3;
		
		i+=3;
	}	
	INFO_PRINT((p_desc->fp_output, "\n\n"));

}

void linkage_descriptor(Descriptor_t *p_desc)
{ 
	UINT8	i, linkage_type;
	UINT16	transport_stream_id, original_network_id,service_id;
	UINT8 * ptr_temp;

	ptr_temp = p_desc->pbuffer;

	INFO_PRINT((p_desc->fp_output, "------ linkage descriptor ------- \n"));

	transport_stream_id = *ptr_temp++;
	transport_stream_id <<= 8;
	transport_stream_id |= *ptr_temp++;
	

	original_network_id = *ptr_temp++;
	original_network_id <<= 8;
	original_network_id |= *ptr_temp++;

	service_id = *ptr_temp++;
	service_id <<= 8;
	service_id |= *ptr_temp++;

	linkage_type = *ptr_temp++;

	INFO_PRINT((p_desc->fp_output, "%-25s :%d\n", "transport_stream_id", transport_stream_id));		
	INFO_PRINT((p_desc->fp_output, "%-25s :%d\n", "orig_network_id", original_network_id));		
	INFO_PRINT((p_desc->fp_output, "%-25s :%d\n", "service_id", service_id));		
	INFO_PRINT((p_desc->fp_output, "%-25s :%d\n", "linkage_type", linkage_type));		
	
	INFO_PRINT((p_desc->fp_output, "\n\n"));

}

void NVOD_reference_descriptor(Descriptor_t *p_desc)
{
}

void time_shifted_service_descriptor(Descriptor_t *p_desc)
{
}

void short_event_descriptor(Descriptor_t *p_desc)
{
	UINT8	event_name_length, text_length, i;
	UINT8 * ptr_temp;

	ptr_temp = p_desc->pbuffer;

	INFO_PRINT((p_desc->fp_output, "------ short_event_descriptor ------- \n"));

	INFO_PRINT((p_desc->fp_output, "%-25s :%c%c%c\n", "language_code", *ptr_temp, *(ptr_temp + 1), *(ptr_temp + 2)));

	ptr_temp += 3;

	event_name_length = *ptr_temp++;

	INFO_PRINT((p_desc->fp_output, "Event Name: "));

	for(i=0;i<event_name_length; i++)
	{		
		INFO_PRINT((p_desc->fp_output, "%c",*ptr_temp++));
	}

	text_length = *ptr_temp++;
	INFO_PRINT((p_desc->fp_output, "\n"));

	INFO_PRINT((p_desc->fp_output, "Event Description: "));

	for(i=0;i<event_name_length; i++)
	{		
		INFO_PRINT((p_desc->fp_output, "%c",*ptr_temp++));
	}

	INFO_PRINT((p_desc->fp_output, "\n\n"));
	
}

void extended_event_descriptor(Descriptor_t *p_desc)
{
	UINT8	desc_num, last_desc_num, i, j,len_of_items, item_desc_len, item_len, text_len;
	UINT8 * ptr_temp;

	ptr_temp = p_desc->pbuffer;

	INFO_PRINT((p_desc->fp_output, "------ extended_event_descriptor ------- \n"));

	desc_num	=	*ptr_temp >> 4;
	last_desc_num = *ptr_temp++ & 0x0F;

	INFO_PRINT((p_desc->fp_output, "%-25s :%c%c%c\n", "language_code", *ptr_temp, *(ptr_temp + 1), *(ptr_temp + 2)));

	ptr_temp += 3;

	len_of_items = *ptr_temp++;

	for(i=0;i<len_of_items; i++)
	{		
		item_desc_len = *ptr_temp++;

		INFO_PRINT((p_desc->fp_output, "Item description: \n"));
		
		for(j=0; j<item_desc_len; j++)
		{
			INFO_PRINT((p_desc->fp_output, "%c",*ptr_temp++));
		}
		
		INFO_PRINT((p_desc->fp_output, "\n\n"));

		item_len = *ptr_temp++;

		INFO_PRINT((p_desc->fp_output, "Item : \n"));
		
		for(j=0; j<item_len; j++)
		{
			INFO_PRINT((p_desc->fp_output, "%c",*ptr_temp++));
		}
		
		INFO_PRINT((p_desc->fp_output, "\n\n"));
	}

	text_len = *ptr_temp++;
	
	INFO_PRINT((p_desc->fp_output, "Text: \t"));

	for(i=0;i<text_len; i++)
	{		
		INFO_PRINT((p_desc->fp_output, "%c",*ptr_temp++));
	}

	INFO_PRINT((p_desc->fp_output, "\n\n"));

}

void time_shifted_event_descriptor(Descriptor_t *p_desc)
{
}

void component_descriptor(Descriptor_t *p_desc)
{
}

void mosaic_descriptor(Descriptor_t *p_desc)
{
}

void stream_identifier_descriptor(Descriptor_t *p_desc)
{
}

void CA_identifier_descriptor(Descriptor_t *p_desc)
{
	UINT8	i, N;
	UINT16  CA_system_id;
	UINT8 * ptr_temp;

	ptr_temp = p_desc->pbuffer;

	INFO_PRINT((p_desc->fp_output, "------ CA identifier descriptor ------- \n"));

	N = p_desc->len / 2;

	for(i=0; i<N;)
	{		
		CA_system_id = ptr_temp;
		INFO_PRINT((p_desc->fp_output, "%-25s :%d\n", "CA_system_id", CA_system_id));
		ptr_temp += 2;
		i+=2;
	}	
	INFO_PRINT((p_desc->fp_output, "\n\n"));

}

void content_descriptor(Descriptor_t *p_desc)
{
	UINT8	i, N;
	UINT8  	nibble_level1, nibble_level2, user_byte;
	UINT8 	* ptr_temp;

	ptr_temp = p_desc->pbuffer;

	INFO_PRINT((p_desc->fp_output, "------ Content descriptor ------- \n"));

	for(i=0; i<p_desc->len;)
	{		
		nibble_level1 = (*ptr_temp) >> 4;
		nibble_level2 = (*ptr_temp) & 0x0F;
		ptr_temp++;
		user_byte = *ptr_temp;		
		ptr_temp++;
		
		INFO_PRINT((p_desc->fp_output, "%-25s :%d\n", "nibble_level1", nibble_level1));
		INFO_PRINT((p_desc->fp_output, "%-25s :%d\n", "nibble_level2", nibble_level2));
		INFO_PRINT((p_desc->fp_output, "%-25s :%d\n", "user_byte", user_byte));

		i+=2;
	}	
	INFO_PRINT((p_desc->fp_output, "\n\n"));


}

void parental_rating_descriptor(Descriptor_t *p_desc)
{

	UINT8	i;
	UINT8	* ptr_temp;

	ptr_temp = p_desc->pbuffer;

	INFO_PRINT((p_desc->fp_output, "------ Parental rating descriptor ------- \n"));

	for(i=0; i<p_desc->len;)
	{				
		INFO_PRINT((p_desc->fp_output, "%-25s :%c%c%c\n", "Country Code", *ptr_temp,*(ptr_temp + 1), *(ptr_temp + 2) ));

		ptr_temp += 3;
		
		INFO_PRINT((p_desc->fp_output, "%-25s :%d\n", "rating", *ptr_temp));
		i+=4;
	}	
	INFO_PRINT((p_desc->fp_output, "\n\n"));

}

void teletext_descriptor(Descriptor_t *p_desc)
{
}

void telephone_descriptor(Descriptor_t *p_desc)
{
}

void local_time_offset_descriptor(Descriptor_t *p_desc)
{
	
	UINT8	i, country_region_id, local_offset_polarity;
	UINT8	* ptr_temp;
	UINT16	local_time_offset, MJD, next_time_offset;

	ptr_temp = p_desc->pbuffer;

	INFO_PRINT((p_desc->fp_output, "------ Local time offset descriptor ------- \n"));

	for(i=0; i<p_desc->len;)
	{				
		INFO_PRINT((p_desc->fp_output, "%-25s :%c%c%c\n", "Country Code", *ptr_temp,*(ptr_temp + 1), *(ptr_temp + 2) ));

		ptr_temp += 3;

		country_region_id = (*ptr_temp ) >> 2;
		local_offset_polarity = (*ptr_temp++) & 0x01;
		
		INFO_PRINT((p_desc->fp_output, "%-25s :0x%X\n", "country_region_id", country_region_id));
		INFO_PRINT((p_desc->fp_output, "%-25s :%d\n", "local_offset_polarity", local_offset_polarity ));
		INFO_PRINT((p_desc->fp_output, "%-25s :%.2X%.2X\n", "Time Offset", *ptr_temp, *(ptr_temp + 1))); 
		
		ptr_temp += 2;

		MJD = *ptr_temp++;
		MJD <<= 8;
		MJD |= *ptr_temp++;

		INFO_PRINT((p_desc->fp_output, "%-25s :%d\n", "Time Of Change: MJD", MJD));	
		INFO_PRINT((p_desc->fp_output, "%-25s :%.2X%.2X%.2X\n", "Time Of Change: UTC", *ptr_temp, *(ptr_temp + 1), *(ptr_temp + 2))); 

		ptr_temp += 3;		
		
		INFO_PRINT((p_desc->fp_output, "%-25s :%.2X%.2X\n", "Next Time Offset", *ptr_temp, *(ptr_temp + 1))); 

		ptr_temp += 2;		
				
		i+=13;
	}	



}


void subtitling_descriptor(Descriptor_t *p_desc)
{
	UINT8	i, subtitling_type,	*ptr_temp;
	UINT16	composition_page_id,ancillary_page_id;
	UINT32	ISO_639_language_code;

	INFO_PRINT((p_desc->fp_output, "------ subtitling_descriptor ------- \n"));
	DBG_PRINT(("Subtitle start" ));

	ptr_temp = p_desc->pbuffer;

	for(i=0; i<p_desc->len;)
	{		
		ISO_639_language_code = *ptr_temp++;
		ISO_639_language_code <<= 8;
		ISO_639_language_code |= *ptr_temp++;
		ISO_639_language_code <<= 8;		
		ISO_639_language_code |= *ptr_temp++;

		subtitling_type = *ptr_temp++;
		
		composition_page_id = *ptr_temp++;
		composition_page_id <<=8;
		composition_page_id |= *ptr_temp++;

		ancillary_page_id = *ptr_temp++;
		ancillary_page_id  <<=8;
		ancillary_page_id |= *ptr_temp++;

		i+= 8;
			
		INFO_PRINT((p_desc->fp_output, "%-25s :0x%X\n", "ISO639_lang_code",ISO_639_language_code));		
		INFO_PRINT((p_desc->fp_output, "%-25s :%d\n", "subtitling_type",subtitling_type));		
		INFO_PRINT((p_desc->fp_output, "%-25s :%d\n", "comp_pg_id",composition_page_id));		
		INFO_PRINT((p_desc->fp_output, "%-25s :%d\n\n", "anci_pg_id",ancillary_page_id));		
		
	}	
	
	DBG_PRINT(("Subtitle end" ));
	INFO_PRINT((p_desc->fp_output, "\n\n"));

}

void terrestrial_delivery_system_descriptor(Descriptor_t *p_desc)
{
}

void multilingual_network_name_descriptor(Descriptor_t *p_desc)
{

	UINT8	i, j, service_provider_name_len, service_name_len;
	UINT8 * ptr_temp;

	ptr_temp = p_desc->pbuffer;

	INFO_PRINT((p_desc->fp_output, "------ multilingual service name descriptor ------- \n"));

	for(i=0; i<p_desc->len;)
	{
		INFO_PRINT((p_desc->fp_output, "%-25s :%c%c%c\n", "ISO Lang Code", *ptr_temp,*(ptr_temp + 1), *(ptr_temp + 2) ));

		ptr_temp += 3;

		service_provider_name_len = *ptr_temp++;

		INFO_PRINT((p_desc->fp_output, "Service Provider name : "));

		for(j=0; j<service_provider_name_len; j++)
		{		
			INFO_PRINT((p_desc->fp_output, "%c",*ptr_temp++));
		}	

		service_name_len = *ptr_temp++;

		INFO_PRINT((p_desc->fp_output, "\nService name : "));

		for(j=0; j<service_name_len; j++)
		{		
			INFO_PRINT((p_desc->fp_output, "%c",*ptr_temp++));
		}		

		i += service_provider_name_len + service_name_len + 5;

		INFO_PRINT((p_desc->fp_output, "\n"));
		
	}

	INFO_PRINT((p_desc->fp_output, "\n\n"));

}

void multilingual_bouquet_name_descriptor(Descriptor_t *p_desc)
{
}

void multilingual_service_name_descriptor(Descriptor_t *p_desc)
{

	UINT8	i, j, service_provider_name_len, service_name_len;
	UINT8 * ptr_temp;

	ptr_temp = p_desc->pbuffer;

	INFO_PRINT((p_desc->fp_output, "------ multilingual service name descriptor ------- \n"));

	for(i=0; i<p_desc->len;)
	{
		INFO_PRINT((p_desc->fp_output, "%-25s :%c%c%c\n", "ISO Lang Code", *ptr_temp,*(ptr_temp + 1), *(ptr_temp + 2) ));

		ptr_temp += 3;

		service_provider_name_len = *ptr_temp++;

		INFO_PRINT((p_desc->fp_output, "Service Provider name : "));

		for(j=0; j<service_provider_name_len; j++)
		{		
			INFO_PRINT((p_desc->fp_output, "%c",*ptr_temp++));
		}	

		service_name_len = *ptr_temp++;

		INFO_PRINT((p_desc->fp_output, "\nService name : "));

		for(j=0; j<service_name_len; j++)
		{		
			INFO_PRINT((p_desc->fp_output, "%c",*ptr_temp++));
		}		

		i += service_provider_name_len + service_name_len + 5;

		INFO_PRINT((p_desc->fp_output, "\n"));
		
		
	}


	INFO_PRINT((p_desc->fp_output, "\n\n"));

}

void multilingual_component_descriptor(Descriptor_t *p_desc)
{
}

void private_data_specifier_descriptor(Descriptor_t *p_desc)
{
}

void service_move_descriptor(Descriptor_t *p_desc)
{
}

void short_smoothing_buffer_descriptor(Descriptor_t *p_desc)
{
}

void frequency_list_descriptor(Descriptor_t *p_desc)
{
	UINT8	i, N, coding_type, *center_frequency;
	UINT8 * ptr_temp;

	ptr_temp = p_desc->pbuffer;

	INFO_PRINT((p_desc->fp_output, "------ Frequency list descriptor ------- \n"));

	coding_type = *ptr_temp++;

	coding_type = coding_type & 0x03;
	INFO_PRINT((p_desc->fp_output, "Coding Type: %d", coding_type ));		

	N = p_desc->len / 4;

	for(i=0; i<N;)
	{		
		center_frequency = ptr_temp;
		INFO_PRINT((p_desc->fp_output, "%-25s :%.2X%.2X%.2X%.2X\n", "Frequency", *center_frequency, *(center_frequency + 1), *(center_frequency + 2), *(center_frequency + 3)));
		ptr_temp += 4;
		i+=4;
	}	
	INFO_PRINT((p_desc->fp_output, "\n\n"));

}

void partial_transport_stream_descriptor(Descriptor_t *p_desc)
{
}

void data_broadcast_descriptor(Descriptor_t *p_desc)
{
}

void scrambling_descriptor(Descriptor_t *p_desc)
{
}

void data_broadcast_id_descriptor(Descriptor_t *p_desc)
{
}

void transport_stream_descriptor(Descriptor_t *p_desc)
{
}

void DSNG_descriptor(Descriptor_t *p_desc)
{
}

void PDC_descriptor(Descriptor_t *p_desc)
{
}

void AC3_descriptor(Descriptor_t *p_desc)
{
}

void ancillary_data_descriptor(Descriptor_t *p_desc)
{
}

void cell_list_descriptor(Descriptor_t *p_desc)
{
}

void cell_frequency_link_descriptor(Descriptor_t *p_desc)
{
}

void announcement_support_descriptor(Descriptor_t *p_desc)
{
}

void application_signalling_descriptor(Descriptor_t *p_desc)
{
}

void adaptation_field_data_descriptor(Descriptor_t *p_desc)
{
}

void service_identifier_descriptor(Descriptor_t *p_desc)
{
}

void default_authority_descriptor(Descriptor_t *p_desc)
{
}

void related_content_descriptor(Descriptor_t *p_desc)
{
}

void TVA_id_descriptor(Descriptor_t *p_desc)
{
}

void content_identifier_descriptor(Descriptor_t *p_desc)
{
}

void time_slice_fec_identifier_descriptor(Descriptor_t *p_desc)
{
}

void ECM_repetition_rate_descriptor(Descriptor_t *p_desc)
{
}

void S2_satellite_delivery_system_descriptor(Descriptor_t *p_desc)
{
}

void enhanced_AC3_descriptor(Descriptor_t *p_desc)
{
}

void DTS_descriptor(Descriptor_t *p_desc)
{
}

void AAC_descriptor(Descriptor_t *p_desc)
{
}

void extension_descriptor(Descriptor_t *p_desc)
{
}

/* these are reserved descriptor -NDS Specific - need to check  */
void logical_channel_descriptor(Descriptor_t *p_desc)
{
	UINT8	i;
	UINT16	service_id, logical_service_id;
	UINT8 * ptr_temp;

	ptr_temp = p_desc->pbuffer;

	INFO_PRINT((p_desc->fp_output, "------ Logical Channel descriptor ------- \n"));

	INFO_PRINT((p_desc->fp_output, "Logical Channel List \n"));
	for(i=0; i<p_desc->len;)
	{		
		service_id = *(UINT16 *)ptr_temp;
		ptr_temp += 2;
		logical_service_id = *(UINT16 *)ptr_temp;
		ptr_temp += 2;		
		INFO_PRINT((p_desc->fp_output, "Service id: %d\t\t",service_id));		
		INFO_PRINT((p_desc->fp_output, "Logical channel id: %d\n",logical_service_id));
		i+=4;
		
	}	
	INFO_PRINT((p_desc->fp_output, "\n\n"));	
}


void Service_genre_descriptor(Descriptor_t *p_desc)
{

	UINT16	service_genre;
	UINT8 * ptr_temp;

	ptr_temp = p_desc->pbuffer;

	INFO_PRINT((p_desc->fp_output, "------ Service Genre descriptor ------- \n"));

	service_genre = *(UINT16 *)ptr_temp;	

	INFO_PRINT((p_desc->fp_output, "Service Genre: %d\n\n", service_genre));
	
}

