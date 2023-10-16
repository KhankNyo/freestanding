#ifndef FREESTANDING_ASSERT_H
#define FREESTANDING_ASSERT_H


#define FS_STATIC_ASSERT(expr, msg) int static_assert(int static_assert[(expr)?1:-1]) 



#endif /* FREESTANDING_ASSERT_H */

