
/* assert macros:
 * http://www.pixelbeat.org/programming/gcc/static_assert.html
 */

#define ASSERT_CONCAT_(a, b) a##b
#define ASSERT_CONCAT(a, b) ASSERT_CONCAT_(a, b)
/* These can't be used after statements in c89. */
#ifdef __COUNTER__
  #define STATIC_ASSERT(e,m) \
    ;enum { ASSERT_CONCAT(static_assert_, __COUNTER__) = 1/(int)(!!(e)) }
#else
  /* This can't be used twice on the same line so ensure if using in headers
   * that the headers are not included twice (by wrapping in #ifndef...#endif)
   * Note it doesn't cause an issue when used on same line of separate modules
   * compiled with gcc -combine -fwhole-program.  */
  #define STATIC_ASSERT(e,m) \
    ;enum { ASSERT_CONCAT(assert_line_, __LINE__) = 1/(int)(!!(e)) }
#endif

tp_inline static int _tp_min(int a, int b) { return (a<b?a:b); }
tp_inline static int _tp_max(int a, int b) { return (a>b?a:b); }
tp_inline static tpd_frame * tp_get_frame(TP, int i) { return TPD_FRAME(tp->frames->items[i]); }
tp_inline static tpd_frame * tp_get_cur_frame(TP) { return tp_get_frame(tp, tp->frames->len - 1); }

/* Detect unintended size changes. Update as needed. */
STATIC_ASSERT(sizeof(tpd_code) == 4, "size of tpd_code must be 4");
