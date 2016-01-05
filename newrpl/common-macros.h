#ifndef COMMONMACROS_H
#define COMMONMACROS_H

#define FIRST(a, ...) a
#define SECOND(a, b, ...) b

#define IS_PROBE(...) SECOND(__VA_ARGS__, 0)
#define PROBE() ~, 1

#define CAT(a,b) a ## b

#define NOT(x) IS_PROBE(CAT(_NOT_, x))
#define _NOT_0 PROBE()

#define BOOL(x) NOT(NOT(x))

#define IF_ELSE(condition) _IF_ELSE(BOOL(condition))
#define _IF_ELSE(condition) CAT(_IF_, condition)

#define _IF_1(...) __VA_ARGS__ _IF_1_ELSE
#define _IF_0(...)             _IF_0_ELSE

#define _IF_1_ELSE(...)
#define _IF_0_ELSE(...) __VA_ARGS__


#define EVAL(...) EVAL1024(__VA_ARGS__)
#define EVAL1024(...) EVAL512(EVAL512(__VA_ARGS__))
#define EVAL512(...) EVAL256(EVAL256(__VA_ARGS__))
#define EVAL256(...) EVAL128(EVAL128(__VA_ARGS__))
#define EVAL128(...) EVAL64(EVAL64(__VA_ARGS__))
#define EVAL64(...) EVAL32(EVAL32(__VA_ARGS__))
#define EVAL32(...) EVAL16(EVAL16(__VA_ARGS__))
#define EVAL16(...) EVAL8(EVAL8(__VA_ARGS__))
#define EVAL8(...) EVAL4(EVAL4(__VA_ARGS__))
#define EVAL4(...) EVAL2(EVAL2(__VA_ARGS__))
#define EVAL2(...) EVAL1(EVAL1(__VA_ARGS__))
#define EVAL1(...) __VA_ARGS__

#define EMPTY()

#define DEFER1(m) m EMPTY()
#define DEFER2(m) m EMPTY EMPTY()()
#define DEFER3(m) m EMPTY EMPTY EMPTY()()()


#define HAS_ARGS(...) BOOL(FIRST(_END_OF_ARGUMENTS_ __VA_ARGS__)())
#define _END_OF_ARGUMENTS_() 0

#define MAP(m, first, ...)           \
  m(first)                           \
  IF_ELSE(HAS_ARGS(__VA_ARGS__))(    \
    DEFER2(_MAP)()(m, __VA_ARGS__)   \
  )(                                 \
     \
  )
#define _MAP() MAP

#define STRING(a) _STRING(a)
#define _STRING(a) #a

#define COUNTWRAP(i) 1 +
#define COUNTARGS(...) EVAL(MAP(COUNTWRAP , __VA_ARGS__)) 0

#define CATWRAP(i) i##.c
#define CATARGS(...) EVAL(MAP(CATWRAP , __VA_ARGS__))

#define HEAD(first, ...) first
#define TAIL(first, ...) __VA_ARGS__

#define _NTH(n, ...) EVAL1(DEFER1(HEAD)(TAILN(n)(__VA_ARGS__)))
#define NTH(...) _NTH(__VA_ARGS__)
#define TAILN(n) TAIL_##n


#define TAIL_0(...) __VA_ARGS__
#define TAIL_1(...) TAIL(__VA_ARGS__)
#define TAIL_2(...) EVAL1(DEFER1(TAIL)(TAIL_1(__VA_ARGS__)))
#define TAIL_3(...) EVAL1(DEFER1(TAIL)(TAIL_2(__VA_ARGS__)))
#define TAIL_4(...) EVAL1(DEFER1(TAIL)(TAIL_3(__VA_ARGS__)))
#define TAIL_5(...) EVAL1(DEFER1(TAIL)(TAIL_4(__VA_ARGS__)))
#define TAIL_6(...) EVAL1(DEFER1(TAIL)(TAIL_5(__VA_ARGS__)))
#define TAIL_7(...) EVAL1(DEFER1(TAIL)(TAIL_6(__VA_ARGS__)))
#define TAIL_8(...) EVAL1(DEFER1(TAIL)(TAIL_7(__VA_ARGS__)))
#define TAIL_9(...) EVAL1(DEFER1(TAIL)(TAIL_8(__VA_ARGS__)))
#define TAIL_10(...) EVAL1(DEFER1(TAIL)(TAIL_9(__VA_ARGS__)))
#define TAIL_11(...) EVAL1(DEFER1(TAIL)(TAIL_10(__VA_ARGS__)))
#define TAIL_12(...) EVAL1(DEFER1(TAIL)(TAIL_11(__VA_ARGS__)))
#define TAIL_13(...) EVAL1(DEFER1(TAIL)(TAIL_12(__VA_ARGS__)))
#define TAIL_14(...) EVAL1(DEFER1(TAIL)(TAIL_13(__VA_ARGS__)))
#define TAIL_15(...) EVAL1(DEFER1(TAIL)(TAIL_14(__VA_ARGS__)))
#define TAIL_16(...) EVAL1(DEFER1(TAIL)(TAIL_15(__VA_ARGS__)))
#define TAIL_17(...) EVAL1(DEFER1(TAIL)(TAIL_16(__VA_ARGS__)))
#define TAIL_18(...) EVAL1(DEFER1(TAIL)(TAIL_17(__VA_ARGS__)))



#endif // COMMONMACROS_H

