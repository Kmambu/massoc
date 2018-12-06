/////////////////////////////////////////////////////////////////
//	text, reset, and exception segments
/////////////////////////////////////////////////////////////////

#define	TEXT_BASE	0x00400000
#define	TEXT_SIZE	0x00050000

/* base address required by MIPS processor */
#define	RESET_BASE	0xBFC00000
#define	RESET_SIZE	0x00010000

#define	EXCEP_BASE	0x80000000
#define	EXCEP_SIZE	0x00010000

#define	DATA_BASE	0x10000000
#define	DATA_SIZE	0x00020000

//////////////////////////////////////////////////////////
//	System devices
///////////////////////////////////////////////////////////

#define	TTY_BASE	0xC0200000
#define	TTY_SIZE	0x00000040

#define	IO_BASE		0xE0200000
#define	IO_SIZE		0x00000040
