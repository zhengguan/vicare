/*
 * Ikarus Scheme -- A compiler for R6RS Scheme.
 * Copyright (C) 2011, 2012 Marco Maggi <marco.maggi-ipsu@poste.it>
 * Copyright (C) 2006,2007,2008  Abdulaziz Ghuloum
 *
 * This program is free software:  you can redistribute it and/or modify
 * it under  the terms of  the GNU General  Public License version  3 as
 * published by the Free Software Foundation.
 *
 * This program is  distributed in the hope that it  will be useful, but
 * WITHOUT  ANY   WARRANTY;  without   even  the  implied   warranty  of
 * MERCHANTABILITY  or FITNESS FOR  A PARTICULAR  PURPOSE.  See  the GNU
 * General Public License for more details.
 *
 * You should  have received  a copy of  the GNU General  Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef VICARE_H
#  define VICARE_H


/** --------------------------------------------------------------------
 ** Headers.
 ** ----------------------------------------------------------------- */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>


/** --------------------------------------------------------------------
 ** Helper macros.
 ** ----------------------------------------------------------------- */

/* The macro  IK_UNUSED indicates that a function,  function argument or
   variable may potentially be unused.  Usage examples:

   static int unused_function (char arg) IK_UNUSED;
   int foo (char unused_argument IK_UNUSED);
   int unused_variable IK_UNUSED;
*/
#ifdef __GNUC__
#  define IK_UNUSED		__attribute__((unused))
#else
#  define IK_UNUSED		/* empty */
#endif

#ifndef __GNUC__
#  define __attribute__(...)	/* empty */
#endif

#ifndef vicare_decl
#  define vicare_decl		extern
#endif


/** --------------------------------------------------------------------
 ** Global constants.
 ** ----------------------------------------------------------------- */

#define pagesize		4096

/* How much  to right-shift a pointer  value to obtain the  index of the
   page (of size PAGESIZE) it is in.

       4000 >> 12 = 0
       8000 >> 12 = 1
      10000 >> 12 = 2
*/
#define pageshift		12

#define generation_count	5  /* generations 0 (nursery), 1, 2, 3, 4 */

#define IK_HEAP_EXT_SIZE  (32 * 4096)
#define IK_HEAPSIZE       (1024 * ((wordsize==4)?1:2) * 4096) /* 4/8 MB */

#define IK_FASL_HEADER		((sizeof(ikptr) == 4)? "#@IK01" : "#@IK02")
#define IK_FASL_HEADER_LEN	(strlen(IK_FASL_HEADER))

#define ik_ptr_page_size \
  ((pagesize - sizeof(long) - sizeof(struct ik_ptr_page*))/sizeof(ikptr))

/* Given the  pointer X evaluate to the  index of the memory  page it is
   in. */
#define IK_PAGE_INDEX(x)   \
  (((ik_ulong)(x)) >> pageshift)

#define IK_ALIGN_TO_NEXT_PAGE(x) \
  (((pagesize - 1 + (ik_ulong)(x)) >> pageshift) << pageshift)

#define IK_ALIGN_TO_PREV_PAGE(x) \
  ((((ik_ulong)(x)) >> pageshift) << pageshift)

#define IK_ASS(LEFT,RIGHT)	\
  { ikptr s_tmp = (RIGHT); (LEFT) = s_tmp; }


/** --------------------------------------------------------------------
 ** Global data types.
 ** ----------------------------------------------------------------- */

typedef signed int		ik_int;
typedef signed long		ik_long;
typedef signed long long	ik_llong;
typedef unsigned int		ik_uint;
typedef unsigned long		ik_ulong;
typedef unsigned long long	ik_ullong;

/* FIXME Should this be a "uintptr_t"? (Marco Maggi; Nov  6, 2011). */
typedef unsigned long		ikptr;

typedef struct ikpage{
  ikptr base;
  struct ikpage* next;
} ikpage;

/* Node for linked list of allocated pages. */
typedef struct ikpages {
  ikptr base;
  int size;
  struct ikpages* next;
} ikpages;

typedef struct ikdl{ /* double-link */
  struct ikdl* prev;
  struct ikdl* next;
} ikdl;


/* Node in  a linked list  referencing all the generated  FFI callbacks.
   It is used  to allow the garbage collector not  to collect data still
   in  use by  the callbacks.   See "ikarus-ffi.c"  for details  on this
   structure. */
typedef struct ik_callback_locative {
  void *        callable_pointer;       /* pointer to callable C function */
  void *        closure;                /* data generated by Libffi */
  ikptr         data;                   /* Scheme value holding required data */
  struct ik_callback_locative * next;   /* pointer to next link */
} ik_callback_locative;

typedef struct ik_ptr_page {
  long count;
  struct ik_ptr_page* next;
  ikptr ptr[ik_ptr_page_size];
} ik_ptr_page;

typedef struct ikpcb
{
  /* the first locations may be accessed by some     */
  /* compiled code to perform overflow/underflow ops */
  ikptr   allocation_pointer;           /* offset =  0 */
  ikptr   allocation_redline;           /* offset =  4 */
  ikptr   frame_pointer;                /* offset =  8 */
  ikptr   frame_base;                   /* offset = 12 */
  ikptr   frame_redline;                /* offset = 16 */
  ikptr   next_k;                       /* offset = 20 */
  ikptr   system_stack;                 /* offset = 24 */
  ikptr   dirty_vector;                 /* offset = 28 */
  ikptr   arg_list;                     /* offset = 32 */
  ikptr   engine_counter;               /* offset = 36 */
  ikptr   interrupted;                  /* offset = 40 */
  ikptr   base_rtd;                     /* offset = 44 */
  ikptr   collect_key;                  /* offset = 48 */

/* ------------------------------------------------------------------ */
  /* The  following fields are  not used  by any  scheme code  they only
     support the runtime system (gc, etc.) */

  /* Linked  list of  FFI callback  support data.   Used by  the garbage
     collector  not  to collect  data  still  needed  by some  callbacks
     registered in data structures handled by foreign libraries. */
  ik_callback_locative * callbacks;

  /* Additional roots for the garbage collector.  They are used to avoid
     collecting objects still in use while they are in use by C code. */
  ikptr*                root0;
  ikptr*                root1;
  ikptr*                root2;
  ikptr*                root3;
  ikptr*                root4;
  ikptr*                root5;
  ikptr*                root6;
  ikptr*                root7;
  ikptr*                root8;
  ikptr*                root9;

  unsigned int*         segment_vector;
  ikptr                 weak_pairs_ap;
  ikptr                 weak_pairs_ep;
  /* Pointer  to  the current  heap  memory  segment.   New objects  are
     allocated here. */
  ikptr                 heap_base;
  /* Number of bytes in the current heap memory segment. */
  ik_ulong         heap_size;
  /* Pointer to first node in  linked list of allocated memory segments.
     Initialised to  NULL when building  the PCB.  Whenever  the current
     heap is full: a new node is prepended to the list, initialised with
     the fields "heap_base" and "heap_size". */
  ikpages*              heap_pages;
  /* pages cached so that we don't map/unmap */
  ikpage*               cached_pages;
  /* ikpages cached so that we don't malloc/free */
  ikpage*               uncached_pages;
  ikptr                 cached_pages_base;
  int                   cached_pages_size;
  ikptr                 stack_base;
  ik_ulong         stack_size;
  ikptr                 symbol_table;
  ikptr                 gensym_table;
  ik_ptr_page*          protected_list[generation_count];
  unsigned int*         dirty_vector_base;
  unsigned int*         segment_vector_base;
  ikptr                 memory_base;
  ikptr                 memory_end;

  /* Number of garbage collections performed so far.  It is used: at the
     beginning  of a  GC ru,  to determine  which objects  generation to
     inspect; when reporting GC statistics to the user, to show how many
     GCs where performed between two timestamps. */
  int                   collection_id;

  int                   allocation_count_minor;
  int                   allocation_count_major;

  /* Used for garbage collection statistics. */
  struct timeval        collect_utime;
  struct timeval        collect_stime;
  struct timeval        collect_rtime;

  /* Value of  "errno" right after the  last call to  a foreign function
     callout. */
  int                   last_errno;
} ikpcb;

typedef struct {
  ikptr tag;
  ikptr top;
  long size;
  ikptr next;
} cont;


/** --------------------------------------------------------------------
 ** Function prototypes.
 ** ----------------------------------------------------------------- */

int     ik_abort                (const char * error_message, ...);
void    ik_error                (ikptr args);
#ifndef NDEBUG
void	ik_debug_message	(const char * error_message, ...);
#else
#define ik_debug_message(MSG,...)	/* empty */
#endif

ikptr   ik_unsafe_alloc         (ikpcb* pcb, ik_ulong size);
ikptr   ik_safe_alloc           (ikpcb* pcb, ik_ulong size);

void    ik_print                (ikptr x);
void	ik_print_no_newline	(ikptr x);
void    ik_fprint               (FILE*, ikptr x);


/** --------------------------------------------------------------------
 ** Basic object related macros.
 ** ----------------------------------------------------------------- */

#define wordsize        ((int)(sizeof(ikptr)))
/* The value of "wordshift" is selected in such a way that:

     length_in_bytes = number_of_words * wordsize
                     = number_of_words << wordshift

   this  allows us,  for example,  to take  the fixnum  representing the
   number of items  in a vector and consider it directly  as size of the
   vector's data area in bytes. */
#define wordshift       ((4 == wordsize)? 2 : 3)
#define IK_ALIGN_SHIFT  (1 + wordshift)
#define IK_ALIGN_SIZE   (2 * wordsize)
#define immediate_tag   7

#define IK_TAGOF(X)     (((int)(X)) & 7)

#define IK_REF(X,N)     (((ikptr*)(((long)(X)) + ((long)(N))))[0])
#define ref(X,N)        IK_REF((X),(N))

/* The smallest multiple of the wordsize which is greater than N. */
#define IK_ALIGN(N) \
  ((((N) + IK_ALIGN_SIZE - 1) >>  IK_ALIGN_SHIFT) << IK_ALIGN_SHIFT)

#define false_object            ((ikptr)0x2F)
#define true_object             ((ikptr)0x3F)
#define null_object             ((ikptr)0x4F)
#define eof_object              ((ikptr)0x5F)
#define void_object             ((ikptr)0x7F)

/* Special machine word value stored in locations that used to hold weak
   references to values which have been already garbage collected. */
#define bwp_object              ((ikptr)0x8F)

/* Special machine word value stored  in the "value" and "proc" field of
   Scheme symbol memory blocks to signal that these fields are unset. */
#define unbound_object          ((ikptr)0x6F)


/** --------------------------------------------------------------------
 ** Code objects.
 ** ----------------------------------------------------------------- */

/* This  is the  primary tag,  in the  machine word  referencing  a code
   object. */
#define code_pri_tag            vector_tag
/* This is the  secondary tag, in the first word  of the referenced heap
   vector. */
#define code_tag                ((ikptr)0x2F)
#define disp_code_code_size     (1 * wordsize)
#define disp_code_reloc_vector  (2 * wordsize)
#define disp_code_freevars      (3 * wordsize)
#define disp_code_annotation    (4 * wordsize)
#define disp_code_unused        (5 * wordsize)
#define disp_code_data          (6 * wordsize)
#define off_code_annotation     (disp_code_annotation   - code_pri_tag)
#define off_code_data           (disp_code_data         - code_pri_tag)
#define off_code_reloc_vector   (disp_code_reloc_vector - code_pri_tag)


/** --------------------------------------------------------------------
 ** Fixnum objects.
 ** ----------------------------------------------------------------- */

#define fx_tag          0
#define fx_shift        wordshift
#define fx_mask         (wordsize - 1)

#define most_positive_fixnum    (((ik_ulong)-1) >> (fx_shift+1))
#define most_negative_fixnum    (most_positive_fixnum+1)

#define IK_FIX(X)       ((ikptr)(((long)(X)) << fx_shift))
#define fix(X)          IK_FIX(X)

#define IK_UNFIX(X)     (((long)(X)) >> fx_shift)
#define unfix(X)        IK_UNFIX(X)

#define IK_IS_FIXNUM(X)    \
  ((((ik_ulong)(X)) & fx_mask) == fx_tag)


/** --------------------------------------------------------------------
 ** Pair and list objects.
 ** ----------------------------------------------------------------- */

#define pair_size       (2 * wordsize)
#define pair_mask       7 /* #b111 */
#define pair_tag        1
#define disp_car        0
#define disp_cdr        wordsize
#define off_car         (disp_car - pair_tag)
#define off_cdr         (disp_cdr - pair_tag)

#define IK_IS_PAIR(X)   (pair_tag == (((long)(X)) & pair_mask))

#define IK_CAR(PAIR)                IK_REF((PAIR), off_car)
#define IK_CDR(PAIR)                IK_REF((PAIR), off_cdr)
#define IK_CAAR(PAIR)               IK_CAR(IK_CAR(PAIR))
#define IK_CDAR(PAIR)               IK_CDR(IK_CAR(PAIR))

#define IKA_DECLARE_ALLOC_AND_CONS(PAIR,LIST,PCB)    \
  ikptr PAIR = IKA_PAIR_ALLOC(PCB);                  \
  IK_CDR(PAIR) = LIST;                              \
  LIST=PAIR;

#define IKA_PAIR_ALLOC(PCB)	(ik_safe_alloc((PCB), IK_ALIGN(pair_size)) | pair_tag)

long    ik_list_length                  (ikptr x);
void    ik_list_to_argv                 (ikptr x, char **argv);
void    ik_list_to_argv_and_argc        (ikptr x, char **argv, long *argc);

ikptr   ika_list_from_argv              (ikpcb * pcb, char ** argv);
ikptr   ika_list_from_argv_and_argc     (ikpcb * pcb, char ** argv, long argc);


/** --------------------------------------------------------------------
 ** Character objects.
 ** ----------------------------------------------------------------- */

typedef uint32_t        ikchar;

#define char_tag        0x0F
#define char_mask       0xFF
#define char_shift      8

#define IK_IS_CHAR(X)           (char_tag == (char_mask & (ikptr)(X)))

#define IK_CHAR_FROM_INTEGER(X) \
  ((ikptr)((((ik_ulong)(X)) << char_shift) | char_tag))

#define IK_CHAR32_FROM_INTEGER(X) \
  ((ikchar)((((ik_ulong)(X)) << char_shift) | char_tag))

#define IK_CHAR_TO_INTEGER(X) \
  ((ik_ulong)(((ikptr)(X)) >> char_shift))


/** --------------------------------------------------------------------
 ** String objects.
 ** ----------------------------------------------------------------- */

#define string_char_size        4
#define string_mask             7
#define string_tag              6
#define disp_string_length      0
#define disp_string_data        wordsize
#define off_string_length       (disp_string_length - string_tag)
#define off_string_data         (disp_string_data   - string_tag)

#define IK_IS_STRING(X)                 (string_tag == (string_mask & (ikptr)(X)))
#define IK_STRING_LENGTH_FX(STR)        IK_REF((STR), off_string_length)
#define IK_STRING_LENGTH(STR)           IK_UNFIX(IK_REF((STR), off_string_length))
#define IK_CHAR32(STR,IDX)              (((ikchar*)(((long)(STR)) + off_string_data))[IDX])

extern ikptr ika_string_alloc           (ikpcb * pcb, long number_of_chars);
extern ikptr ikrt_string_to_symbol      (ikptr, ikpcb* pcb);
extern ikptr ikrt_strings_to_gensym     (ikptr, ikptr,  ikpcb* pcb);


/** --------------------------------------------------------------------
 ** Symbol objects.
 ** ----------------------------------------------------------------- */

#define symbol_tag                      ((ikptr) 0x5F)
#define disp_symbol_record_tag		0
#define disp_symbol_record_string       (1 * wordsize)
#define disp_symbol_record_ustring      (2 * wordsize)
#define disp_symbol_record_value        (3 * wordsize)
#define disp_symbol_record_proc         (4 * wordsize)
#define disp_symbol_record_plist        (5 * wordsize)
#define symbol_record_size              (6 * wordsize)

#define off_symbol_record_tag		(disp_symbol_record_tag     - record_tag)
#define off_symbol_record_string        (disp_symbol_record_string  - record_tag)
#define off_symbol_record_ustring       (disp_symbol_record_ustring - record_tag)
#define off_symbol_record_value         (disp_symbol_record_value   - record_tag)
#define off_symbol_record_proc          (disp_symbol_record_proc    - record_tag)
#define off_symbol_record_plist         (disp_symbol_record_plist   - record_tag)


/** --------------------------------------------------------------------
 ** Bignum objects.
 ** ----------------------------------------------------------------- */

#define bignum_mask             0x7
#define bignum_tag              0x3
#define bignum_sign_mask        0x8
#define bignum_sign_shift       3
#define bignum_length_shift     4
#define disp_bignum_data        wordsize
#define off_bignum_data         (disp_bignum_data - vector_tag)

vicare_decl ikptr   ika_integer_from_int		(ikpcb* pcb, int N);
vicare_decl ikptr   ika_integer_from_long		(ikpcb* pcb, long N);
vicare_decl ikptr   ika_integer_from_long_long		(ikpcb* pcb, ik_llong n);
vicare_decl ikptr   ika_integer_from_unsigned_int	(ikpcb* pcb, unsigned N);
vicare_decl ikptr   ika_integer_from_unsigned_long	(ikpcb* pcb, ik_ulong N);
vicare_decl ikptr   ika_integer_from_unsigned_long_long	(ikpcb* pcb, ik_ullong N);
vicare_decl ikptr   ik_flonum_from_double		(ikpcb* pcb, double N);

vicare_decl int32_t	ik_integer_to_sint32 (ikptr x);
vicare_decl int64_t	ik_integer_to_sint64 (ikptr x);
vicare_decl uint32_t	ik_integer_to_uint32 (ikptr x);
vicare_decl uint64_t	ik_integer_to_uint64 (ikptr x);

vicare_decl int                 ik_integer_to_int                   (ikptr x);
vicare_decl unsigned int        ik_integer_to_unsigned_int          (ikptr x);
vicare_decl long                ik_integer_to_long                  (ikptr x);
vicare_decl ik_ulong       ik_integer_to_unsigned_long         (ikptr x);
vicare_decl ik_llong           ik_integer_to_long_long             (ikptr x);
vicare_decl ik_ullong  ik_integer_to_unsigned_long_long    (ikptr x);


/** --------------------------------------------------------------------
 ** Ratnum objects.
 ** ----------------------------------------------------------------- */

#define ratnum_tag              ((ikptr) 0x27)
#define disp_ratnum_num         (1 * wordsize)
#define disp_ratnum_den         (2 * wordsize)
#define disp_ratnum_unused      (3 * wordsize)
#define ratnum_size             (4 * wordsize)


/** --------------------------------------------------------------------
 ** Compnum objects.
 ** ----------------------------------------------------------------- */

#define compnum_tag             ((ikptr) 0x37)
#define disp_compnum_real       (1 * wordsize)
#define disp_compnum_imag       (2 * wordsize)
#define disp_compnum_unused     (3 * wordsize)
#define compnum_size            (4 * wordsize)


/** --------------------------------------------------------------------
 ** Flonum objects.
 ** ----------------------------------------------------------------- */

#define flonum_tag              ((ikptr)0x17)
#define flonum_size             16
#define disp_flonum_data        8 /* not f(wordsize) */
#define off_flonum_data         (disp_flonum_data - vector_tag)

#define IKU_DEFINE_AND_ALLOC_FLONUM(VARNAME)		\
  ikptr VARNAME = ik_unsafe_alloc(pcb, flonum_size)	\
    | vector_tag;					\
  IK_REF(VARNAME, -vector_tag) = (ikptr)flonum_tag

ikptr   iku_flonum_alloc	(ikpcb * pcb, double fl);

#define IK_FLONUM_DATA(X)       (*((double*)(((long)(X))+off_flonum_data)))

#define IKU_DEFINE_AND_ALLOC_CFLONUM(VARNAME)		\
  ikptr VARNAME = ik_unsafe_alloc(pcb, cflonum_size)	\
    | vector_tag;					\
  IK_REF(VARNAME, -vector_tag) = (ikptr)cflonum_tag


/** --------------------------------------------------------------------
 ** Cflonum objects.
 ** ----------------------------------------------------------------- */

#define cflonum_tag             ((ikptr) 0x47)
#define disp_cflonum_real       (1 * wordsize)
#define disp_cflonum_imag       (2 * wordsize)
#define disp_cflonum_unused     (3 * wordsize)
#define cflonum_size            (4 * wordsize)
#define off_cflonum_real        (disp_cflonum_real - vector_tag)
#define off_cflonum_imag        (disp_cflonum_imag - vector_tag)

ikptr   iku_cflonum_alloc	(ikpcb * pcb, double re, double im);

#define IK_CFLONUM_REAL(X)      IK_REF((X), off_cflonum_real)
#define IK_CFLONUM_IMAG(X)      IK_REF((X), off_cflonum_imag)
#define IK_CFLONUM_REAL_DATA(X)	IK_FLONUM_DATA(IK_CFLONUM_REAL(X))
#define IK_CFLONUM_IMAG_DATA(X)	IK_FLONUM_DATA(IK_CFLONUM_IMAG(X))


/** --------------------------------------------------------------------
 ** Pointer objects.
 ** ----------------------------------------------------------------- */

#define pointer_tag           ((ikptr) 0x107)
#define disp_pointer_data     (1 * wordsize)
#define pointer_size          (2 * wordsize)
#define off_pointer_data      (disp_pointer_data - vector_tag)

ikptr   ik_pointer_alloc        (ik_ulong memory, ikpcb* pcb);
ikptr   ikrt_is_pointer         (ikptr x);

#define IK_POINTER_DATA_VOIDP(X)  \
  ((void *)IK_REF((X), off_pointer_data))

#define IK_POINTER_DATA_CHARP(X)	((char *)   IK_REF((X), off_pointer_data))
#define IK_POINTER_DATA_UINT8P(X)	((uint8_t *)IK_REF((X), off_pointer_data))
#define IK_POINTER_DATA_LONG(X)		((long)     IK_REF((X), off_pointer_data))
#define IK_POINTER_DATA_LLONG(X)	((ik_llong) IK_REF((X), off_pointer_data))
#define IK_POINTER_DATA_ULONG(X)	((ik_ulong) IK_REF((X), off_pointer_data))
#define IK_POINTER_DATA_ULLONG(X)	((ik_ullong)IK_REF((X), off_pointer_data))

#define IK_POINTER_SET_NULL(X)		(IK_REF((X), off_pointer_data) = 0)


/** --------------------------------------------------------------------
 ** Vector objects.
 ** ----------------------------------------------------------------- */

#define vector_mask             7
#define vector_tag              5
#define disp_vector_length      0
#define disp_vector_data        wordsize
#define off_vector_data         (disp_vector_data   - vector_tag)
#define off_vector_length       (disp_vector_length - vector_tag)

extern ikptr ik_vector_alloc    (ikpcb * pcb, long number_of_items);
extern int   ik_is_vector       (ikptr s_vec);

#define IK_VECTOR_LENGTH_FX(VEC)        IK_REF((VEC), off_vector_length)
#define IK_VECTOR_LENGTH(VEC)           IK_UNFIX(IK_REF((VEC), off_vector_length))
#define IK_ITEM(VEC,IDX)                IK_REF((VEC), off_vector_data + (IDX) * wordsize)


/** --------------------------------------------------------------------
 ** Bytevector objects.
 ** ----------------------------------------------------------------- */

#define bytevector_mask         7
#define bytevector_tag          2
#define disp_bytevector_length  0
#define disp_bytevector_data    8 /* not f(wordsize) */
#define off_bytevector_length   (disp_bytevector_length - bytevector_tag)
#define off_bytevector_data     (disp_bytevector_data   - bytevector_tag)

#define IK_IS_BYTEVECTOR(X)    \
  (bytevector_tag == (((long)(X)) & bytevector_mask))

extern ikptr   ika_bytevector_alloc (ikpcb * pcb, long int requested_number_of_bytes);
extern ikptr   ik_bytevector_from_cstring       (ikpcb * pcb, const char * cstr);
extern ikptr   ik_bytevector_from_cstring_len   (ikpcb * pcb, const char * cstr, size_t len);
extern ikptr   ik_bytevector_from_memory_block  (ikpcb * pcb, void * memory, size_t length);

#define IK_BYTEVECTOR_LENGTH(BV)                    \
  IK_UNFIX(IK_REF((BV), off_bytevector_length))

#define IK_BYTEVECTOR_LENGTH_FX(BV)                 \
  IK_REF((BV), off_bytevector_length)

#define IK_BYTEVECTOR_DATA_CHARP(BV)                \
  ((char*)(long)((BV) + off_bytevector_data))

#define IK_BYTEVECTOR_DATA_UINT8P(BV)               \
  ((uint8_t*)(long)((BV) + off_bytevector_data))

#define IK_BYTEVECTOR_DATA_VOIDP(BV)                \
  ((void*)(long)((BV) + off_bytevector_data))


/** --------------------------------------------------------------------
 ** Struct objects.
 ** ----------------------------------------------------------------- */

#define record_mask             7
#define record_tag              vector_tag
#define disp_record_rtd         0
#define disp_record_data        wordsize
#define off_record_rtd          (disp_record_rtd  - record_tag)
#define off_record_data         (disp_record_data - record_tag)

#define rtd_tag                 record_tag
#define disp_rtd_rtd            0
#define disp_rtd_name           (1 * wordsize)
#define disp_rtd_length         (2 * wordsize)
#define disp_rtd_fields         (3 * wordsize)
#define disp_rtd_printer        (4 * wordsize)
#define disp_rtd_symbol         (5 * wordsize)
#define rtd_size                (6 * wordsize)

#define off_rtd_rtd             (disp_rtd_rtd     - rtd_tag)
#define off_rtd_name            (disp_rtd_name    - rtd_tag)
#define off_rtd_length          (disp_rtd_length  - rtd_tag)
#define off_rtd_fields          (disp_rtd_fields  - rtd_tag)
#define off_rtd_printer         (disp_rtd_printer - rtd_tag)
#define off_rtd_symbol          (disp_rtd_symbol  - rtd_tag)

extern ikptr    ik_struct_alloc (ikpcb * pcb, ikptr rtd);
extern int      ik_is_struct    (ikptr R);

#define IK_FIELD(STRUCT,FIELD)         \
  IK_REF((STRUCT), (off_record_data+(FIELD)*wordsize))


/** --------------------------------------------------------------------
 ** Port objects.
 ** ----------------------------------------------------------------- */

#define port_tag		0x3F
#define port_mask		0x3F
#define disp_port_attrs		0)
#define disp_port_index		(1 * wordsize)
#define disp_port_size		(2 * wordsize)
#define disp_port_buffer	(3 * wordsize)
#define disp_port_transcoder	(4 * wordsize)
#define disp_port_id		(5 * wordsize)
#define disp_port_read		(6 * wordsize)
#define disp_port_write		(7 * wordsize)
#define disp_port_get_position	(8 * wordsize)
#define disp_port_set_position	(9 * wordsize)
#define disp_port_close		(10 * wordsize)
#define disp_port_cookie	(11 * wordsize)
#define disp_port_unused1	(12 * wordsize)
#define disp_port_unused2	(13 * wordsize)
#define port_size		(14 * wordsize)

#define off_port_attrs		(disp_port_attrs	- vector_tag)
#define off_port_index		(disp_port_index	- vector_tag)
#define off_port_size		(disp_port_size		- vector_tag)
#define off_port_buffer		(disp_port_buffer	- vector_tag)
#define off_port_transcoder	(disp_port_transcoder	- vector_tag)
#define off_port_id		(disp_port_id		- vector_tag)
#define off_port_read		(disp_port_read		- vector_tag)
#define off_port_write		(disp_port_write	- vector_tag)
#define off_port_get_position	(disp_port_get_position	- vector_tag)
#define off_port_set_position	(disp_port_set_position	- vector_tag)
#define off_port_close		(disp_port_close	- vector_tag)
#define off_port_cookie		(disp_port_cookie	- vector_tag)
#define off_port_unused1	(disp_port_unused1	- vector_tag)
#define off_port_unused2	(disp_port_unused2	- vector_tag)


/** --------------------------------------------------------------------
 ** Closure objects.
 ** ----------------------------------------------------------------- */

#define closure_tag             3
#define closure_mask            7
#define disp_closure_code       0
#define disp_closure_data       wordsize
#define off_closure_code        (disp_closure_code - closure_tag)
#define off_closure_data        (disp_closure_data - closure_tag)

#define is_closure(X)   \
  ((((long)(X)) & closure_mask) == closure_tag)


/** --------------------------------------------------------------------
 ** Continuation objects.
 ** ----------------------------------------------------------------- */

#define continuation_tag		((ikptr)0x1F)
#define disp_continuation_tag		0
#define disp_continuation_top		(1 * wordsize)
#define disp_continuation_size		(2 * wordsize)
#define disp_continuation_next		(3 * wordsize)
#define continuation_size		(4 * wordsize)

#define off_continuation_tag		(disp_continuation_tag  - vector_tag)
#define off_continuation_top		(disp_continuation_top  - vector_tag)
#define off_continuation_size		(disp_continuation_size - vector_tag)
#define off_continuation_next		(disp_continuation_next - vector_tag)

#define system_continuation_tag         ((ikptr) 0x11F)
#define disp_system_continuation_tag    0
#define disp_system_continuation_top    (1 * wordsize)
#define disp_system_continuation_next   (2 * wordsize)
#define disp_system_continuation_unused (3 * wordsize)
#define system_continuation_size        (4 * wordsize)

#define off_system_continuation_tag	(disp_system_continuation_tag    - vector_tag)
#define off_system_continuation_top	(disp_system_continuation_top    - vector_tag)
#define off_system_continuation_next	(disp_system_continuation_next   - vector_tag)
#define off_system_continuation_unused	(disp_system_continuation_unused - vector_tag)


/** --------------------------------------------------------------------
 ** Tcbucket objects.
 ** ----------------------------------------------------------------- */

#define disp_tcbucket_tconc     (0 * wordsize)
#define disp_tcbucket_key       (1 * wordsize)
#define disp_tcbucket_val       (2 * wordsize)
#define disp_tcbucket_next      (3 * wordsize)
#define tcbucket_size           (4 * wordsize)

#define off_tcbucket_tconc      (disp_tcbucket_tconc - vector_tag)
#define off_tcbucket_key        (disp_tcbucket_key   - vector_tag)
#define off_tcbucket_val        (disp_tcbucket_val   - vector_tag)
#define off_tcbucket_next       (disp_tcbucket_next  - vector_tag)


/** --------------------------------------------------------------------
 ** Done.
 ** ----------------------------------------------------------------- */

#endif /* ifndef VICARE_H */

/* end of file */
