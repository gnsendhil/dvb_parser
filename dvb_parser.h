/*  *	File	: Dvb_parser.h  *	Author : Sendhilkumar  *	Description : Contains structures for Filters and sections.  */

#include<stdio.h>

#define SYNC_BYTE 0x47
#define TS_PACK_SIZE 188
#define TS_PACK_PL_SIZE 184
#define MIN_SEC_SIZE 16	

/*PIDS*/
#define PAT_PID 0x00
#define NIT_PID 0x1010  /*0x478*/
#define SDT_PID 0x1011 /*0x479*/
#define BAT_PID 0x11
#define EIT_PID 0x1012 /*0x47A*/
#define TDT_PID 0x1014


#define PMT_PAYLOAD_SIZE	4

#define PID_FILTER_SIZE (4 * 1024 * 1024)

#define SIZE_1K (1 * 1024 * 1024)
#define SIZE_4K (4 * 1024 * 1024)

#define DEBUG
#define INFO
//#define CRIC_DEBUG
//#define FUNC_ENTRY
//#define FUNC_EXIT
//#define DEBUG_SEC_FILTER_CODE


#ifdef CRIC_DEBUG
#define CRIC_DEBUG_PRINT(X) printf X;
#else
#define CRIC_DEBUG_PRINT(X) 
#endif


#ifdef DEBUG
#define DBG_PRINT(X) printf X;
#else
#define DBG_PRINT(X) 
#endif

#ifdef INFO
#define INFO_PRINT(X) fprintf X
#else
#define INFO_PRINT(X) 
#endif


#ifdef FUNC_ENTRY
#define ENTRY_PRINT(X) printf X;
#else
#define ENTRY_PRINT(X) 
#endif


#ifdef FUNC_EXIT
#define EXIT_PRINT(X) printf X;
#else
#define EXIT_PRINT(X) 
#endif


#define	MASK_BIT_0 0x01
#define	MASK_BIT_1 0x02
#define	MASK_BIT_2 0x04
#define	MASK_BIT_3 0x08
#define	MASK_BIT_4 0x10
#define	MASK_BIT_5 0x20
#define	MASK_BIT_6 0x40
#define	MASK_BIT_7 0x80


/*Datatypes*/

typedef char INT8;
typedef	unsigned char UINT8;
typedef short INT16;
typedef	unsigned short UINT16;
typedef	unsigned int UINT32;
typedef int INT32;

typedef enum tp_payload
{
	ES,
	SECTION
} TP_payload;

typedef enum boolean
{
	FALSE =0,
	TRUE,
} BOOLEAN;


/*Parser function pointers*/
typedef void (*fp_Parser)(struct sec_filter *);

struct sec_filter
{
	UINT8 	table_id;
	UINT8 * pBuffer;
	UINT8 last_sec_ver_processed;
	UINT8 sec_num_to_parse;	
	UINT32 totSize;
	UINT32 FilledSize;
	UINT32 Readptr;
	UINT32 writeptr;	
	BOOLEAN		Mask;
	fp_Parser	fptr_Parser;
	FILE *fp_output_file;
};

typedef struct sec_filter Sec_Filter;

typedef struct tpid_filter
{
	UINT32 pid;
	TP_payload type;
	UINT8	First_PUSI_Found; /* Filtering should happen, only after finding first start of particular packet */
	UINT8 * pBuffer;
	UINT32 totSize;
	UINT32 FilledSize;
	UINT32 Readptr;
	UINT32 writeptr;
}Tpid_Filter;

typedef struct Pmt_info
{
	UINT16	PMT_PID[0XFF];
	UINT16	Prog_Count;
}PMT_Info;

typedef	struct Sec_Parser
{
	UINT8	 	table_id;
	BOOLEAN		Mask;
	fp_Parser	ptr_func;
	UINT32		sec_size;
	UINT8		Table_name[20];
}SEC_PARSER;

typedef	struct table_ver_status
{
	UINT8	 	ver_processed;
	UINT8		sec_num_to_parse;
	/* This is field is used only for PMT*/
	UINT32		processed_prog_ver_sec;
}Table_ver_status;


void PidFilter(UINT8 * pBuffer, UINT32 BufferSize);
UINT8 CheckSync(UINT8 * pBuffer);
void Init_PidFilters(void);
void DeInit_PidFilters(void);
void Register_section_filters(void);
void Register_Descriptors(void);                                             
void Section_Filter(Tpid_Filter *pidfilter_buffer );
void PAT_Parser(Sec_Filter *sec_filter);
void Schedule_PMT(UINT16 pid);
void Deschedule_PMT(void);
void PMT_Parser(Sec_Filter * sec_filter);
void NIT_Parser(Sec_Filter * sec_filter);
void SDT_Parser(Sec_Filter * sec_filter);
void BAT_Parser(Sec_Filter * sec_filter);
void EIT_Parser(Sec_Filter * sec_filter);
void TDT_TOT_Parser(Sec_Filter *sec_filter);
void ConvertMJD(UINT16 MJD);

