#include "ncxtypes.h"
#include "val.h"
val_value_t* agt_nmda_get_root_operational(void);
val_value_t* agt_nmda_get_root_system(void);


/* module initialization and cleanup */
status_t agt_nmda_init (void);
status_t agt_nmda_init2 (void);
void agt_nmda_cleanup (void);
