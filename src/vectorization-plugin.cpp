#include <gcc-plugin.h>

#include <input.h>
#include <c-family/c-pragma.h>    // c_register_pragma

int plugin_is_GPL_compatible;

static void dummyHandler(cpp_reader *ARG_UNUSED(notUsed)) {
  printf("Pragma recognized\n");
  return;
}

static void register_vector_pragmas(void *even_data, void *data) {
  printf("Registering vectorization pragmas\n");
  c_register_pragma("MIHPS", "vcheck", dummyHandler);
}

int plugin_init (struct plugin_name_args *plugin_info,
		 struct plugin_gcc_version *version ) {
  printf("Vectorization plugin loaded!\n");

  register_callback(plugin_info->base_name,
		    PLUGIN_PRAGMAS,
		    register_vector_pragmas,
		    NULL);

  return 0;
}
