#include <clondike/tcmi/tcmi_dbg.h>
#include <linux/cache.h>
#include <linux/module.h>

int tcmi_dbg __read_mostly = 1;
EXPORT_SYMBOL_GPL(tcmi_dbg);
