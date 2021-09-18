/*
  *	File	: pid_filter.c
  *	Author : Sendhilkumar
  *	Description : Contains pid filtering and triggers section filtering
  */


#include "dvb_parser.h"

extern UINT32 Pid_Table[];


/* Extracts the one complete payload of PES or section and passes to next level */
void PidFilter(UINT8 * pBuffer, UINT32 BufferSize)
{

	UINT32 Bytesread = 0, PUSI, Trans_err_indicator, cont_counter =0, pointer_field;
	UINT32 Offset = 0,Byte2, Byte3, Byte4, Byte5; 
	UINT16 PID; 
	Tpid_Filter * TPid_filter;

	ENTRY_PRINT(("ENTRY func %s Line %d \n", __func__, __LINE__));

	while(BufferSize - Bytesread > TS_PACK_SIZE)
	{
		if(*pBuffer == SYNC_BYTE )
		{
			Byte2 = *(pBuffer + 1);
			Byte3 = *(pBuffer + 2);
			Byte4 = *(pBuffer + 3);
			Byte5 = *(pBuffer + 4);
			
			Trans_err_indicator = (Byte2 >> 7);
			PUSI = (Byte2 & 0x40) >> 6;
			PID = (Byte2 & 0x1F) << 8;
			PID |= Byte3;
			cont_counter = Byte4 & 0x0F;

			TPid_filter = (Tpid_Filter *)Pid_Table[PID];
			if(TPid_filter != NULL )
			{
			
				if( PUSI )					
				{
					TPid_filter->First_PUSI_Found = TRUE;
					if(TPid_filter->type == SECTION)
					{
						pointer_field = Byte5;
						memcpy((TPid_filter->pBuffer + TPid_filter->writeptr), (pBuffer + 4 + 1 ), \
							pointer_field);		
						TPid_filter->writeptr += pointer_field;
						TPid_filter->FilledSize += pointer_field;

						DBG_PRINT(("One complete Section extracted for the PID %d Size %d\n",PID, TPid_filter->FilledSize ));
/*
						if(TPid_filter->writeptr == 0)						
						{
							getch();
						}
*/
						Section_Filter(TPid_filter);

						/*Reset the Bufer pointers, since section buffer is consumed */
						TPid_filter->writeptr = 0;						
						TPid_filter->FilledSize = 0;
						
						memcpy((TPid_filter->pBuffer + TPid_filter->writeptr), \
							(pBuffer + 4 + pointer_field + 1), (TS_PACK_PL_SIZE - (pointer_field + 1)));
						
						TPid_filter->writeptr += TS_PACK_PL_SIZE - (pointer_field +1);						
						TPid_filter->FilledSize += TS_PACK_PL_SIZE - (pointer_field +1);
					}
					else
					{			
						DBG_PRINT(("One complete ES extracted for the PID %d\n",PID));					
						/*Reset the Bufer pointers*/
					}

				}
				else
				{
					if(TPid_filter->First_PUSI_Found) /*TO handle if TS stream has a first section without PUSI set */
					{
						memcpy((TPid_filter->pBuffer + TPid_filter->writeptr), \
							(pBuffer + 4 ), TS_PACK_PL_SIZE );
						TPid_filter->writeptr += TS_PACK_PL_SIZE;						
						TPid_filter->FilledSize += TS_PACK_PL_SIZE;
					}
					
				}
			}
			DBG_PRINT((""));

			pBuffer += TS_PACK_SIZE;
			Bytesread += TS_PACK_SIZE;
		}
		else
		{
			DBG_PRINT(("Sync lost!!! Trying to regain sync\n"));

			Offset = CheckSync(pBuffer);
			
			pBuffer += Offset;
			Bytesread += Offset;
		}
	}

	DBG_PRINT(("Pidfilter processing\n"));
	
	EXIT_PRINT(("EXIT func %s Line %d \n", __func__, __LINE__));

}

/*returns offset of TS stream after acquiring sync byte*/
UINT8 CheckSync(UINT8 * pBuffer)
{

UINT32 offset = 0;
UINT32	i = 0;
UINT8 flag = 0;

ENTRY_PRINT(("ENTRY func %s Line %d \n", __func__, __LINE__));

while(1)
{
	DBG_PRINT(("while loop\n"));
	if((*pBuffer == SYNC_BYTE )&& !flag)
	{
		for(i=0;i<=(7*TS_PACK_SIZE);)
		{
			if(*(pBuffer + i) == 0x47)
			{
				flag = 1;
				i += TS_PACK_SIZE;
				continue;
			}
			else
			{
				flag = 0;
				break;
			}
		}
	}

	if(flag)
	{
		DBG_PRINT(("Sync found at the offset %d\n", offset));
		break;
	}
	else
	{
		offset++;
		pBuffer++;
	}

}

EXIT_PRINT(("EXIT func %s Line %d \n", __func__, __LINE__));

return offset;
}


