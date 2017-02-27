#ifdef ENABLE_DEBUGGING
#define DEBUG(expr) expr
#endif

#ifndef ENABLE_DEBUGGING
#define DEBUG(expr) {}
#endif
