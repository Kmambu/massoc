
/////////////////////////////////////////////////////////////////////
// File: soclib_vci_multi_tty.h 
// Author :  Alain Greiner & Frédéric Pétrot & Georges Adouko
// Date : 12/08/2004
// This program is released under the GNU Public License
// Copyright : UPMC-LIP6
//
// This component is a VCI ADVANCED character display.
// Il is a read-only VCI target.
// 
// The VCI DATA and ADDRESS fields must be 32 bits.
// The VCI ERROR field must be 1 bit.
// The VCI PLEN, CLEN, CONS, CONTIG, WRAP fields are not used.
//
// Each 32 bits VCI data word contains a single byte to be  displayed.
// 
// This component cheks the segmentation violations, and can be used
// as a default target.
//
// The VCI response packet is stored in a two slots output FIFO.
//
// This component can be used in a 2 level hierarchical interconnected system
// That is why it has got 2 VCI index : 1 for the upper level , 1 for the lower
//
/////////////////////////////////////////////////////////////////////////
// This component has 3 "constructor" parameters :
// - char			*name   : instance name  
// - int			index   : VCI target index
// - SOCLIB_SEGMENT_TABLE	*segtab : pointer on the segment table
//
// User can set SOCLIB_VCI_MULTI_TTY environment variable to :
// - "DISABLE_GRAPHIC" to disable the graphical display.
//
/////////////////////////////////////////////////////////////////////////
// This component has 8  "template" parameters :
// - int	NTTY		: number of teminals TTY, max 16 TTY
// - int	ADDRSIZE	: number of bits of the VCI ADDRESS field
// - int	CELLSIZE	: number of bytes of the VCI DATA field
// - int	ERRSIZE		: number of bits of the VCI ERROR field
// - int	PLENSIZE	: number of bits of the VCI PLEN field
// - int	CLENSIZE	: number of bits of the VCI CLEN field
// - int	SRCIDSIZE	: number of bits of the VCI SRCID field
// - int	TRDIDSIZE	: number of bits of the VCI TRDID field
// - int	PKTIDSIZE	: number of bits of the VCI PKTID field
//////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
//
// - Tree importants addresses for each TTY
// For the TTY 0
// 	00	: write address, data to the screen
//	04      : read address, the status of the keyboard of TTY 0
//	08      : read address, the caracter read from the keyboard of TTY 0
// For the TTY 1
// 	10	: write address, data to the screen
//	14      : read address, the status of the keyboard of TTY 1
//	18      : read address, the caracter read from the keyboard of TTY 1
// ...
/////////////////////////////////////////////////////////////////////






#ifndef SOCLIB_VCI_MULTI_TTY_H
#define SOCLIB_VCI_MULTI_TTY_H

#include <fcntl.h>
#include <stdarg.h>
#include <stdlib.h>

#include <systemc.h>
#include "shared/soclib_vci_interfaces.h"
#include "shared/soclib_mapping_table.h"
#include "shared/soclib_generic_fifo.h"

#define FIFO_MAX 10



///////////////////////////////////////
//         Structure de donnée
///////////////////////////////////////

typedef struct  canal {	int CanalPipe[2]; } canal ;




////////////////////////////////////////
//	structure definition
////////////////////////////////////////

template<
int NTTY,
    VCI_PARAM_DECLAR>

    struct SOCLIB_VCI_MULTI_TTY : sc_module {

	    //	IO PORTS

	    sc_in<bool> 			CLK;
	    sc_in<bool> 			RESETN;
	    sc_out<bool> 			IRQ[NTTY] ;

	    ADVANCED_VCI_TARGET<VCI_PARAM> VCI;

	    //	REGISTERS

	    sc_signal<bool>			REG_IRQ[NTTY];
	    sc_signal<int>			FIFO_STATE;
	    sc_signal<int>			FIFO_RDATA[2];
	    sc_signal<int>			FIFO_RERROR[2];
	    sc_signal<int>			FIFO_REOP[2];
	    sc_signal<int>			FIFO_RSRCID[2];
	    sc_signal<int>			FIFO_RPKTID[2];
	    sc_signal<int>			FIFO_RTRDID[2];
	    soclib_generic_fifo<FIFO_MAX, 8>	FIFO_CLAVIER[NTTY];

	    //	STRUTURAL PARAMETERS

	    const char 			*NAME , *segNAME;	// instance name
	    int				BASE;			// Base address
	    int				SIZE;			// Segment size
	    FILE				*TTY[NTTY];		// log file
	    pid_t				PID[NTTY];		// process ID for XTTY child
	    int				POUT[NTTY];		// pipe output buffer
	    int				PIN[NTTY];		// pipe input  buffer
	    char 				readed_character[NTTY];

	    ///////////////////////////////////////////////////
	    //	constructor
	    ///////////////////////////////////////////////////

	    SC_HAS_PROCESS(SOCLIB_VCI_MULTI_TTY);

	    SOCLIB_VCI_MULTI_TTY (
			    sc_module_name insname, // instance name
			    unsigned int index,     // VCI target index
			    SOCLIB_MAPPING_TABLE segtab, // segment table pointer
			    char * nomXtty,
			    ...
			    ) : CLK("CLK"),
				RESETN("RESETN"),
				FIFO_STATE("FIFO_STATE")
	    {
#ifdef NONAME_RENAME
		    char newname[100];
		    for (int i=0; i<NTTY; i++ ) {
			    sprintf(newname, "REG_IRQ_%2.2d", i);
			    REG_IRQ[i].rename(newname);
			    sprintf(newname, "IRQ_%2.2d", i);
			    IRQ[i].rename(newname);
			    sprintf(newname, "FIFO_CLAVIER_%2.2d", i);
			    FIFO_CLAVIER[i].rename(newname);
		    }
		    for (int i=0; i<2; i++ ) {
			    sprintf(newname, "FIFO_RDATA_%2.2d", i);
			    FIFO_RDATA[i].rename(newname);
			    sprintf(newname, "FIFO_RERROR_%2.2d", i);
			    FIFO_RERROR[i].rename(newname);
			    sprintf(newname, "FIFO_REOP_%2.2d", i);
			    FIFO_REOP[i].rename(newname);
			    sprintf(newname, "FIFO_RSRCID_%2.2d", i);
			    FIFO_RSRCID[i].rename(newname);
			    sprintf(newname, "FIFO_RTRDID_%2.2d", i);
			    FIFO_RTRDID[i].rename(newname);
			    sprintf(newname, "FIFO_RPKTID_%2.2d", i);
			    FIFO_RPKTID[i].rename(newname);
		    }
#endif

		    canal	pin[NTTY];		// inputs pipes for the simulator process
		    canal	pout[NTTY];		// outputs pipes for the simulator process

		    va_list listeNomXtty ;		// contient le reste des noms de fenetres xtty
		    char * tempNomXtty ;		// variable de gestion des noms de xtty

		    int i;

		    SC_METHOD (transition);
		    sensitive_pos << CLK;

		    SC_METHOD (genMoore);
		    sensitive_neg << CLK;

		    NAME = (const char*) insname;

		    // get the BASE address and segment SIZE 

		    std::list<SEGMENT_TABLE_ENTRY> seglist = segtab.getSegmentList(index);
		    BASE = (*seglist.begin()).getBase(); 
		    SIZE = (*seglist.begin()).getSize(); 
		    segNAME = (*seglist.begin()).getName();

		    // checking the VCI parameters


		    if (NTTY > 16) {
			    fprintf (stderr," Error in TTY component : the number of TTY is too high ! \n");
			    fprintf (stderr," The number of TTY must be lower or equal to 16\n");
			    sc_stop(); 
		    }


		    if (ADDRSIZE != 32) {
			    fprintf (stderr," Error in TTY component : the ADDRSIZE parameter\n");
			    fprintf (stderr," must be equal to 32\n");
			    sc_stop(); 
		    }

		    if (CELLSIZE != 4) {
			    fprintf (stderr," Error in TTY component : the CELLSIZE parameter\n");
			    fprintf (stderr," must be equal to 4\n");
			    sc_stop(); 
		    }

		    if (ERRSIZE != 1) {
			    fprintf (stderr," Error in TTY component : the ERRSIZE parameter\n");
			    fprintf (stderr," must be equal to 1\n");
			    sc_stop(); 
		    }

		    if (SRCIDSIZE > 16) {
			    fprintf (stderr," Error in TTY component : the SRCIDSIZE parameter\n");
			    fprintf (stderr," cannot be larger than 16\n");
			    sc_stop(); 
		    }

		    if (TRDIDSIZE > 32) {
			    fprintf (stderr," Error in TTY component : the TRDIDSIZE parameter\n");
			    fprintf (stderr," cannot be larger than 32\n");
			    sc_stop(); 
		    }

		    if (PKTIDSIZE > 32) {
			    fprintf (stderr," Error in TTY component : the PKTIDSIZE parameter\n");
			    fprintf (stderr," cannot be larger than 32\n");
			    sc_stop(); 
		    }


		    // Procedure d'ouverture de tous les XTTY
		    va_start (listeNomXtty, nomXtty);

		    tempNomXtty = nomXtty ;

		    for (i=0; i<NTTY; i++)
		    {
			    // open the TTY file

			    if ((TTY[i] = fopen(tempNomXtty, "w")) == NULL) {
				    printf ("Some TTY names may be missing as constructor parameters.\n");
				    printf(" Error in TTY component : the \"%s\" TTY file cannot be open !\n", tempNomXtty);
				    exit(1); 
			    }

			    // XTTY and pipe initialisation

			    if (pipe((pout[i]).CanalPipe) != 0) {
				    printf(" Error in TTY component : the output pipe to the \"%s\" XTTY terminal cannot be open !\n", tempNomXtty);
				    exit(1); 
			    }


			    if (pipe((pin[i]).CanalPipe) != 0) {
				    printf(" Error in TTY component : the input pipe from the \"%s\" XTTY terminal cannot be open !\n", tempNomXtty);
				    exit(1); 
			    }

			    const char *graphic_env = getenv ("SOCLIB_VCI_MULTI_TTY");
			    bool graphic_enabled = (graphic_env == NULL)
				    || (strstr (graphic_env,"DISABLE_GRAPHIC") == NULL);
			    if (!graphic_enabled) {
				    fprintf(stderr,"X11 display disabled.\n");
			    } else if ((PID[i] = fork()) == 0) { // we are in the child process -> XTTY
				    close(0);
				    //dup(pout[0]);
				    dup(  ((pout[i]).CanalPipe) [0] );

				    close(1);
				    //dup (pin[1]);
				    dup(  ((pin[i]).CanalPipe) [1] );

				    execlp("xtty","xtty", tempNomXtty ,NULL);
				    printf(" Error in TTY component : the \"%s\" XTTY terminal cannot be open !\n",tempNomXtty );
				    printf(" Error in TTY component : the \"%s\" XTTY terminal cannot be open !\n",tempNomXtty );
				    exit(1);
			    }
			    // we are in the parent process -> SYSTEMC SIMULATOR
			    PIN[i]  = ((pin[i]).CanalPipe) [0];
			    fcntl (PIN[i], F_SETFL, O_NONBLOCK);
			    POUT[i] = ((pout[i]).CanalPipe)[1];

			    tempNomXtty = va_arg (listeNomXtty, char *);

		    } // fin de creation de tous les xtty

		    va_end (listeNomXtty);

		    printf("Successful Instanciation of SOCLIB_VCI_MULTI_TTY : %s\n", NAME);
		    printf ("segmentName : %s \n" ,segNAME);



	    }; // end constructor

	    ///////////////////////////////////////////////////////
	    //	destructor()
	    //////////////////////////////////////////////////////

	    ~SOCLIB_VCI_MULTI_TTY()
	    {
#if 1
		    fprintf(stderr,"Closing %s.\n", NAME);
#endif
		    int i;
		    for (i=0; i<NTTY; i++)
		    {
			    fclose (TTY[i]); // STOP TO REMOVE FCLOSE ! Unclosed files are empty files.
			    if (PID[i])
				    kill(PID[i],SIGTERM);
		    }
	    }

	    ///////////////////////////////////////////////////////
	    //	Transition method()
	    //////////////////////////////////////////////////////

	    void transition()
	    {

		    int	address;
		    int	cmd;
		    int	rerror;
		    char	wdata;
		    char 	rdata;
		    bool	FIFO_CLAVIER_READ [NTTY];
		    bool	FIFO_CLAVIER_WRITE[NTTY];
		    int 	i;
		    int localTerminal, numTerminal;

		    if(RESETN == false) {


			    FIFO_STATE = 0;
			    for (i=0; i<NTTY; i++)
				    FIFO_CLAVIER[i].init();
			    return;
		    } // end RESETN

		    for (i=0; i<NTTY; i++)
		    {
			    FIFO_CLAVIER_READ[i] = false; //Don't read any data from the fifo
			    FIFO_CLAVIER_WRITE[i] = false; //Don' write any data to the fifo

			    if( FIFO_CLAVIER[i].filled_status() < FIFO_MAX)
			    {
				    char test;
				    test = read (PIN[i], &readed_character[i], 1);
				    if (test != -1) {
					    FIFO_CLAVIER_WRITE[i] = true;
#if 0
					    std::cerr << "Typed key : #" << caractere_lu[i] << std::endl;
#endif
				    }
			    }
		    }

		    switch (FIFO_STATE) {

			    case 0 : 	//  FIFO empty

				    if (VCI.CMDVAL == true) { 
					    address = (int)VCI.ADDRESS.read();
					    cmd     = (int)VCI.CMD.read();
					    wdata   = (char)(VCI.WDATA.read()  & 0x000000FF);
					    localTerminal = (address-BASE) & 0xf ;
					    numTerminal = (address-BASE)/16 & 0xf ;

					    if ((address < BASE) || 
							    (address >= (BASE + SIZE)))
					    {
						    rerror = 1;
					    } else {
						    if (cmd ==  VCI_CMD_WRITE) 
						    {		

							    rerror = 0;
							    rdata = 0;
							    if (localTerminal == 0 && numTerminal < NTTY)   // si l'adresse se termine par 0 et que on ecrit un bon numero de terminal
							    {
								    write(POUT[numTerminal], &wdata, 1);
								    fputc(wdata, TTY[numTerminal]);
							    }


						    } else {

							    if ( (localTerminal == 4) && numTerminal < NTTY)
							    {
								    rerror = 0;
								    rdata = FIFO_CLAVIER[numTerminal].filled_status();
							    } else {
								    if ( (localTerminal == 8) && numTerminal < NTTY)
								    {
									    rerror = 0;
									    rdata = (char)FIFO_CLAVIER[numTerminal].read();
									    FIFO_CLAVIER_READ[numTerminal] = true;	
								    } else {
									    rerror = 0;
									    rdata = 0;
								    }
							    }

						    }
					    }
					    FIFO_RERROR[0] = rerror;
					    FIFO_RDATA[0]  = (int)rdata ;
					    FIFO_RSRCID[0] = (int)VCI.SRCID.read(); 
					    FIFO_RPKTID[0] = (int)VCI.PKTID.read(); 
					    FIFO_RTRDID[0] = (int)VCI.TRDID.read(); 
					    FIFO_REOP[0]   = (bool)VCI.EOP.read();
					    FIFO_STATE     = 1;
				    } 
				    break;

			    case 1 :	//  FIFO containing one response in slot 0

				    if (VCI.CMDVAL == true) {

					    address = (int)VCI.ADDRESS.read();
					    cmd     = (int)VCI.CMD.read();
					    wdata   = (char)(VCI.WDATA.read()  & 0x000000FF);
					    localTerminal = (address-BASE) & 0xf ;
					    numTerminal = (address-BASE)/16 & 0xf ;

					    if ((address < BASE) || 
							    (address >= (BASE + SIZE)))
					    {
						    rerror = 1;
					    } else {
						    if (cmd ==  VCI_CMD_WRITE) 
						    {	

							    rerror = 0;
							    rdata = 0;
							    if (localTerminal == 0 && numTerminal < NTTY)   // si l'adresse se termine par 0 et que on ecrit un bon numero de terminal
							    {
								    write(POUT[numTerminal], &wdata, 1);
								    fputc(wdata, TTY[numTerminal]);
							    }

						    } else {

							    if ( (localTerminal == 4) && numTerminal < NTTY)
							    {
								    rerror = 0;
								    rdata = FIFO_CLAVIER[numTerminal].filled_status();
							    } else {
								    if ( (localTerminal == 8) && numTerminal < NTTY)
								    {
									    rerror = 0;
									    rdata = (char)FIFO_CLAVIER[numTerminal].read();
									    FIFO_CLAVIER_READ[numTerminal] = true;	
								    } else {
									    rerror = 0;
									    rdata = 0;
								    }
							    }

						    }
					    }


					    if (VCI.RSPACK == true) { 
						    FIFO_RDATA[0]  = (int)rdata ;
						    FIFO_RERROR[0] = rerror;
						    FIFO_RSRCID[0] = (int)VCI.SRCID.read(); 
						    FIFO_RPKTID[0] = (int)VCI.PKTID.read(); 
						    FIFO_RTRDID[0] = (int)VCI.TRDID.read(); 
						    FIFO_REOP[0]   = (bool)VCI.EOP.read();
						    FIFO_STATE     = 1;
					    } else {
						    FIFO_RDATA[1]  = (int)rdata ;
						    FIFO_RERROR[1] = rerror;
						    FIFO_RSRCID[1] = (int)VCI.SRCID.read(); 
						    FIFO_RPKTID[1] = (int)VCI.PKTID.read(); 
						    FIFO_RTRDID[1] = (int)VCI.TRDID.read(); 
						    FIFO_REOP[1]   = (bool)VCI.EOP.read();
						    FIFO_STATE     = 2;
					    }
				    } else {  // CMDVAL == false
					    if (VCI.RSPACK == true) { 
						    FIFO_STATE      = 0;
					    } else {
						    FIFO_STATE	= 1;
					    }
				    }
				    break;

					    case 2 :	// FIFO Full


				    if (VCI.RSPACK == true) {
					    FIFO_RDATA [0] = FIFO_RDATA[1];	
					    FIFO_RERROR[0] = FIFO_RERROR[1];
					    FIFO_RSRCID[0] = FIFO_RSRCID[1]; 
					    FIFO_RPKTID[0] = FIFO_RPKTID[1]; 
					    FIFO_RTRDID[0] = FIFO_RTRDID[1]; 
					    FIFO_REOP[0]   = FIFO_REOP[1];
					    FIFO_STATE     = 1;
				    } else {
					    FIFO_STATE	= 2;
				    }
				    break;

				    } // end switch FIFO_STATE

				    for (i=0; i<NTTY; i++)
				    {
					    if(( FIFO_CLAVIER_READ[i] == true ) && ( FIFO_CLAVIER_WRITE[i] == false)) FIFO_CLAVIER[i].simple_get();
					    if(( FIFO_CLAVIER_READ[i] == true ) && ( FIFO_CLAVIER_WRITE[i] == true )) FIFO_CLAVIER[i].put_and_get(readed_character[i]);
					    if(( FIFO_CLAVIER_READ[i] == false) && ( FIFO_CLAVIER_WRITE[i] == true )) FIFO_CLAVIER[i].simple_put(readed_character[i]);
				    }


		    }; // end transition()

		    /////////////////////////////////////////////////////
		    //	genMoore() method
		    /////////////////////////////////////////////////////

		    void genMoore()
		    {

			    int i;

			    switch(FIFO_STATE) {

				    case 0 :
					    VCI.CMDACK = true;
					    VCI.RSPVAL = false;
					    VCI.RDATA  = 0;
					    VCI.RERROR = 0;
					    VCI.RSRCID = 0;
					    VCI.RPKTID = 0;
					    VCI.RTRDID = 0;
					    VCI.REOP   = false;
					    break;

				    case 1 :
					    VCI.CMDACK = true;
					    VCI.RSPVAL = true;
					    VCI.RDATA  = (sc_uint<8*CELLSIZE>) FIFO_RDATA[0];
					    VCI.RERROR = (sc_uint<ERRSIZE>) FIFO_RERROR[0];
					    VCI.RSRCID = (sc_uint<SRCIDSIZE>) FIFO_RSRCID[0];
					    VCI.RTRDID = (sc_uint<TRDIDSIZE>) FIFO_RTRDID[0];
					    VCI.RPKTID = (sc_uint<PKTIDSIZE>) FIFO_RPKTID[0];
					    VCI.REOP   = (bool) FIFO_REOP[0];
					    break;

				    case 2 :
					    VCI.CMDACK = false;
					    VCI.RSPVAL = true;
					    VCI.RDATA  = (sc_uint<8*CELLSIZE>) FIFO_RDATA[0];
					    VCI.RERROR = (sc_uint<ERRSIZE>) FIFO_RERROR[0];
					    VCI.RSRCID = (sc_uint<SRCIDSIZE>) FIFO_RSRCID[0];
					    VCI.RTRDID = (sc_uint<TRDIDSIZE>) FIFO_RTRDID[0];
					    VCI.RPKTID = (sc_uint<PKTIDSIZE>) FIFO_RPKTID[0];
					    VCI.REOP   = (bool) FIFO_REOP[0];
					    break;

			    } // end switch FIFO_STATE



			    for (i=0; i<NTTY; i++)
				    IRQ[i] = (FIFO_CLAVIER[i].filled_status()>0) ? true : false;


		    }; // end genMoore()




	    }; // end structure SOCLIB_VCI_MULTI_TTY

#endif
