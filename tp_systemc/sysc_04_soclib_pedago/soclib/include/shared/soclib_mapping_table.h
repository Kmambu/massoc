// -*- c++ -*-
/*
 * File : soclib_mapping_table.h
 * Date : 30/08/2005
 * authors : Francois Pecheux & Alain Greiner
 *
 * Copyright : UPMC - LIP6
 * This program is released under the GNU general public license
 *
 * Jan-2006, Nicolas Pouillon, <nipo@ssji.net>
 *   Introducing arbitraty number of levels in hierarchy
 *   - intList
 *
 *
 * This object is used to describe the memory mapping and address
 * decoding scheme of an hardware architecture build with the SoCLib
 * library.  All VCI initiators and VCI targets share the same address
 * space.
 *
 *  *** Routing for dummies ***
 *
 * Routing involves MSBs of addresses, the more global the
 * interconnect is, the more significant the bits are matched.
 *
 * examples:
 *
 * - 1 level of interconnect, matching 4 bits for routing, 16
     different targets possible.
 *
 * - 2 levels of routing, matching 4 bits for first level of
     interconnection, 3 for second level.  On first level, there will
     be 16 clusters max, each with 8 targets max.
 *
 * All the mapping process is to determine which ports on
 * interconnects addresses will be sent to. This process must be kept
 * simple stupid so:
 * - It cant involve full address decoding
   - It cant involve matching on bits from hier levels of hierarchy
 *
 *  *** Multiple addresses on one target ***
 *
 * A target or a routing level may be reachable through more than one
 * address, this restricts other uses for same addresses. It even
 * restricts repartition of addresses at lower levels of routing (if
 * any):
 *
 * example: 2 level hierarchy, matching 4 bits on first level, 4 bits
 * on second.
 *
 * Segment list:
 * name base       length     target
 * seg1 0x24000000 0x01000000 1, 1
 * seg2 0x34000000 0x01000000 2, 3
 * seg3 0x67000000 0x01000000 1, 4
 * seg4 0x27000000 0x01000000 1, 4*
 *        ||                  |  \  index in cluster
 *        ||                  \  Cluster
 *        |\ Matched for 2nd level routing
 *        \  Matched for first level routing
 *
 * This assigns addresses starting with 2 and 6 to cluster 1,
 * addresses starting with 3 to cluster 2.
 * In cluster 1, routing is done in two steps:
 * - Match "hier" levels bits (here, 4 MSBs), and determine if they
     are local (if this decodes to the local cluster)
   - If true, decode local bits (from current level)
 *
 * This explains the fact seg4 can only be assigned to target 1, 4:
 * local address field 7 was assigned to target 4 when registered
 * seg1, event if MSBs are different, we cant make any difference
 * between 0x67... and 0x27... on second level routing, qo they MUST
 * be the same target.
 *
 * After routing part of address, Soclib uses two more bits of
 * addresses to force typing of memory, in order to control
 * cachability of addresses.
 *
 *  *** intList ***
 *
 * In order to ba able to designate arbitrary levels of hierarchy,
 * there is a macro intList(x...). This creates a table usable by the
 * mapping table.
 *
 *  *** Sample ***
 *
 * Let's define some segments on a 1-level hierarchy:
 *
 * SOCLIB_MAPPING_TABLE map_tbl(32, 1, intList(1), intList(1), 0x3<<29);
 *
 * Creates a mapping table for 32-bits address fields, 1 level of
 * routing, 1 bit of address used for routing, 1 bit of index for vci
 * ids, bits 30-29 used for caching type
 *
 * map_tbl.addSegment(    "cram", 0xa0000000, 0x00001410, map_tbl.ident(intList(0)), SEGMENT_TYPE_CACHED);
 * map_tbl.addSegment(    "uram", 0xc0000000, 0x000001a0, map_tbl.ident(intList(0)), SEGMENT_TYPE_UNCACHED);
 * map_tbl.addSegment(   "reset", 0xbfc00000, 0x00000188, map_tbl.ident(intList(0)), SEGMENT_TYPE_CACHED);
 * map_tbl.addSegment(   "excep", 0x80000080, 0x00000174, map_tbl.ident(intList(0)), SEGMENT_TYPE_CACHED);
 * map_tbl.addSegment(     "tty", 0x40000000, 0x20000000, map_tbl.ident(intList(1)), SEGMENT_TYPE_UNCACHED);
 *
 * This mapping table maps segment tty (MSB=0) on target 1, all others (MSB=1) on target 0.
 *
 * Now let's define a 2-levels hierarchy
 *
 * SOCLIB_MAPPING_TABLE map_tbl(32, 2, intList(2, 2), intList(3, 2), 0x3<<20);
 *
 * 32 addresses, 2 levels, 2 bits for address routing on each level, 3
 * bits for vci source ids on first level, 2 on second.
 * Using bits 21 and 20 to contain address type field.
 *
 *                                  /- cluster, target 2x2 bits        /- cluster
 *                                  | / address type                   | / target
 * map_tbl.addSegment(   "cram0", 0xb0100000, 0x00100000, map_tbl.ident(intList(0,0)), SEGMENT_TYPE_CACHED);
 * map_tbl.addSegment(   "uram0", 0xb0200000, 0x00100000, map_tbl.ident(intList(0,0)), SEGMENT_TYPE_UNCACHED);
 * map_tbl.addSegment(   "reset", 0xbfc00000, 0x00100000, map_tbl.ident(intList(0,0)), SEGMENT_TYPE_CACHED);
 * map_tbl.addSegment(   "excep", 0x80000080, 0x000fff80, map_tbl.ident(intList(0,0)), SEGMENT_TYPE_CACHED);
 * map_tbl.addSegment(    "sem0", 0x90200000, 0x00100000, map_tbl.ident(intList(0,1)), SEGMENT_TYPE_UNCACHED);
 * map_tbl.addSegment(     "tty", 0xa0200000, 0x00100000, map_tbl.ident(intList(0,2)), SEGMENT_TYPE_UNCACHED);
 * map_tbl.addSegment(   "cram1", 0x00100000, 0x00100000, map_tbl.ident(intList(1,0)), SEGMENT_TYPE_CACHED);
 * map_tbl.addSegment(   "uram1", 0x00200000, 0x00100000, map_tbl.ident(intList(1,0)), SEGMENT_TYPE_UNCACHED);
 * map_tbl.addSegment(    "sem1", 0x10200000, 0x00100000, map_tbl.ident(intList(1,1)), SEGMENT_TYPE_UNCACHED);
 * map_tbl.addSegment(   "cram2", 0x40100000, 0x00100000, map_tbl.ident(intList(2,0)), SEGMENT_TYPE_CACHED);
 * map_tbl.addSegment(   "uram2", 0x40200000, 0x00100000, map_tbl.ident(intList(2,0)), SEGMENT_TYPE_UNCACHED);
 * map_tbl.addSegment(    "sem2", 0x50200000, 0x00100000, map_tbl.ident(intList(2,1)), SEGMENT_TYPE_UNCACHED);
 * map_tbl.addSegment(   "cram3", 0xc0100000, 0x00100000, map_tbl.ident(intList(3,0)), SEGMENT_TYPE_CACHED);
 * map_tbl.addSegment(   "uram3", 0xc0200000, 0x00100000, map_tbl.ident(intList(3,0)), SEGMENT_TYPE_UNCACHED);
 * map_tbl.addSegment(    "sem3", 0xd0200000, 0x00100000, map_tbl.ident(intList(3,1)), SEGMENT_TYPE_UNCACHED);
 *
 *  *** Segments ***
 *
 * The segment table is an associative table that is organized as a
 * list of segments.  Each segment descriptor contains the following
 * fields:
 *
 * - const char    *name   : segment name
 * - unsigned int  base    : base address
 * - unsigned int  size    : segment size (in bytes)
 * - unsigned int  *index  : VCI target index 
 * - unsigned int  type    : UNCHECKED / CACHED / UNCACHED / PREFETCH 
 *
 * The constraints on the memory mapping are the following :
 * 
 * 1/ The system designer must explicitely define and map all logical
 * segments to a given VCI target, using the addMapping() method.
 *
 * 2/ A segment must be contained in a single page or in several
 * contiguous pages.
 * 
 * 3/ Several segments can be mapped to the same VCI target (thanks to
 * the SOCLIB_VCI_MULTIRAM or SOCLIB_MULTI_TIMER components).  A given
 * page cannot contain two segments that are not mapped to the same
 * VCI target.
 *
 * 4/ The requests with an "illegal" address (address that are outside
 * all defined sgments) will be routed to the default target of the
 * addressed cluster.  There is one default target per cluster, that
 * is the VCI target with the local index 0.  A default target can be
 * any VCI target that decode all address bits (in order to detect
 * erroneous addresses) The default cluster is the cluster with the
 * smallest cluster index.
 * 
 * 5/ Each segment must have a type, defining a specific cache
 * behaviour for this segment: The SOCLIB_XCACHE decodes two bits of
 * the VCI address, defined by the SEGTYPEMASK parameter to obtain the
 * segment type:
 * - if the value is 0*, the address a will be cached. 
 * - if the value is 10, the address a will be uncached. 
 * - if the value is 11, the address a will be prefetched.
 *
 * The system designer must define a segment type, that can be checked
 * versus the actual segment mapping in the memory space.
 * - SEGMENT_TYPE_UNCHECKED  : Mapping type is not checked
 * - SEGMENT_TYPE_CACHED     : the segment will be cached 
 * - SEGMENT_TYPE_UNCACHED   : the segment will not be cached
 * - SEGMENT_TYPE_PREFETCH   : the segment will be prefetched 
 */

#ifndef MAPPING_TABLE_H
#define MAPPING_TABLE_H

#include <list>
#include <iostream>
#include <cassert>
#include <systemc.h>

/*
 * We would like to have a list of passed args with -1 at the end.
 * The preprocessor can handle variable length macros, and may even
 * remove a comma separating arguments in expanded result,
 * unfortunately it can only do so for a comma _preceding_ a macro
 * argument expansion.
 *
 * So here is a little hack, we add an useless value before expansion
 * for the sole purpose of having a removable comma. The argument is
 * discarded by the __list+1 expression afterwards.
 *
 * On all cases, result is coherent, we have a returned value looking
 * like { args..., -1 }, or { -1 } when empty.
 */
#define intList(x...) ({int __list[] = {0,##x, -1}; __list+1;})

#define SEGMENT_EXTERNAL_TARGET 0xfffffffe

typedef enum {
    SEGMENT_TYPE_UNCHECKED = 0x0,
    SEGMENT_TYPE_CACHED = 0x1,
    SEGMENT_TYPE_UNCACHED = 0x2,
    SEGMENT_TYPE_PREFETCH = 0x3,
    SEGMENT_TYPE_INVALID
} SEGMENT_TYPE;

const char *segment_type_str[] = {
    "SEGMENT_TYPE_UNCHECKED",
    "SEGMENT_TYPE_CACHED",
    "SEGMENT_TYPE_UNCACHED",
    "SEGMENT_TYPE_PREFETCH",
    "SEGMENT_TYPE_INVALID",
    NULL
};

/*
 * SEGMENT_TABLE_ENTRY definition
 */

struct SEGMENT_TABLE_ENTRY {
private:
    const char *name;
    int *addr_bits, *ids;
    unsigned int base, size;
    SEGMENT_TYPE type;

public:
    SEGMENT_TABLE_ENTRY( const char *_name,
                         unsigned int _base, 
                         unsigned int _size,
                         int *_ids,
                         SEGMENT_TYPE _type,
                         int *_addr_bits) {
        name = _name;
        base = _base;
        size = _size;
        type = _type;

        int levels = 0;
        while (_ids[levels]!=-1) ++levels;

        ids = new int[levels+1];
        for (int i=0; i<levels; ++i)
            ids[i] = _ids[i];
        ids[levels] = -1;

        addr_bits = new int[levels+1];
        for (int i=0; i<levels; ++i)
            addr_bits[i] = _addr_bits[i];
        addr_bits[levels] = -1;
        
        if(type >= SEGMENT_TYPE_INVALID) {
            std::cerr << "ERROR allocating segment :" << name << std::endl
                 << "The segment's type must be in" << std::endl
                 << "SEGMENT_TYPE_UNCHECKED, SEGMENT_TYPE_CACHED," << std::endl
                 << "SEGMENT_TYPE_UNCACHED, SEGMENT_TYPE_PREFETCH." << std::endl;
            exit(1);
        }
    }

    bool idMatches(int *oids) {
        int *a = ids, *o = oids;
        
        while (*o != -1 && *a == *o) {
            ++a; ++o;
        }
        if ( *o == -1 )
            return true;
        return false;
    }
    
    void print() {
        std::cout << "segment: "<<name<<", base = 0x"<<std::hex<<base<< ", size = 0x"<<size
             << ", index = ("<< std::dec ;
        for ( int i=0; ids[i]!=-1; ++i )
            std::cout << (int)ids[i] << ((ids[i+1]!=-1)?", ":"");
        std::cout << "), type = " << segment_type_str[type] << std::endl;
    }

    const char *getName()  {return name;}
    unsigned int getBase() {return base;}
    unsigned int getSize() {return size;}
    unsigned int getId(int level) {
        return ids[level];
    }
    unsigned int getIdTo(int *ids_width, int level) {
        unsigned int r=0;
        for ( int i=0; i<level; i++ )
            r = (r<<ids_width[i])|ids[i];
        return r;
    }
    unsigned int getType() {return type;}
    int *getAddrBits() {return addr_bits;}

    unsigned int getAddrTo(int *addr_bits_widths, int level) {
        unsigned int ret = 0;
        int i;
        for (i=0; i<level; ++i) {
            ret = (ret<<(addr_bits_widths[i]))|addr_bits[i];
        }
        return ret;
    }
    
    bool isCompatible( int *other_ids,
                       unsigned int other_base,
                       int *other_addr_bits,
                       unsigned int other_size) {
        if(((base < other_base) && ((base+size-1) >= other_base)) ||
           ((base > other_base) && ((other_base+other_size-1) >= base)) ||
           (base == other_base))
            return false;
        
        for ( int level = 0; addr_bits[level]!=-1; ++level ) {
            if ( addr_bits[level] == other_addr_bits[level] ) {
                if ( ids[level] != other_ids[level] )
                    return false;
            } else
                return true;
        }
        return true;
    }
};// end SEGMENT_TABLE_ENTRY

/*
 * MappingTable definition
 */

#define TYPE_OF(x) (((x)>>type_bit)&3)

struct SOCLIB_MAPPING_TABLE {

private:
    std::list<SEGMENT_TABLE_ENTRY>     segList;
    unsigned int addr_size, levels;
    int *addr_bits;
    int *id_bits, total_id_bits;
    unsigned int type_bit;

public:
    SOCLIB_MAPPING_TABLE( unsigned int _addr_size,
                          unsigned int _levels,
                          int *_addr_bits,
                          int *_id_bits,
                          unsigned int _type_bit_mask ) {
        int i = 0;
        unsigned int tbm = _type_bit_mask;

        while (tbm && tbm != 3) {
            ++i;
            tbm >>= 1;
        }
        if (tbm != 3) {
            fprintf(stderr, "Incorrect type bit mask: 0x%08x\n",
                    _type_bit_mask);
            sc_stop();
        }

        addr_size= _addr_size;
        levels = _levels;
        type_bit = i;

        // Have to copy them
        addr_bits = new int[levels+1];
        for (unsigned int i=0; i<levels; ++i)
            addr_bits[i] = _addr_bits[i];
        addr_bits[levels] = -1;

        id_bits = new int[levels+1];
        total_id_bits = 0;
        for (unsigned int i=0; i<levels; ++i) {
            id_bits[i] = _id_bits[i];
            total_id_bits += id_bits[i];
        }
        id_bits[levels] = -1;
    }

    int getAddrBits(int level) {
        return addr_bits[level];
    }

    int getIdBits(int level) {
        return id_bits[level];
    }

    int getAddrSize() {
        return addr_size;
    }

    int getSegTypeMask() {
        return 3<<type_bit;
    }

    void addrToBits(unsigned int address, int *dest) {
        int r = 0;
        for (unsigned int i=0; i<levels; ++i) {
            r += addr_bits[i];
            dest[i] = (address>>(addr_size-r))&((1<<addr_bits[i])-1);
        }
        dest[levels] = -1;
    }

    void idToBits(unsigned int id, int *dest) {
        int r = 0;
        for (int i=levels-1; i>=0; --i) {
            dest[i] = (id>>r)&((1<<id_bits[i])-1);
            r += id_bits[i];
        }
        dest[levels] = -1;
    }

    int idToStr(unsigned int id, char *str, size_t len) {
        int *dest = new int[levels+1];
        size_t left = len;
        unsigned int i;

        idToBits(id, dest);
        for (i=0; i<levels; ++i) {
            int ret = snprintf(str, left, "%d%s", dest[i], (i==levels-1)?"":",");
            if (ret < 0) {
                delete(dest);
                return -1;
            }
            left -= ret;
            str += ret;
        }
        delete[] dest;
        return 0;
    }

    unsigned int ident(int *idbits) {
        return ident(levels, idbits);
    }

    unsigned int ident(int level, int *idbits) {
        unsigned int ret = 0;
        int i;
        assert(idbits[level] == -1);
        for (i=0; i<level; ++i)
            ret = (ret<<(id_bits[i]))|idbits[i];
        return ret;
    }

    void paramsTo( int level,
                   int &a_off, int &a_mask,
                   int &i_off, int &i_mask) {
        a_off = addr_size;
        a_mask = 0;
        for (int i=0; i<level; ++i) {
            a_off -= addr_bits[i];
            a_mask += addr_bits[i];
        }
        a_mask = (1<<a_mask)-1;

        i_off = total_id_bits;
        i_mask = 0;
        for (int i=0; i<level; ++i) {
            i_off -= id_bits[i];
            i_mask += id_bits[i];
        }
        i_mask = (1<<i_mask)-1;
    }

    void localParams( int level,
                      int &a_off, int &a_mask,
                      int &i_off, int &i_mask) {
        a_off = addr_size;
        for (int i=0; i<=level; ++i)
            a_off -= addr_bits[i];
        a_mask = (1<<addr_bits[level])-1;

        i_off = total_id_bits;
        for (int i=0; i<=level; ++i)
            i_off -= id_bits[i];
        i_mask = (1<<id_bits[level])-1;
    }

///////////////////////////////////////////////////////////////
//        addSegment()
//  This method introduces one segment  in the segment table.
//  The segment non-overlaping is checked.
//  The cache type is checked, versus the segment mapping.
//  The local index and global index are checked versus the
//  values of LSBnumber and MSBnumber.
///////////////////////////////////////////////////////////////
    void addSegment( const char *name,
                     unsigned int base,
                     unsigned int size,
                     unsigned int id,
                     SEGMENT_TYPE type) {
        bool compatible = true;
        int *addr_bits = new int[levels+1];
        int *index = new int[levels+1];

        idToBits(id, index);

        addrToBits(base, addr_bits);
        SEGMENT_TABLE_ENTRY *nseg = new SEGMENT_TABLE_ENTRY(name, base, size, index, type, addr_bits);

        std::list<SEGMENT_TABLE_ENTRY>::iterator seg ;
        for (seg = segList.begin() ; seg != segList.end() ; ++seg) {
            compatible &= seg->isCompatible( index, base, addr_bits, size );
            if ( !compatible ) {
                std::cout << "ERROR:"<<std::endl;
                nseg->print();
                std::cout << "is not compatible with"<<std::endl;
                seg->print();
                exit(1);
            }
        }

        unsigned int adrmintype    = base>>type_bit;
        unsigned int adrmaxtype    = (base+size-1)>>type_bit;
        if(adrmintype != adrmaxtype) {
            std::cerr << "ERROR in the Mapping Table for segment :" << name << std::endl ;
            std::cerr << "All addresses have not the same cache type " << std::endl ;
            exit(1);
        }
        if (( TYPE_OF(base) != SEGMENT_TYPE_UNCHECKED && type != SEGMENT_TYPE_UNCHECKED && type != (int) TYPE_OF(base) ) ||
		(((adrmintype&3) >  1 ) && (type == SEGMENT_TYPE_CACHED)) ||
		(((adrmintype&3) != 2 ) && (type == SEGMENT_TYPE_UNCACHED)) ||
		(((adrmintype&3) != 3 ) && (type == SEGMENT_TYPE_PREFETCH)))
	
	{
            std::cerr << "ERROR in the Mapping Table for segment :" << name << std::endl ;
            std::cerr << "The declared type is " << segment_type_str[type] << std::endl ;
            std::cerr << "... and doesn't fit with the segment mapping"  << std::endl ;
            std::cerr << "adrmintype = " << segment_type_str[(adrmintype&3)] << std::endl;
            std::cerr << "type from addr = " << segment_type_str[TYPE_OF(base)] <<
                " (0x" << std::hex << base << "&0x" << std::hex << (3<<type_bit) << ")" << std::dec << std::endl;
            exit(1);
        }

        for (unsigned int i=0; i<levels; ++i ) {
            if (index[i] >= 1<<id_bits[i] ) {
                std::cerr << "ERROR in the Mapping Table for segment: " << name << std::endl
                     << "The segment index at level " << i << std::endl
                     << "is too large for the set value" << std::endl
                     << "(" << index[i] << ">=(1<<" << id_bits[i] << "))" << std::endl;
                exit(1);
            }
        }

        char idstr[15];
        idToStr(id, idstr, 15);
        std::cout << "Adding segment for target " << idstr << std::endl;
        nseg->print();
        segList.push_back(*nseg);
        delete[] addr_bits;
        delete[] index;
	delete nseg;
    }  // end addSegment()

    void print() {
        std::cerr << " == MAPPING_TABLE ==" << std::endl << std::endl ;
        std::list<SEGMENT_TABLE_ENTRY>::iterator iter;
        for (iter = segList.begin() ; iter != segList.end() ; ++iter) {
            iter->print();
        }
        std::cerr << std::endl ;
    }

    int levelCount(int *list) {
        int i=0;
        while(list[i]!=-1) ++i;
        return i;
    }

    int addrBitsToLevel(int l) {
        int r=0;
        for (int i=0; i<l; ++i)
            r+=addr_bits[i];
        return r;
    }

    void printRoutingTableTo(int level, int *indexes) {
        std::cout << " == ROUTING TABLE TO "<<" ==" << std::endl;

        int size = 1<<addrBitsToLevel(level);
        int *tab = new int[size];
        initRoutingTableTo(level, indexes, tab);
        for(int index = 0 ; index < size ; index++)
            std::cout << "TAB[0x" << std::hex << index << "] = 0x" << tab[index] << std::dec << std::endl;
        std::cout << std::endl ;
        delete(tab);
    }

    void printLocalRoutingTable(int level, int *indexes) {
        std::cout << " == LOCAL ROUTING TABLE @ "<<"==" << std::endl;

        int size = 1<<addr_bits[level];
        int *tab = new int[size];
        initLocalRoutingTable(level, indexes, tab);
        for(int index = 0 ; index < size ; index++)
            std::cout << "TAB[0x" << std::hex << index << "] = 0x" << tab[index] << std::dec << std::endl;
        std::cout << std::endl ;
        delete(tab);
    }

#define MT_UNALLOCATED 0xffffffff

    void initRoutingTableTo(int level, int *ids, int* tab) {
        int lowestid = -1;
        std::list<SEGMENT_TABLE_ENTRY>::iterator seg;

        int addr_bits_to = 1<<addrBitsToLevel(level);

        for (int i=0; i<addr_bits_to; ++i )
            tab[i] = MT_UNALLOCATED;
        
        for ( seg = segList.begin();
              seg != segList.end();
              ++seg ) {
            int id = seg->getIdTo(id_bits, level);
            if (!seg->idMatches(ids))
                if (id<lowestid) lowestid = id;
            tab[seg->getAddrTo(addr_bits, level)] = id;
        }
        for (int i=0; i<addr_bits_to; ++i )
            if (tab[i] == (int) MT_UNALLOCATED)
                tab[i] = lowestid;
    }

    void initLocalRoutingTable(int level, int *ids, int* tab) {
        int lowestid = -1;
        std::list<SEGMENT_TABLE_ENTRY>::iterator seg;

        int *our_id = new int[level+1];
        for (int i=0; i<level; ++i) our_id[i] = ids[i];
        our_id[level] = -1;
        int size = 1<<addr_bits[level];

        for (int i=0; i<size; ++i )
            tab[i] = MT_UNALLOCATED;
        
        for ( seg = segList.begin();
              seg != segList.end();
              ++seg ) {
            if (seg->idMatches(our_id)) {
                int id = seg->getId(level);
                if (id<lowestid) lowestid = id;
                tab[seg->getAddrBits()[level]] = id;
            }
        }
        for (int i=0; i<size; ++i )
            if (tab[i] == (int) MT_UNALLOCATED)
                tab[i] = lowestid;
        
    }

#undef MT_UNALLOCATED

//////////////////////////////////////////////////////////////
//        getSegmentList()
//    returns the list of all segments 
//    allocated to a given VCI target.
//////////////////////////////////////////////////////////////

    std::list<SEGMENT_TABLE_ENTRY> getSegmentList(int id) {
        std::list<SEGMENT_TABLE_ENTRY> targetlist;
        std::list<SEGMENT_TABLE_ENTRY>::iterator segIterator;
        int *ids = new int[levels+1];
        idToBits(id, ids);

        for (segIterator = segList.begin() ; segIterator != segList.end() ; ++segIterator) {
            if (segIterator->idMatches(ids)) {
                targetlist.push_back(*segIterator);
            }
        }
        delete[] ids;
        return(targetlist);
    }

////////////////////////////////////////////////////
// function getTotalsegmentList
// used to get the whole segmentlist
///////////////////////////////////////////////////

    std::list<SEGMENT_TABLE_ENTRY> getTotalMappingList() {
        return(segList);
    }

};  // end struct SOCLIB_MAPPING_TABLE

#undef TYPE_OF
#endif

