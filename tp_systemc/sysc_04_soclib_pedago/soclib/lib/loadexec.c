/*
 * File       : loadexec.c
 * Author     : Frédéric Pétrot
 *
 * Load an executable into an array using the GNU BFD
 *
 * $Log: loadexec.c,v $
 * Revision 1.3  2006/06/08 14:32:41  nipo
 * Use _loading_ memory address rather than _virtual_ memory address when
 * loading objects.
 *
 * Patch from Alexandre Becoulet
 *
 * Revision 1.2  2006/02/21 10:37:40  buchmann
 * Changes :
 * - random optimizations
 *   - now uses conditional expressions instead of if statements
 *   - disables some useless assignments
 *   - now uses shift operators instead of arithmetic operators
 * - fix documentations
 *
 * Revision 1.1.1.1  2005/01/27 13:42:45  wahid
 * First project import
 * Wahid
 *
 * Revision 1.1  2003/03/10 10:38:05  fred
 * Adding the bfd loader for good.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <bfd.h>

#ifndef TRUE
#define TRUE true
#endif

#ifndef FALSE
#define FALSE false
#endif

/* Loading the sections sections of file file into the array 
 * pointed to by emem.
 * esize is the expected size, but it can be modified by the function if it too 
 * small for the sections
 * All sizes and addresses are given in bytes
 * eoffset is the address of the section with the lowest address among
 * all required sections.
 * Code should be self explanatory once you've read the 230 pages of
 * the BFD documentation :) */

typedef struct raminfo {
  void *mem;
  unsigned int  size;
  unsigned int  ladr;
  unsigned int  hadr;
  const char *file;
  const char **sections;
} raminfo;

static void bindsection(bfd *exec, asection *sect, PTR x)
{
   int i;
   raminfo *rinfo = x; 

   for (i = 0; rinfo->sections[i]; i++)
      if (!strcmp(sect->name, rinfo->sections[i]))
         break;
   if (!rinfo->sections[i])
      return;

   if ((sect->flags & SEC_LOAD) && !(sect->flags & SEC_IN_MEMORY)) {
      bfd_get_section_contents(exec, sect,
                               rinfo->mem + (sect->lma - rinfo->ladr),
                               0, bfd_section_size(exec, sect));
      sect->contents = rinfo->mem;
   }
}

static void sectionssize(bfd *exec, asection *sect, PTR x)
{
   int i;
   raminfo *rinfo = x; 

   for (i = 0; rinfo->sections[i]; i++)
      if (!strcmp(sect->name, rinfo->sections[i]))
         break;
   if (!rinfo->sections[i])
      return;

   rinfo->size += bfd_section_size(exec, sect);
   rinfo->ladr  = sect->lma < rinfo->ladr ? sect->lma : rinfo->ladr;
   rinfo->hadr  = sect->lma + rinfo->size > rinfo->hadr ?
                  sect->lma + rinfo->size : rinfo->hadr;
}

void loadexec(void **emem, int *esize, int *eoffset, const char *file, const char **sections)
{
  bfd      *exec;
  static   int x;
  int      i;
  raminfo  ringo;

   if (!x)
      bfd_init();

   exec = bfd_openr(file, NULL);

   if (!exec) {
      fprintf(stderr, "Cannot open File '%s'\n", file);
      exit(1);
   }

   if (bfd_check_format(exec, bfd_object) != TRUE && !(exec->flags & EXEC_P)) {
      fprintf(stderr, "File %s is not an executable file\n",
                       file); //bfd_get_filename(exec));
      exit(1);
   }

#if 1
   printf("Loading sections ");
   for (i = 0; sections[i]; i++)
      printf("%s%s", sections[i], sections[i+1] ? ", " : " ");
   printf("from \"%s\"\n",bfd_get_filename(exec));
   //printf("of executable '%s' for '%s' architecture in format '%s'\n",
   //        bfd_get_filename(exec), bfd_printable_name(exec), exec->xvec->name);
#endif

   /* Set input parameters to bindsection */
   ringo.file     = file;
   ringo.sections = sections;
   ringo.size     = 0;
   ringo.ladr     = ~0;
   ringo.hadr     = 0;
   bfd_map_over_sections(exec, sectionssize, &ringo);
   /* Get output parameters from sectionssize */
   if (ringo.size < ringo.hadr - ringo.ladr)
      ringo.size = ringo.hadr - ringo.ladr;
   *esize         = ringo.size > *esize ? ringo.size : *esize;
   *eoffset       = ringo.ladr;
   ringo.mem      = malloc(*esize * sizeof(char));
   /* Start over again from the start of the memory */
   bfd_map_over_sections(exec, bindsection, &ringo);
   /* Get output parameters from bindsection */

   *emem          = ringo.mem;

   if (bfd_close(exec) == FALSE) {
      bfd_perror(exec->filename);
      exit(1);
   }
}
