#include <gcc-plugin.h>

#include <tree.h>
#include <tree-pass.h>
#include <context.h>
#include <input.h>
#include <c-family/c-pragma.h>     // c_register_pragma
#include <function.h>              // cfun
#include <diagnostic.h>            // error(...)
#include <intl.h>                  // G_(...)

#include <iostream>
#include <string>

int plugin_is_GPL_compatible;

/******* Begin analysis storage/functions ******/
vec<const char*, va_heap, vl_ptr> instr_args;
vec<bool, va_heap, vl_ptr> arg_used;


void check_instr_args_for_doubles() {
  for ( int i = 0 ; i < instr_args.length() ; ++i ) {
    bool flag = false;
    for ( int j = i+1 ; j < instr_args.length() ; ++j ) {
      if ( 0 == strcmp(instr_args[i],instr_args[j]) )
	flag = true;
    }
    if ( flag == true ) {
      warning (OPT_Wpragmas,
	       "Function %s declared to be checked multiple times",
	       instr_args[i]);
      // Remove doublets here?
    }
  }

  return;
}

bool function_to_check(const char *fn) {
  for ( int i = 0 ; i < instr_args.length() ; ++i )
    if ( 0 == strcmp(fn, instr_args[i]) ) {
      arg_used[i] = true;
      return true;
    }
  return false;
}


static void test_if_all_used(void *event_data, void *data) {
  for ( int i = 0 ; i < arg_used.length() ; ++i )
    if ( !arg_used[i] ) 
      warning (OPT_Wpragmas,
	       "Function %s declared to analyze, but not found",
	       instr_args[i]);
}
/******* End analysis storage/functions ******/

/******* Begin pragma handling *******/
static void vcheck_pragma_handler(cpp_reader *ARG_UNUSED(notUsed)) {
  tree tmpTree;
  enum cpp_ttype tmpType;
  const char *current_string;
  bool need_close_paren = false;

  if (cfun) {
    error("#pragma instrument function not allowed inside functions");
    return;
  }

  tmpType = pragma_lex(&tmpTree);

  if ( tmpType == CPP_OPEN_PAREN) {
    need_close_paren = true;
    tmpType = pragma_lex (&tmpTree);
  }

  if ( tmpType != CPP_NAME) {
    warning (OPT_Wpragmas, "%<#pragma instrument function%> is not a string");
    return;
  }
  else {
    while ( tmpType == CPP_NAME) {
      current_string = IDENTIFIER_POINTER (tmpTree);
      printf("Function %s recognized as needing analysis\n", current_string);

      instr_args.safe_push(current_string);
      arg_used.safe_push(false);

      tmpType = pragma_lex (&tmpTree);
      while ( tmpType == CPP_COMMA)
	tmpType = pragma_lex (&tmpTree);
    }

    if ( need_close_paren ) {
      if ( tmpType == CPP_CLOSE_PAREN )
	tmpType = pragma_lex (&tmpTree);
      else
	warning (OPT_Wpragmas,
		 "%<#pragma instrument function (string [,string]...)%> "
		 "does not have final %<)%>");
    }
  }
  std::cout << "Pragma handled!\n";
  check_instr_args_for_doubles();
  //printf("Pragma recognized\n");
  return;
}

static void register_vector_pragmas(void *even_data, void *data) {
  printf("Registering vectorization pragmas\n");
  c_register_pragma("MIHPS", "vcheck", vcheck_pragma_handler);
}

/******* End pragma handling ********/

int plugin_init (struct plugin_name_args *plugin_info,
		 struct plugin_gcc_version *version ) {
  printf("Vectorization plugin loaded!\n");

  register_callback(plugin_info->base_name,
		    PLUGIN_PRAGMAS,
		    register_vector_pragmas,
		    NULL);

  return 0;
}
