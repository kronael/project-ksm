extern bool DEBUGGING_ENABLED;

#ifdef ENABLE_DEBUGGING
#define DEBUG(expr) if(DEBUGGING_ENABLED) expr
#endif

#ifndef ENABLE_DEBUGGING
#define DEBUG(expr) {}
#endif
