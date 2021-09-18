/*
  *	File	: Table_parser.c
  *	Author : Sendhilkumar
  *	Description : Section Parsing, triggers descriptor parsing and logs into file
  */

#include "dvb_parser.h"
#include "descriptor.h"                                

#define	TDT_TABLE_ID 0x71
#define	TOT_TABLE_ID 0x73

static fp_DescParser fp_DescParserArr[0xFF];
Table_ver_status	table_ver_sec_parse_status[0xFF];
Table_ver_status	pmt_ver_sec_parse_status[0xFFFF];



void Register_Descriptors(void)
{
	UINT16	i;
	fp_DescParserArr[0x09] = CA_descriptor;
	fp_DescParserArr[0x0A] = ISO_639_language_descriptor;
	fp_DescParserArr[0x40] = network_name_descriptor;
	fp_DescParserArr[0x41] = service_list_descriptor;
	fp_DescParserArr[0x42] = stuffing_descriptor;
	fp_DescParserArr[0x43] = satellite_delivery_system_descriptor;
	fp_DescParserArr[0x44] = cable_delivery_system_descriptor;
	fp_DescParserArr[0x47] = bouquet_name_descriptor;
	fp_DescParserArr[0x48] = service_descriptor;
	fp_DescParserArr[0x4A] = linkage_descriptor;
	fp_DescParserArr[0x4D] = short_event_descriptor;	
	fp_DescParserArr[0x4E] = extended_event_descriptor;	
	fp_DescParserArr[0x53] = CA_identifier_descriptor;
	fp_DescParserArr[0x54] = content_descriptor;	
	fp_DescParserArr[0x55] = parental_rating_descriptor;
	fp_DescParserArr[0x58] = local_time_offset_descriptor;
	fp_DescParserArr[0x59] = subtitling_descriptor;
	fp_DescParserArr[0x5D] = multilingual_service_name_descriptor;
	fp_DescParserArr[0x62] = frequency_list_descriptor;
//	fp_DescParserArr[0xE2] = logical_channel_descriptor; /*Reserved tag, NDS specific */
	fp_DescParserArr[0xCC] = Service_genre_descriptor;

	for(i=0; i<0xFF; i++)
	{
		table_ver_sec_parse_status[i].ver_processed = 0xFF;
		table_ver_sec_parse_status[i].sec_num_to_parse = 0x0;
	}

	for(i=0; i<0xFFFF; i++)
	{
		/* based on program number */
		pmt_ver_sec_parse_status[i].ver_processed = 0xFF;
		pmt_ver_sec_parse_status[i].sec_num_to_parse = 0x0;
	}
	
	
}
void PAT_Parser(Sec_Filter *sec_filter)
{
	UINT8 tableid,sec_num, last_sec_num, cur_next_ind, i, num_of_prog, version_num;
	UINT32 sec_length, transport_id,prog_num, prog_pid; 
	UINT8 * pbuffer = (unsigned char *)sec_filter->pBuffer;

	ENTRY_PRINT(("ENTRY func %s Line %d \n", __func__, __LINE__));

#ifdef DEBUG_CODE
	for(i=0; i<sec_filter->FilledSize; i++)
	{
		DBG_PRINT(("%02X ", *pbuffer++));
	}
	getch();
	
#endif /* BOGUS */
	

	pbuffer = sec_filter->pBuffer;

	tableid = *pbuffer++;
	sec_length = (*pbuffer++)& 0x0F;
	sec_length <<=8;
	sec_length |= *pbuffer++;
	transport_id = *pbuffer++;
	transport_id <<=8;
	transport_id |= *pbuffer++;
	version_num = *pbuffer & 0x3E;
	version_num >>=1;
	cur_next_ind = *pbuffer++ & 0x01;
	sec_num = *pbuffer++;
	last_sec_num = *pbuffer++;

	num_of_prog = (sec_length - 9 )/PMT_PAYLOAD_SIZE;

	if(table_ver_sec_parse_status[tableid].ver_processed != version_num )
	{
		/* check the section to parse*/
		if(table_ver_sec_parse_status[tableid].sec_num_to_parse == sec_num )	
		{
	
			INFO_PRINT((sec_filter->fp_output_file, "============= PAT ==============\n"));
			INFO_PRINT((sec_filter->fp_output_file, "%-25s :0x%X\n", "tableid", tableid));
			INFO_PRINT((sec_filter->fp_output_file, "%-25s :%d\n", "transport_id", transport_id));
			INFO_PRINT((sec_filter->fp_output_file, "%-25s :%d\n", "sec_length", sec_length));
			INFO_PRINT((sec_filter->fp_output_file, "%-25s :%d\n", "version_num", version_num));	
			INFO_PRINT((sec_filter->fp_output_file, "%-25s :%d\n", "cur_next_ind", cur_next_ind));	
			INFO_PRINT((sec_filter->fp_output_file, "%-25s :%d\n", "sec_num", sec_num));
			INFO_PRINT((sec_filter->fp_output_file, "%-25s :%d\n", "last_sec_num", last_sec_num));
		
			for(i=0; i<num_of_prog; i++)
			{
				prog_num = *pbuffer++;
				prog_num <<=8;
				prog_num |= *pbuffer++;
				prog_pid = (*pbuffer++) & 0x1F;
				prog_pid <<=8;		
				prog_pid |= *pbuffer++;

				INFO_PRINT((sec_filter->fp_output_file, "%-25s :%d\n", "Program Num", prog_num));
				INFO_PRINT((sec_filter->fp_output_file, "%-25s :%d\n\n", "Program PID", prog_pid));
				Schedule_PMT(prog_pid);
				
			}

			/* All secions of the table is processed, so make this table as processed */
			if(table_ver_sec_parse_status[tableid].sec_num_to_parse == last_sec_num)
			{
				table_ver_sec_parse_status[tableid].ver_processed = version_num;
				table_ver_sec_parse_status[tableid].sec_num_to_parse = 0;				
			}
			else
			{
				table_ver_sec_parse_status[tableid].sec_num_to_parse++;
			}
						
		}
		
	}
	else
	{		
		DBG_PRINT(("Version of PAT table id %d is same\n", tableid));
	}
	DBG_PRINT(("CRC %X %X %X %X\n", *pbuffer++, *pbuffer++, *pbuffer++, *pbuffer++));
	
	EXIT_PRINT(("EXIT func %s Line %d \n", __func__, __LINE__));
	
}

void PMT_Parser(Sec_Filter *sec_filter)
{
	UINT8 tableid,sec_num, last_sec_num, cur_next_ind, num_of_streams, version_num, stream_type;
	UINT16 PCR_PID,prog_info_length, Elementary_PID, Es_Info_Length, prog_num;
	UINT32 sec_length, prog_pid, i, j; 	
	Descriptor_t	desc;
	
	UINT8 * pbuffer = (unsigned char *)sec_filter->pBuffer;

	ENTRY_PRINT(("ENTRY func %s Line %d \n", __func__, __LINE__));

#ifdef DEBUG_CODE
	for(i=0; i<sec_filter->FilledSize; i++)
	{
		DBG_PRINT(("%02X ", *pbuffer++));
	}
	getch();
		
#endif /* DEBUG_CODE */
		
	tableid = *pbuffer++;
	sec_length = (*pbuffer++)& 0x0F;
	sec_length <<=8;
	sec_length |= *pbuffer++;
	prog_num = *pbuffer++;
	prog_num <<=8;
	prog_num |= *pbuffer++;
	version_num = *pbuffer & 0x3E;
	version_num >>=1;
	cur_next_ind = *pbuffer++ & 0x01;
	sec_num = *pbuffer++;
	last_sec_num = *pbuffer++;

	PCR_PID = *pbuffer++;		
	PCR_PID <<=8;
	PCR_PID |= *pbuffer++;
	PCR_PID &= 0x1FFF;

	prog_info_length = *pbuffer++;		
	prog_info_length <<=8;
	prog_info_length |= *pbuffer++;
	prog_info_length &= 0xFFF;

	if(pmt_ver_sec_parse_status[prog_num].ver_processed != version_num )
	{
		/* check the section to parse*/
		if(pmt_ver_sec_parse_status[prog_num].sec_num_to_parse == sec_num )	
		{
	
			INFO_PRINT((sec_filter->fp_output_file, "************* PMT for the program %d *************\n", prog_num));
			INFO_PRINT((sec_filter->fp_output_file, "%-25s :0x%X\n", "tableid", tableid));
			INFO_PRINT((sec_filter->fp_output_file, "%-25s :%d\n", "prog num", prog_num));
			INFO_PRINT((sec_filter->fp_output_file, "%-25s :%d\n", "sec_length", sec_length));
			INFO_PRINT((sec_filter->fp_output_file, "%-25s :%d\n", "version_num", version_num)); 
			INFO_PRINT((sec_filter->fp_output_file, "%-25s :%d\n", "cur_next_ind", cur_next_ind));	
			INFO_PRINT((sec_filter->fp_output_file, "%-25s :%d\n", "sec_num", sec_num));
			INFO_PRINT((sec_filter->fp_output_file, "%-25s :%d\n", "last_sec_num", last_sec_num));
			INFO_PRINT((sec_filter->fp_output_file, "%-25s :%d\n", "PCR_PID", PCR_PID)); 		

			INFO_PRINT((sec_filter->fp_output_file,"\n============= PROGRAM DESCRIPTORS =============\n\n"));

			for(i=0; i<prog_info_length;)	
			{
				desc.tag = *pbuffer++;
				desc.len = *pbuffer++;	
				desc.fp_output = sec_filter->fp_output_file;
					
				desc.pbuffer = (UINT8 *)malloc(desc.len);
				memcpy(desc.pbuffer, pbuffer, desc.len);
				
				if(fp_DescParserArr[desc.tag] != NULL)
				{
					fp_DescParserArr[desc.tag](&desc);
				}

				DBG_PRINT(("Line:%d tag %X Len %d\n", __LINE__, desc.tag, desc.len));
				
				INFO_PRINT((sec_filter->fp_output_file, "%-25s :%d\n", "Program Descriptor in PMT", desc.tag));

				free(desc.pbuffer);
				i+= desc.len + 2;
				pbuffer += desc.len;
			}

			INFO_PRINT((sec_filter->fp_output_file, "============= END OF PROGRAM DESCRIPTORS ===============\n\n"));


			INFO_PRINT((sec_filter->fp_output_file, "%-25s :%d\n\n", "prog_info_length", prog_info_length));

			for(i=0; i<(sec_length - prog_info_length - 13);)
			{
				stream_type = *pbuffer++;
				Elementary_PID = *pbuffer++;
				Elementary_PID <<=8;
				Elementary_PID |= *pbuffer++;
				Elementary_PID &= 0x1FFF;

				Es_Info_Length = *pbuffer++;
				Es_Info_Length <<=8;
				Es_Info_Length |= *pbuffer++;
				Es_Info_Length &= 0xFFF;
				
				INFO_PRINT((sec_filter->fp_output_file, "%-25s :%d\n", "Stream Type", stream_type));
				INFO_PRINT((sec_filter->fp_output_file, "%-25s :%d\n", "Elementary PID", Elementary_PID));
				INFO_PRINT((sec_filter->fp_output_file, "%-25s :%d\n\n", "Es_Info_Length", Es_Info_Length));

	/*			ES Info Parsing */
				for(j=0; j<Es_Info_Length;)
				{
				
					DBG_PRINT(("Line:%d J:%d Es:%d\n", __LINE__, j,Es_Info_Length));
					desc.tag = *pbuffer++;
					desc.len = *pbuffer++;	
					desc.fp_output = sec_filter->fp_output_file;
						
					desc.pbuffer = (UINT8 *)malloc(desc.len);
					if(desc.pbuffer == NULL)
					{					
						DBG_PRINT(("Line:%d desc.pbuffer %X\n", __LINE__, desc.pbuffer));
						getch();
					}
					DBG_PRINT(("Line:%d desc.pbuffer %X\n", __LINE__, desc.pbuffer));

					memcpy(desc.pbuffer, pbuffer, desc.len);
					
					if(fp_DescParserArr[desc.tag] != NULL)
					{
						fp_DescParserArr[desc.tag](&desc);
					}
					DBG_PRINT(("Es Len %d\n", Es_Info_Length));
					DBG_PRINT(("tag %X Len %d\n", desc.tag, desc.len));

					j+= desc.len + 2;
					pbuffer += desc.len;
					
					DBG_PRINT(("Line:%d J:%d Es:%d\n", __LINE__, j,Es_Info_Length));
					DBG_PRINT(("Line:%d desc.pbuffer %X\n", __LINE__, desc.pbuffer));

					free(desc.pbuffer);
					
					DBG_PRINT(("Line:%d J:%d Es:%d\n", __LINE__, j,Es_Info_Length));
				}
	
				i += Es_Info_Length + 5;
				
				DBG_PRINT(("Line:%d i:%d Es:%d\n", __LINE__, i,Es_Info_Length));
			}

			DBG_PRINT(("Line:%d \n", __LINE__));
			
			/* All secions of the table is processed, so make this table as processed */
			if(pmt_ver_sec_parse_status[prog_num].sec_num_to_parse == last_sec_num)
			{
				pmt_ver_sec_parse_status[prog_num].ver_processed = version_num;
				pmt_ver_sec_parse_status[prog_num].sec_num_to_parse = 0;				
			}
			else
			{
				pmt_ver_sec_parse_status[prog_num].sec_num_to_parse++;
			}
			
		}
			

	}
	else
	{
		DBG_PRINT(("Version of PMT table of program %d is same\n", prog_num));
	}
		
	EXIT_PRINT(("EXIT func %s Line %d \n", __func__, __LINE__));
}

void NIT_Parser(Sec_Filter * sec_filter)
{
	UINT8 tableid,sec_num, last_sec_num, cur_next_ind, version_num;
	UINT16 network_desc_length, transport_stream_loop_len, 
		transport_stream_id, original_network_id,transport_desc_length;
	UINT32 sec_length,network_id,i, j; 
	Descriptor_t	desc;
		
	UINT8 * pbuffer = (unsigned char *)sec_filter->pBuffer;

	ENTRY_PRINT(("ENTRY func %s Line %d \n", __func__, __LINE__));

#ifdef DEBUG_CODE
	temp = (unsigned char *)sec_filter->pBuffer;

	for(i=0; i<sec_filter->FilledSize; i++)
	{
		DBG_PRINT(("%02X ", *temp++));
	}
	getch();
		
#endif /* DEBUG_CODE */
		
	tableid = *pbuffer++;
	sec_length = (*pbuffer++)& 0x0F;
	sec_length <<=8;
	sec_length |= *pbuffer++;
	network_id = *pbuffer++;
	network_id <<=8;
	network_id |= *pbuffer++;
	version_num = *pbuffer & 0x3E;
	version_num >>=1;
	cur_next_ind = *pbuffer++ & 0x01;
	sec_num = *pbuffer++;
	last_sec_num = *pbuffer++;


	network_desc_length = *pbuffer++;		
	network_desc_length <<=8;
	network_desc_length |= *pbuffer++;
	network_desc_length &= 0xFFF;

	DBG_PRINT(("NIT %d\n", __LINE__ ));

	if(table_ver_sec_parse_status[tableid].ver_processed != version_num )
	{
		/* check the section to parse*/
		if(table_ver_sec_parse_status[tableid].sec_num_to_parse == sec_num )	
		{
			INFO_PRINT((sec_filter->fp_output_file, "************* %s NIT *************\n",(tableid==0x40)?"ACTUAL":"OTHER" ));
			INFO_PRINT((sec_filter->fp_output_file, "%-25s :0x%X\n", "tableid", tableid));
			INFO_PRINT((sec_filter->fp_output_file, "%-25s :%d\n", "network_id", network_id));
			INFO_PRINT((sec_filter->fp_output_file, "%-25s :%d\n", "sec_length", sec_length));
			INFO_PRINT((sec_filter->fp_output_file, "%-25s :%d\n", "version_num", version_num)); 
			INFO_PRINT((sec_filter->fp_output_file, "%-25s :%d\n", "cur_next_ind", cur_next_ind));	
			INFO_PRINT((sec_filter->fp_output_file, "%-25s :%d\n", "sec_num", sec_num));
			INFO_PRINT((sec_filter->fp_output_file, "%-25s :%d\n", "last_sec_num", last_sec_num));
			INFO_PRINT((sec_filter->fp_output_file, "%-25s :%d\n", "network_desc_length", network_desc_length));

			INFO_PRINT((sec_filter->fp_output_file, "============= NETWORK DESCRIPTORS =============\n\n"));


		/*		Network Descriptor processing */
			for(i=0; i<network_desc_length;)
			{
				desc.tag = *pbuffer++;
				desc.len = *pbuffer++;	
				desc.fp_output = sec_filter->fp_output_file;
				
				desc.pbuffer = (UINT8 *)malloc(desc.len);
				memcpy(desc.pbuffer, pbuffer, desc.len);
				
				if(fp_DescParserArr[desc.tag] != NULL)
				{
					fp_DescParserArr[desc.tag](&desc);
				}

				DBG_PRINT(("tag %X Len %d\n", desc.tag, desc.len));
				free(desc.pbuffer);
				i+= desc.len + 2;
				pbuffer += desc.len;
			}

			INFO_PRINT((sec_filter->fp_output_file,"============= END OF NETWORK DESCRIPTORS ===============\n\n"));
			
			transport_stream_loop_len = *pbuffer++ & 0x0F;
			transport_stream_loop_len <<= 8;
			transport_stream_loop_len |= *pbuffer++; 

			INFO_PRINT((sec_filter->fp_output_file, "%-25s :%d\n", "tp_stream_loop_len", transport_stream_loop_len));

			for(i=0; i<transport_stream_loop_len;)
			{
				transport_stream_id = *pbuffer++;
				transport_stream_id <<=8;
				transport_stream_id |= *pbuffer++; 
				
				original_network_id = *pbuffer++;
				original_network_id <<=8;
				original_network_id |= *pbuffer++; 

				transport_desc_length = *pbuffer++ & 0x0F;
				transport_desc_length <<= 8;
				transport_desc_length |= *pbuffer++; 
				
				INFO_PRINT((sec_filter->fp_output_file, "%-25s :%d\n", "transport_stream_id", transport_stream_id));
				INFO_PRINT((sec_filter->fp_output_file, "%-25s :%d\n", "orig_network_id", original_network_id));
				INFO_PRINT((sec_filter->fp_output_file, "%-25s :%d\n\n", "tp_stream_desc_len", transport_desc_length));

	/*			Transport descriptor Parsing */
				for(j=0; j<transport_desc_length;)
				{				
					desc.tag = *pbuffer++;
					desc.len = *pbuffer++;	
					desc.fp_output = sec_filter->fp_output_file;

					desc.pbuffer = (UINT8 *)malloc(desc.len);
					memcpy(desc.pbuffer, pbuffer, desc.len);
					
					if(fp_DescParserArr[desc.tag] != NULL)
					{
						fp_DescParserArr[desc.tag](&desc);
					}

					DBG_PRINT(("tag %X Len %d\n", desc.tag, desc.len));
					free(desc.pbuffer);
					j+= desc.len + 2;
					pbuffer += desc.len;
					
				}
	
				i += transport_desc_length + 6;
			}

			
			/* All secions of the table is processed, so make this table as processed */
			if(table_ver_sec_parse_status[tableid].sec_num_to_parse == last_sec_num)
			{
				table_ver_sec_parse_status[tableid].ver_processed = version_num;
				table_ver_sec_parse_status[tableid].sec_num_to_parse = 0;				
			}
			else
			{
				table_ver_sec_parse_status[tableid].sec_num_to_parse++;
			}
			
		}

	}
	else
	{
		DBG_PRINT(("Version of NIT table id %d is same\n", tableid));
	}
		
	EXIT_PRINT(("EXIT func %s Line %d \n", __func__, __LINE__));
}



void SDT_Parser(Sec_Filter * sec_filter)
{
	UINT8 tableid, sec_num, last_sec_num, cur_next_ind, version_num, 
		eit_schedule_flag, eit_pf_flag,running_status, free_CA_mode;
	UINT16 sec_length, service_id, desc_loop_len, transport_stream_id, original_network_id;
	UINT32 i, j; 
	Descriptor_t	desc;
		
	UINT8 * pbuffer = (unsigned char *)sec_filter->pBuffer;

	ENTRY_PRINT(("ENTRY func %s Line %d \n", __func__, __LINE__));

#ifdef DEBUG_CODE
	temp = (unsigned char *)sec_filter->pBuffer;

	for(i=0; i<sec_filter->FilledSize; i++)
	{
		DBG_PRINT(("%02X ", *temp++));
	}
	getch();
		
#endif /* DEBUG_CODE */
		
	tableid = *pbuffer++;
	sec_length = (*pbuffer++)& 0x0F;
	sec_length <<=8;
	sec_length |= *pbuffer++;
	       
	transport_stream_id = *pbuffer++;
	transport_stream_id <<=8;
	transport_stream_id |= *pbuffer++;
	
	version_num = *pbuffer & 0x3E;
	version_num >>=1;
	cur_next_ind = *pbuffer++ & 0x01;
	sec_num = *pbuffer++;
	last_sec_num = *pbuffer++;

	original_network_id = *pbuffer++;
	original_network_id <<=8;
	original_network_id |= *pbuffer++;

	/* Reserved byte */
	pbuffer++;
	
	if(table_ver_sec_parse_status[tableid].ver_processed != version_num )
	{
		/* check the section to parse*/
		if(table_ver_sec_parse_status[tableid].sec_num_to_parse == sec_num )	
		{

			INFO_PRINT((sec_filter->fp_output_file, "************* %s SDT *************\n",(tableid==0x42)?"ACTUAL":"OTHER" ));
			INFO_PRINT((sec_filter->fp_output_file, "%-25s :0x%X\n", "tableid", tableid));
			INFO_PRINT((sec_filter->fp_output_file, "%-25s :%d\n", "tp_stream_id", transport_stream_id));
			INFO_PRINT((sec_filter->fp_output_file, "%-25s :%d\n", "sec_length", sec_length));
			INFO_PRINT((sec_filter->fp_output_file, "%-25s :%d\n", "version_num", version_num)); 
			INFO_PRINT((sec_filter->fp_output_file, "%-25s :%d\n", "cur_next_ind", cur_next_ind));	
			INFO_PRINT((sec_filter->fp_output_file, "%-25s :%d\n", "sec_num", sec_num));
			INFO_PRINT((sec_filter->fp_output_file, "%-25s :%d\n", "last_sec_num", last_sec_num));
			INFO_PRINT((sec_filter->fp_output_file, "%-25s :%d\n", "orig_network_id", original_network_id));


			for(i=0; i<sec_length - 12;)
			{
				service_id = *pbuffer++;
				service_id <<=8;
				service_id |= *pbuffer++; 

				eit_schedule_flag = (*pbuffer & 0x02) >> 1;
				eit_pf_flag = (*pbuffer++ & 0x01);
				
				running_status = (*pbuffer & 0xE0) >> 5;
				free_CA_mode = (*pbuffer & 0x10) >> 4;

				desc_loop_len = (*pbuffer++ & 0x0F) << 8;
				desc_loop_len |= *pbuffer++;
			
				INFO_PRINT((sec_filter->fp_output_file, "%-25s :%d\n", "service_id", service_id));
				INFO_PRINT((sec_filter->fp_output_file, "%-25s :%d\n", "eit_schedule_flag", eit_schedule_flag));
				INFO_PRINT((sec_filter->fp_output_file, "%-25s :%d\n", "eit_pf_flag", eit_pf_flag));
				INFO_PRINT((sec_filter->fp_output_file, "%-25s :%d\n", "running_status", running_status));
				INFO_PRINT((sec_filter->fp_output_file, "%-25s :%d\n", "free_CA_mode", free_CA_mode));
				INFO_PRINT((sec_filter->fp_output_file, "%-25s :%d\n", "desc_loop_len", desc_loop_len));


				INFO_PRINT((sec_filter->fp_output_file,"\n============= SERVICE DESCRIPTORS =============\n\n"));

				/*	Service descriptor Parsing */
				for(j=0; j<desc_loop_len;)
				{
					desc.tag = *pbuffer++;
					desc.len = *pbuffer++;	
					desc.fp_output = sec_filter->fp_output_file;
					
					desc.pbuffer = (UINT8 *)malloc(desc.len);
					memcpy(desc.pbuffer, pbuffer, desc.len);
					
					if(fp_DescParserArr[desc.tag] != NULL)
					{
						fp_DescParserArr[desc.tag](&desc);
					}

					DBG_PRINT(("tag %X Len %d\n", desc.tag, desc.len));
			
					free(desc.pbuffer);
					j+= desc.len + 2;
					pbuffer += desc.len;
				
				}

				i += desc_loop_len + 5;

			INFO_PRINT((sec_filter->fp_output_file,"\n============= END OF SERVICE DESCRIPTORS =============\n\n"));
				
				
			}

			/* All secions of the table is processed, so make this table as processed */
			if(table_ver_sec_parse_status[tableid].sec_num_to_parse == last_sec_num)
			{
				table_ver_sec_parse_status[tableid].ver_processed = version_num;
				table_ver_sec_parse_status[tableid].sec_num_to_parse = 0;				
			}
			else
			{
				table_ver_sec_parse_status[tableid].sec_num_to_parse++;
			}
						
		}

	}
	else
	{
		DBG_PRINT(("Version of NIT table id %d is same\n", tableid));
	}
		
	EXIT_PRINT(("EXIT func %s Line %d \n", __func__, __LINE__));
}


void BAT_Parser(Sec_Filter * sec_filter)
{
	UINT8 tableid,sec_num, last_sec_num, cur_next_ind, version_num;
	UINT16 bouquet_desc_length, transport_stream_loop_len, 
		transport_stream_id, original_network_id,transport_desc_length, bouquet_id;
	UINT32 sec_length,i, j; 
	Descriptor_t	desc;
		
	UINT8 * pbuffer = (unsigned char *)sec_filter->pBuffer;

	ENTRY_PRINT(("ENTRY func %s Line %d \n", __func__, __LINE__));

#ifdef DEBUG_CODE
	temp = (unsigned char *)sec_filter->pBuffer;

	for(i=0; i<sec_filter->FilledSize; i++)
	{
		DBG_PRINT(("%02X ", *temp++));
	}
	getch();
		
#endif /* DEBUG_CODE */
		
	tableid = *pbuffer++;
	sec_length = (*pbuffer++)& 0x0F;
	sec_length <<=8;
	sec_length |= *pbuffer++;
	bouquet_id = *pbuffer++;
	bouquet_id <<=8;
	bouquet_id |= *pbuffer++;
	version_num = *pbuffer & 0x3E;
	version_num >>=1;
	cur_next_ind = *pbuffer++ & 0x01;
	sec_num = *pbuffer++;
	last_sec_num = *pbuffer++;


	bouquet_desc_length = (*pbuffer++ & 0x0F);		
	bouquet_desc_length <<=8;
	bouquet_desc_length |= *pbuffer++;

	/* check for processed table version */
	if(table_ver_sec_parse_status[tableid].ver_processed != version_num )
	{
		/* check the section to parse*/
		if(table_ver_sec_parse_status[tableid].sec_num_to_parse == sec_num )	
		{

			INFO_PRINT((sec_filter->fp_output_file, "************* BAT *************\n"));
			INFO_PRINT((sec_filter->fp_output_file, "%-25s :0x%X\n", "tableid", tableid));
			INFO_PRINT((sec_filter->fp_output_file, "%-25s :%d\n", "bouquet_id", bouquet_id));
			INFO_PRINT((sec_filter->fp_output_file, "%-25s :%d\n", "sec_length", sec_length));
			INFO_PRINT((sec_filter->fp_output_file, "%-25s :%d\n", "version_num", version_num)); 
			INFO_PRINT((sec_filter->fp_output_file, "%-25s :%d\n", "cur_next_ind", cur_next_ind));	
			INFO_PRINT((sec_filter->fp_output_file, "%-25s :%d\n", "sec_num", sec_num));
			INFO_PRINT((sec_filter->fp_output_file, "%-25s :%d\n", "last_sec_num", last_sec_num));
			INFO_PRINT((sec_filter->fp_output_file, "%-25s :%d\n", "bouq_desc_length", bouquet_desc_length));

			INFO_PRINT((sec_filter->fp_output_file, "============= BOUQUET DESCRIPTORS =============\n\n"));


		/*		Network Descriptor processing */
			for(i=0; i<bouquet_desc_length;)
			{
				desc.tag = *pbuffer++;
				desc.len = *pbuffer++;	
				desc.fp_output = sec_filter->fp_output_file;
				
				desc.pbuffer = (UINT8 *)malloc(desc.len);
				memcpy(desc.pbuffer, pbuffer, desc.len);
				
				if(fp_DescParserArr[desc.tag] != NULL)
				{
					fp_DescParserArr[desc.tag](&desc);
				}

				DBG_PRINT(("tag %X Len %d\n", desc.tag, desc.len));
				free(desc.pbuffer);
				i+= desc.len + 2;
				pbuffer += desc.len;
			}

			INFO_PRINT((sec_filter->fp_output_file, "============= END OF BOUQUET DESCRIPTORS ===============\n\n"));
			
			transport_stream_loop_len = *pbuffer++ & 0x0F;
			transport_stream_loop_len <<= 8;
			transport_stream_loop_len |= *pbuffer++; 

			INFO_PRINT((sec_filter->fp_output_file, "%-25s :%d\n", "tp_stream_loop_len", transport_stream_loop_len));

			for(i=0; i<transport_stream_loop_len;)
			{
				transport_stream_id = *pbuffer++;
				transport_stream_id <<=8;
				transport_stream_id |= *pbuffer++; 
				
				original_network_id = *pbuffer++;
				original_network_id <<=8;
				original_network_id |= *pbuffer++; 

				transport_desc_length = *pbuffer++ & 0x0F;
				transport_desc_length <<= 8;
				transport_desc_length |= *pbuffer++; 
				
				INFO_PRINT((sec_filter->fp_output_file, "%-25s :%d\n", "transport_stream_id", transport_stream_id));
				INFO_PRINT((sec_filter->fp_output_file, "%-25s :%d\n", "orig_network_id", original_network_id));
				INFO_PRINT((sec_filter->fp_output_file, "%-25s :%d\n", "transport_desc_len", transport_desc_length));

	/*			Transport descriptor Parsing */
				for(j=0; j<transport_desc_length;)
				{				
					desc.tag = *pbuffer++;
					desc.len = *pbuffer++;	
					desc.fp_output = sec_filter->fp_output_file;

					desc.pbuffer = (UINT8 *)malloc(desc.len);
					memcpy(desc.pbuffer, pbuffer, desc.len);
					
					if(fp_DescParserArr[desc.tag] != NULL)
					{
						fp_DescParserArr[desc.tag](&desc);
					}

					DBG_PRINT(("tag %X Len %d\n", desc.tag, desc.len));
					free(desc.pbuffer);
					j+= desc.len + 2;
					pbuffer += desc.len;
					
				}
	
				i += transport_desc_length + 6;
			}

			/* All secions of the table is processed, so make this table as processed */
			if(table_ver_sec_parse_status[tableid].sec_num_to_parse == last_sec_num)
			{
				table_ver_sec_parse_status[tableid].ver_processed = version_num;
				table_ver_sec_parse_status[tableid].sec_num_to_parse = 0;				
			}
			else
			{
				table_ver_sec_parse_status[tableid].sec_num_to_parse++;
			}
			
		}
		

	}
	else
	{
		DBG_PRINT(("Version of NIT table id %d is same\n", tableid));
	}
		
	EXIT_PRINT(("EXIT func %s Line %d \n", __func__, __LINE__));
}


void EIT_Parser(Sec_Filter * sec_filter)
{
	UINT8 tableid,sec_num, last_sec_num, cur_next_ind, version_num,
		segment_last_section_number, last_table_id, running_status, free_CA_mode;
	UINT16 event_id, descriptor_stream_loop_len, 
		transport_stream_id, original_network_id,transport_desc_length, service_id, MJD;
	UINT32 sec_length,i, j, UTC, duration; 
	Descriptor_t	desc;
		
	UINT8 * pbuffer = (unsigned char *)sec_filter->pBuffer;

	ENTRY_PRINT(("ENTRY func %s Line %d \n", __func__, __LINE__));

#ifdef DEBUG_CODE
	temp = (unsigned char *)sec_filter->pBuffer;

	for(i=0; i<sec_filter->FilledSize; i++)
	{
		DBG_PRINT(("%02X ", *temp++));
	}
	getch();
		
#endif /* DEBUG_CODE */
		
	tableid = *pbuffer++;
	sec_length = (*pbuffer++)& 0x0F;
	sec_length <<=8;
	sec_length |= *pbuffer++;
	service_id = *pbuffer++;
	service_id <<=8;
	service_id |= *pbuffer++;
	
	version_num = *pbuffer & 0x3E;
	version_num >>=1;
	cur_next_ind = *pbuffer++ & 0x01;
	sec_num = *pbuffer++;
	last_sec_num = *pbuffer++;

	transport_stream_id = *pbuffer++;
	transport_stream_id <<=8;
	transport_stream_id |= *pbuffer++;
	
	original_network_id = *pbuffer++;
	original_network_id <<=8;
	original_network_id |= *pbuffer++;

	segment_last_section_number = *pbuffer++;		
	last_table_id = *pbuffer++;


	INFO_PRINT((sec_filter->fp_output_file, "************* EIT *************\n"));
	INFO_PRINT((sec_filter->fp_output_file, "%-25s :0x%X\n", "tableid", tableid));
	INFO_PRINT((sec_filter->fp_output_file, "%-25s :%d\n", "service_id", service_id));
	INFO_PRINT((sec_filter->fp_output_file, "%-25s :%d\n", "sec_length", sec_length));
	INFO_PRINT((sec_filter->fp_output_file, "%-25s :%d\n", "version_num", version_num)); 
	INFO_PRINT((sec_filter->fp_output_file, "%-25s :%d\n", "cur_next_ind", cur_next_ind));	
	INFO_PRINT((sec_filter->fp_output_file, "%-25s :%d\n", "sec_num", sec_num));
	INFO_PRINT((sec_filter->fp_output_file, "%-25s :%d\n", "last_sec_num", last_sec_num));
	INFO_PRINT((sec_filter->fp_output_file, "%-25s :%d\n", "transport_stream_id", transport_stream_id));	
	INFO_PRINT((sec_filter->fp_output_file, "%-25s :%d\n", "orig_network_id", original_network_id));
	INFO_PRINT((sec_filter->fp_output_file, "%-25s :%d\n", "seg_last_sec_num", last_sec_num));
	INFO_PRINT((sec_filter->fp_output_file, "%-25s :0x%X\n", "last_table_id", last_table_id));

	/*	Events processing */
	for(i=0; i< sec_length - 15;)
	{
		event_id = *pbuffer++;
		event_id <<= 8;
		event_id |= *pbuffer++;

		MJD	= *pbuffer++;
		MJD <<= 8;
		MJD |= *pbuffer++;

		UTC = *pbuffer++;
		UTC <<= 8;
		UTC |= *pbuffer++;
		UTC <<= 8;
		UTC |= *pbuffer++;
	 			
		duration = *pbuffer++;
		duration <<= 8;
		duration |= *pbuffer++;
		duration <<= 8;
		duration |= *pbuffer++;

		running_status = (*pbuffer & 0xE0) >>5;
		free_CA_mode = (*pbuffer & 0x10) >> 4;
		descriptor_stream_loop_len = *pbuffer++ & 0x0F;
		descriptor_stream_loop_len <<= 8;
		descriptor_stream_loop_len |= *pbuffer++; 

		INFO_PRINT((sec_filter->fp_output_file, "************* EVENT *************\n"));
		INFO_PRINT((sec_filter->fp_output_file, "%-25s :%d\n", "event_id", event_id));
		INFO_PRINT((sec_filter->fp_output_file, "%-25s :0x%X\n", "MJD", MJD));
		INFO_PRINT((sec_filter->fp_output_file, "%-25s :0x%X\n", "UTC", UTC));
		INFO_PRINT((sec_filter->fp_output_file, "%-25s :0x%X\n", "duration", duration));
		INFO_PRINT((sec_filter->fp_output_file, "%-25s :%d\n", "running_status", running_status));
		INFO_PRINT((sec_filter->fp_output_file, "%-25s :%d\n", "free_CA_mode", free_CA_mode)); 
		INFO_PRINT((sec_filter->fp_output_file, "%-25s :%d\n", "desc_stream_loop_len", descriptor_stream_loop_len)); 

		INFO_PRINT((sec_filter->fp_output_file,"\n============= EVENT DESCRIPTORS =============\n\n"));


		/*Conversion of MJD*/
//		ConvertMJD(MJD);

		/* Events processing */				
		for(j=0; j < descriptor_stream_loop_len;)
		{
			desc.tag = *pbuffer++;
			desc.len = *pbuffer++;	
			desc.fp_output = sec_filter->fp_output_file;
			
			desc.pbuffer = (UINT8 *)malloc(desc.len);
			memcpy(desc.pbuffer, pbuffer, desc.len);
			
			if(fp_DescParserArr[desc.tag] != NULL)
			{
				fp_DescParserArr[desc.tag](&desc);
			}
			
			DBG_PRINT(("tag %X Len %d\n", desc.tag, desc.len));
			free(desc.pbuffer);
			j+= desc.len + 2;
			pbuffer += desc.len;		
		}

		i += descriptor_stream_loop_len + 12;

		INFO_PRINT((sec_filter->fp_output_file,"\n============= END OF EVENT DESCRIPTORS =============\n\n"));
		
	}

	EXIT_PRINT(("EXIT func %s Line %d \n", __func__, __LINE__));
}


void TDT_TOT_Parser(Sec_Filter *sec_filter)
{
	UINT8 tableid,i;
	UINT32 sec_length, UTC; 
	UINT8 * pbuffer = (unsigned char *)sec_filter->pBuffer;
	UINT16 MJD, descriptors_loop_length;
	Descriptor_t	desc;

	ENTRY_PRINT(("ENTRY func %s Line %d \n", __func__, __LINE__));

#ifdef DEBUG_CODE
	for(i=0; i<sec_filter->FilledSize; i++)
	{
		DBG_PRINT(("%02X ", *pbuffer++));
	}
	getch();
	
#endif /* BOGUS */
	

	pbuffer = sec_filter->pBuffer;

	tableid = *pbuffer++;
	sec_length = (*pbuffer++)& 0x0F;
	sec_length <<=8;
	sec_length |= *pbuffer++;
	
	MJD = *pbuffer++;
	MJD <<= 8;
	MJD |= *pbuffer++;
	
	INFO_PRINT((sec_filter->fp_output_file, "============= %s ==============\n", (tableid == 0x70)?"TDT":"TOT"));
	INFO_PRINT((sec_filter->fp_output_file, "%-25s :0x%X\n", "tableid", tableid));
	INFO_PRINT((sec_filter->fp_output_file, "%-25s :%d\n", "sec_length", sec_length));
	INFO_PRINT((sec_filter->fp_output_file, "%-25s :%d\n", "MJD", MJD));	
	INFO_PRINT((sec_filter->fp_output_file, "%-25s :%.2X%.2X%.2X\n", "UTC", *pbuffer, *(pbuffer + 1), *(pbuffer + 2)));	

	pbuffer += 3;

	if(tableid == TOT_TABLE_ID)
	{
		descriptors_loop_length = (*pbuffer++) & 0x0F;		
		descriptors_loop_length <<= 8;
		descriptors_loop_length |= *pbuffer++;

		for(i=0; i<descriptors_loop_length;)
		{			
			desc.tag = *pbuffer++;
			desc.len = *pbuffer++;	
			desc.fp_output = sec_filter->fp_output_file;
			
			desc.pbuffer = (UINT8 *)malloc(desc.len);
			memcpy(desc.pbuffer, pbuffer, desc.len);
			
			if(fp_DescParserArr[desc.tag] != NULL)
			{
				fp_DescParserArr[desc.tag](&desc);
			}
			
			DBG_PRINT(("tag %X Len %d\n", desc.tag, desc.len));
			free(desc.pbuffer);
			i+= desc.len + 2;
			pbuffer += desc.len;
		}
	}

				
	DBG_PRINT(("CRC %X %X %X %X\n", *pbuffer++, *pbuffer++, *pbuffer++, *pbuffer++));
	
	EXIT_PRINT(("EXIT func %s Line %d \n", __func__, __LINE__));
	
}



void ConvertMJD(UINT16 MJD)
{

	INT32	Year, Month, Day, temp;

	Year = ( (MJD - 150782)/36525);
	Month = ((MJD - 149561 - (Year * 36525)) / 306001);
	Day = MJD - 14956 - (Year * 36525) - (Month * 306001);

	if((Month == 14)|| (Month == 15))
	{
		temp = 1;
	}
	else
	{
		temp = 0;
	}

	Year += temp;
	Month = Month - 1 - temp * 12;

	DBG_PRINT(("%d/%d/%d \n", Day, Month, Year ));

}

