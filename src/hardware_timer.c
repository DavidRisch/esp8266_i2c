

// workaround for problem in hw_timer.c, FRC1 is not used anyway
#undef ETS_FRC1_INTR_ENABLE
#define ETS_FRC1_INTR_ENABLE()

#include <hw_timer.c>
