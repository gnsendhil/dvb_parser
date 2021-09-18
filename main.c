/*

  *	File	: main.c
  *	Author : Sendhilkumar
  *	Description : Opens the TS file, initializes pid filters and trigerrs section filtering and 
  *	parsing.
  */

#include "dvb_parser.h"

#define STREAM_PATH_NAME "tatasky.ts"

/* Pid table which contains the registered PID's */
UINT32 Pid_Table[0x1FFF];
PMT_Info	Stream_PMT_Info;


/* First two bytes for PID and last two bytes for Section mask */

/* PID LIST */
UINT16 PID_LIST[] = {	
//						PAT_PID,
//						NIT_PID,
						SDT_PID, /* SDT/BAT */
//						EIT_PID,
//						TDT_PID,
					};



void main()
{

	DBG_PRINT(("~~~~~~~~~~~~DVB PARSER ~~~~~~~~~~~~~~ \n"));

	DBG_PRINT(("sizeof(int) %d\n", sizeof(int)));
	DBG_PRINT(("sizeof(short) %d\n", sizeof(short)));
	DBG_PRINT(("sizeof(short int) %d\n", sizeof(short int)));
	DBG_PRINT(("sizeof(long) %d\n", sizeof(long)));


	FILE * fd;
	int File_Length, TS_Offset;
	char * TsBuffer;

	ENTRY_PRINT(("ENTRY func %s Line %d \n", __func__, __LINE__));

	
	fd = fopen(STREAM_PATH_NAME, "rb");
	if (fd == NULL)
	{
		DBG_PRINT(("Error opening the file %s \n", STREAM_PATH_NAME ));
	}
	else
	{
		DBG_PRINT(("Opening the file %s is successful \n", STREAM_PATH_NAME ));		
	}

	fseek(fd,0,SEEK_END);
	File_Length = ftell(fd);
	
	fseek(fd,0,SEEK_SET);

	DBG_PRINT(("Size of file %d \n", File_Length ));

	TsBuffer = malloc(File_Length);
	if(TsBuffer == NULL)
	{
		DBG_PRINT(("Unable to allocate memory for TS buffer\n"));
		free(TsBuffer);		
		fclose(fd);
		exit(0);
	}

	if ( fread(TsBuffer, 1, File_Length, fd) != File_Length)
	{
		DBG_PRINT(("Data is not copied to bufffer \n"));
	}

	Init_PidFilters();
	Register_section_filters();
	Register_Descriptors();
	
	TS_Offset = CheckSync(TsBuffer);
	PidFilter(TsBuffer + TS_Offset, File_Length - TS_Offset);

	free(TsBuffer);
	DBG_PRINT(("End of TS Parsing\n"));
	Deschedule_PMT();
	DeInit_PidFilters();
	UnRegister_section_filters();
	
	fclose(fd);

	EXIT_PRINT(("EXIT func %s Line %d \n", __func__, __LINE__));

getch();

}

/* Register Pid filters for DVB tables */
void Init_PidFilters(void)
{
	UINT8	i;
	UINT32	PID;
	Tpid_Filter * pid_filter;

	ENTRY_PRINT(("ENTRY func %s Line %d \n", __func__, __LINE__));
	
	for(i=0; i<(sizeof(PID_LIST)/sizeof(UINT16));i++)
	{
		pid_filter = (Tpid_Filter *)malloc(sizeof(Tpid_Filter));		
		memset(pid_filter,0,sizeof(Tpid_Filter));
		
		PID = PID_LIST[i];

		/*Initialize pid filter for PAT */
		pid_filter->pid = PID;
		pid_filter->type = SECTION;
		pid_filter->First_PUSI_Found = FALSE;
		pid_filter->pBuffer = (UINT8 *)malloc(PID_FILTER_SIZE);

		if(pid_filter->pBuffer == NULL)
		{
			CRIC_DEBUG_PRINT(("FILE %s, LINE %d, Malloc failed \n", __FILE__, __LINE__ ));
		}
		pid_filter->totSize = PID_FILTER_SIZE;
		
		Pid_Table[PID]=(UINT32)pid_filter;

		
	}

	EXIT_PRINT(("EXIT func %s Line %d \n", __func__, __LINE__));
	
}


void DeInit_PidFilters(void)
{
	UINT8	i;
	Tpid_Filter * pid_filter;
	UINT32	PID;

	ENTRY_PRINT(("ENTRY func %s Line %d \n", __func__, __LINE__));
	
	for(i=0; i<(sizeof(PID_LIST)/sizeof(UINT16));i++)
	{
		PID = PID_LIST[i];
		pid_filter = (Tpid_Filter * ) Pid_Table[PID];
		free(pid_filter->pBuffer);
		free(pid_filter);
		Pid_Table[PID] = NULL;
	}
	
	EXIT_PRINT(("EXIT func %s Line %d \n", __func__, __LINE__));

}


void Schedule_PMT(UINT16 pid)
{
	Tpid_Filter * TPid_filter;

	ENTRY_PRINT(("ENTRY func %s Line %d \n", __func__, __LINE__));

	/*Add to pid table, if the entry is not present */
	if(Pid_Table[pid]== NULL)
	{
		TPid_filter = (Tpid_Filter *)malloc(sizeof(Tpid_Filter));
		memset(TPid_filter,0,sizeof(Tpid_Filter));

		TPid_filter->totSize = PID_FILTER_SIZE;
		TPid_filter->pid	 = pid;
		TPid_filter->type	 = SECTION;
		TPid_filter->pBuffer = (UINT8 *)malloc(PID_FILTER_SIZE);
		Pid_Table[pid]   = TPid_filter;

		/*Update the PMT Pid details for descheduling */
		Stream_PMT_Info.PMT_PID[Stream_PMT_Info.Prog_Count] = pid;
		Stream_PMT_Info.Prog_Count++;
	}
	
	EXIT_PRINT(("EXIT func %s Line %d \n", __func__, __LINE__));

}

void Deschedule_PMT(void)
{
	Tpid_Filter * TPid_filter;
	UINT8	i;

	ENTRY_PRINT(("ENTRY func %s Line %d \n", __func__, __LINE__));
	
	for(i=0; i<Stream_PMT_Info.Prog_Count; i++)
	{
		TPid_filter = Pid_Table[Stream_PMT_Info.PMT_PID[i]];
		if(TPid_filter != NULL)
		{
			if(TPid_filter->pBuffer != NULL)
			{
				free(TPid_filter->pBuffer);
			}
			
			free(TPid_filter);
		}
		Pid_Table[Stream_PMT_Info.PMT_PID[i]] = 0;
	}
	
	EXIT_PRINT(("EXIT func %s Line %d \n", __func__, __LINE__));

}


