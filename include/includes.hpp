#include <gcc-plugin.h>
//#include "tree-ssa.h"
//#include <plugin.h>

#include "cp/cp-tree.h"

#include "plugin.h"
#include "bversion.h"
#include "plugin-version.h"
#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "line-map.h"
#include "input.h"
#include "tree.h"

#include "tree-inline.h"
#include "version.h"
#include "rtl.h"
#include "tm_p.h"
#include "flags.h"
//#include "insn-attr.h"
//#include "insn-config.h"
//#include "insn-flags.h"
#include "hard-reg-set.h"
//#include "recog.h"
#include "output.h"
#include "except.h"
#include "function.h"
#include "toplev.h"
//#include "expr.h"
#include "basic-block.h"
#include "intl.h"
#include "ggc.h"
//#include "regs.h"
#include "timevar.h"

#include "params.h"
#include "pointer-set.h"
#include "emit-rtl.h"
//#include "reload.h"
//#include "ira.h"
//#include "dwarf2asm.h"
#include "debug.h"
#include "target.h"
#include "langhooks.h"
#include "cfgloop.h"
//#include "hosthooks.h"
#include "cgraph.h"
#include "opts.h"
//#include "coverage.h"
//#include "value-prof.h"

#if BUILDING_GCC_VERSION >= 4007
#include "tree-pretty-print.h"
#include "gimple-pretty-print.h"
//#include "c-tree.h"
//#include "alloc-pool.h"
#endif

#include "diagnostic.h"
//#include "tree-diagnostic.h"
#include "tree-dump.h"
#include "tree-pass.h"
//#include "df.h"
#include "predict.h"
//#include "lto-streamer.h"
#include "ipa-utils.h"

#if BUILDING_GCC_VERSION >= 4009
#include "varasm.h"
#include "stor-layout.h"
#include "internal-fn.h"
#include "gimple-expr.h"
//#include "diagnostic-color.h"
#include "context.h"
#include "tree-ssa-alias.h"
#include "stringpool.h"
#include "tree-ssanames.h"
#include "print-tree.h"
#include "tree-eh.h"
#endif

#include "gimple.h"

#if BUILDING_GCC_VERSION >= 4009
#include "tree-ssa-operands.h"
#include "tree-phinodes.h"
#include "tree-cfg.h"
#include "gimple-iterator.h"
#include "gimple-ssa.h"
#include "ssa-iterators.h"
#endif

#include "cp/name-lookup.h"
