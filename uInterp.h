/*
 * Name: 
 * uInterp
 *
 * Description:
 * uInterp pronounced Micro Interpreter.  This interpreter is for 
 * small systems to allow interactive use to a computer program or system.
 *
 * Revision 	Who 		History
 * 01/30/13 	SFK 		Genesis
 *
 * Example usage:
 *
 * // Declare a command table
 * const uInterpCmd_T gExtCmds [] = 
 * {
 * {Echo,	"echo",  "echo arguments", "."},
 * };
 *
 * void StartInterpreter(void)
 * {
 *     char Line[MAXLINE];
 *
 *     // Construct the interpreter 	
 *	   uInterpCtor(gExtCmds, sizeof(gExtCmds)/sizeof(*gExtCmds));
 *
 *     // Start accepting lines of text 
 *	   for ( ; ; )
 *	   {
 *	       // Read a line of text from the device / console 
 *		   if (fgets(Line, MAXLINE, stdin))
 *		   {	
 *             // Give the line to the interpreter to execute
 *			   uInterpExecute(Line);
 *         }
 *     }	
 *
 *     // Destroy the interpreter 
 *     uInterpDtor();
 * }	
 *
 * // Echo command arguments
 * int Echo(void)
 * {
 *      int Count;
 *      int i;
 *      char Buff[80];
 *   
 *      Count = uInterpNumArgs();
 *      printf("Arguments %d\n", Count);
 *   
 *      for (i = 0; i < Count; i = i + 1)
 *      {
 *          uInterpArgs(i, UINTERP_STR, Buff, sizeof(Buff));
 *          printf("Arg: %d Str: %s\n", i, Buff);
 *      }
 *	
 *      return 0;
 * }   
 */
#ifndef UINTERP_H
#define UINTERP_H

/* Defines used to parse arguments into simple data types */
#define UINTERP_UINT8       0
#define UINTERP_UINT16      1
#define UINTERP_UINT32      2
#define UINTERP_STR         3
#define UINTERP_RAW         4    

#define UINTERP_OK          0
#define UINTERP_EFAIL       -1
#define UINTERP_EARG        -2
#define UINTERP_ERANGE      -3
#define UINTERP_ECOMMAND    -4

/* Command table data structure */
typedef struct 
{
    int (*pfCmd)(void);
    char 			        *pName;
    char			        *pDescription;
    char			        *pUsage;
} uInterpCmd_T; 

/* Initialize the interpreter */
int
uInterpCtor(
    const uInterpCmd_T	    *pExtCmds,
    unsigned int            uCmdCount
    );
    
/* Relase any resourcs alocated by the interpreter */
void 
uInterpDtor(
    void
    );

/* Lookup and excute the command on line completion */
int
uInterpExecute(
    char 	                *pLine
    );

/* Return the number of arguments, command name is always here */
int 
uInterpNumArgs(
    void
    );

/* Parse the arguments to the interpreter command */
int
uInterpArgs(
    unsigned int            uArgNum,
    unsigned int            eArgType,
    void                    *pBuff,
    unsigned int            uLen
    );

/* Convert a string to binary buffer */
int
uInterpStrToBin(
    const char              *pStr,
    unsigned char           *pBuf,
    int                     Len,
    int                     *pLen
    );

/* Returns S_OK match, S_FALSE no match, E_XXX on hard error */
int 
uInterpMatchParam(
    unsigned int            uArgNum,
    const char              *pMatch
    );
    

#endif


