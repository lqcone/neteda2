
//#define NETDATA_WITH_ZLIB 1


/* gcc branch optimization */
#define likely(x) __builtin_expect(!!(x), 1)

/* jemalloc prefix */
#define prefix_jemalloc 

/* Define to the type of an unsigned integer type of width exactly 16 bits if
   such a type exists and the standard includes do not define it. */
   /* #undef uint16_t */

   /* Define to the type of an unsigned integer type of width exactly 32 bits if
      such a type exists and the standard includes do not define it. */
      /* #undef uint32_t */

      /* Define to the type of an unsigned integer type of width exactly 64 bits if
         such a type exists and the standard includes do not define it. */
         /* #undef uint64_t */

         /* Define to the type of an unsigned integer type of width exactly 8 bits if
            such a type exists and the standard includes do not define it. */
            /* #undef uint8_t */

            /* gcc branch optimization */
#define unlikely(x) __builtin_expect(!!(x), 0)