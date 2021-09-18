/*
  *	File	: sec_filter.c
  *	Author : Sendhilkumar
  *	Description : Section filtering and triggers section parsing
  */

#include "dvb_parser.h"

UINT32 Sec_Table[0xFF];

SEC_PARSER sec_parser[] = { 
				{0x00, FALSE, PAT_Parser, 		SIZE_1K, "PAT"},
				{0x02, FALSE, PMT_Parser, 		SIZE_1K, "PMT"},
				{0x40, FALSE, NIT_Parser, 		SIZE_1K, "NIT-ACT"},
				{0x41, FALSE, NIT_Parser, 		SIZE_1K, "NIT-OTH"},
				{0x42, FALSE, SDT_Parser, 		SIZE_1K, "SDT-ACT"},
				{0x46, FALSE, SDT_Parser, 		SIZE_1K, "SDT-OTH"},
				{0x4A, FALSE, BAT_Parser, 		SIZE_1K, "BAT"},
				{0x4E, FALSE, EIT_Parser, 		SIZE_4K, "EIT-PF-ACT"},
				{0x4F, FALSE, EIT_Parser,		SIZE_4K, "EIT-PF-OTH"},
				{0x5F, TRUE,  EIT_Parser, 		SIZE_4K, "EIT-SCH-ACT"},
				{0x6F, TRUE,  EIT_Parser,		SIZE_4K, "EIT-SCH-OTH"},
				{0x70, FALSE, TDT_TOT_Parser, 	SIZE_1K, "TDT"},				
				{0x73, FALSE, TDT_TOT_Parser, 	SIZE_1K, "TOT"},
			};


void Register_section_filters(void)
{
	UINT8	i, j;
	Sec_Filter * p_sec_filter;

	for(i=0; i<(sizeof(sec_parser)/sizeof(SEC_PARSER));i++)
	{
		p_sec_filter = (Sec_Filter *)malloc(sizeof(Sec_Filter));
		memset(p_sec_filter,0,sizeof(Sec_Filter));

		p_sec_filter->table_id = sec_parser[i].table_id;
		p_sec_filter->Mask		= sec_parser[i].Mask;
		p_sec_filter->pBuffer = (UINT8 *)malloc(sec_parser[i].sec_size);
		
		if(p_sec_filter->pBuffer == NULL)
		{
			CRIC_DEBUG_PRINT(("FILE %s, LINE %d, Malloc failed \n", __FILE__, __LINE__ ));
		}

		DBG_PRINT(("In Register section filter table id %d\n", p_sec_filter->table_id ));
		p_sec_filter->totSize = sec_parser[i].sec_size;
		p_sec_filter->fptr_Parser = sec_parser[i].ptr_func;
		p_sec_filter->last_sec_ver_processed = 0xFF;
		p_sec_filter->sec_num_to_parse = 0x00;

		p_sec_filter->fp_output_file = fopen(strcat(sec_parser[i].Table_name, ".txt"), "w");

		DBG_PRINT(("After registering \n"));

		if(p_sec_filter->fp_output_file == NULL )
		{
			DBG_PRINT(("Error opening the file %s \n", sec_parser[i].Table_name ));
		}
		else
		{
			DBG_PRINT(("Opening the file %s is successful \n", sec_parser[i].Table_name )); 	
		}

		if(p_sec_filter->Mask)
		{
			/*Implemented for LSB 4bits */
			for(j = (p_sec_filter->table_id & 0xF0); j<=p_sec_filter->table_id; j++)
			{
				Sec_Table[j] = (UINT32)p_sec_filter;				
			}
		
		}
		else
		{
			Sec_Table[sec_parser[i].table_id] = (UINT32)p_sec_filter;
		}
	}

}


void UnRegister_section_filters(void)
{
	UINT8	i, table_id;
	Sec_Filter * p_sec_filter;

	ENTRY_PRINT(("ENTRY func %s Line %d \n", __func__, __LINE__));
	
	for(i=0; i<(sizeof(sec_parser)/sizeof(SEC_PARSER));i++)
	{
		table_id = sec_parser[i].table_id;
		p_sec_filter = (Sec_Filter *) Sec_Table[table_id];
		free(p_sec_filter->pBuffer);
		fclose(p_sec_filter->fp_output_file);
		free(p_sec_filter);	
	}

	/* Reset all func pointers */
	memset(Sec_Table, 0x00, 0xFF);
	
	EXIT_PRINT(("EXIT func %s Line %d \n", __func__, __LINE__));

}


void Section_Filter(Tpid_Filter *pidfilter_buffer )
{

	UINT8 *temp = pidfilter_buffer->pBuffer, Sec_Mask;
	UINT32 table_id;
	UINT32 section_length = 0, total_sec_length_processed =0;
	Sec_Filter * p_SecFilter;

	ENTRY_PRINT(("ENTRY func %s Line %d \n", __func__, __LINE__));

	DBG_PRINT(("FilledSize %d ", pidfilter_buffer->FilledSize));
	
#ifdef DEBUG_SEC_FILTER_CODE
	UINT8	* pbuffer, i;
	pbuffer = pidfilter_buffer->pBuffer;

		for(i=0; i<pidfilter_buffer->FilledSize; i++)
		{
			DBG_PRINT(("%X ", *pbuffer++));			
		}

#endif /* DEBUG_SEC_FILTER_CODE */

	if(pidfilter_buffer->FilledSize >= MIN_SEC_SIZE)
	{
		/*	This is required when two complete sections are present in one TS packet.
			Section filter should process all complete sections from the section 
			buffer */
		while(pidfilter_buffer->FilledSize > total_sec_length_processed )
		{
			table_id = *temp;
			section_length = *(temp + 1 ) & 0x0F;
			section_length = section_length  << 8;
			section_length |= *(temp + 2 );		
			DBG_PRINT(("Table id %d length %d\n",table_id,section_length ));

			/*Assumption: 0xFF can't be valid table so ignore and stop current section buffer processing */
			if(table_id == 0xFF)
				break;
				
			if(Sec_Table[table_id] != NULL)
			{
				p_SecFilter = (Sec_Filter *)Sec_Table[table_id];
				memcpy(p_SecFilter->pBuffer, temp, section_length + 3);
				p_SecFilter->FilledSize = section_length + 3;
				p_SecFilter->fptr_Parser(p_SecFilter);
			}
			else
			{
				DBG_PRINT(("Discarded the section as it is not required\n"));
			}
			temp += section_length + 3;
			total_sec_length_processed += section_length;
			
		}
	}
	else
	{		
		DBG_PRINT(("Invalid Section\n"));
	}
	
	EXIT_PRINT(("EXIT func %s Line %d \n", __func__, __LINE__));

}


void CalculateCrc(UINT8 *addr, UINT32 length, UINT32 *crc) 
{
#define CAL_MCRC(rem, dat) (rem) = ((rem) << 4) ^ mmask_CRC[(((rem) >> 28) ^ ((dat) >> 4)) & 0x0f],\
		           (rem) = ((rem) << 4) ^ mmask_CRC[(((rem) >> 28) ^ ((dat))) & 0x0f]
  static const UINT32 mmask_CRC[] =
  {
    0x00000000UL,
    0x04c11db7UL,
    0x09823b6eUL,
    0x0d4326d9UL,
    0x130476dcUL,
    0x17c56b6bUL,
    0x1a864db2UL,
    0x1e475005UL,
    0x2608edb8UL,
    0x22c9f00fUL,
    0x2f8ad6d6UL,
    0x2b4bcb61UL,
    0x350c9b64UL,
    0x31cd86d3UL,
    0x3c8ea00aUL,
    0x384fbdbdUL,
  };

  UINT32 offset;

  *crc = 0xffffffffUL;
  for (offset = 0; offset < length; offset++)                 
  {
    CAL_MCRC (*crc, addr[offset]);
  }
}


