#include <gcc-plugin.h>

#include <tree.h>
#include <tree-pass.h>
#include <basic-block.h>

#include "includes.hpp"

#include <context.h>
#include <input.h>
#include <c-family/c-pragma.h>     // c_register_pragma
#include <function.h>              // cfun
#include <diagnostic.h>            // error(...)
#include <intl.h>                  // G_(...)

//#include <builtins.h>

#include <iostream>
#include <string>

int plugin_is_GPL_compatible;

/******* Begin analysis storage/functions ******/
vec<const char*, va_heap, vl_ptr> instr_args;
vec<bool, va_heap, vl_ptr> arg_used;


void check_instr_args_for_doubles() {
  for ( int i = 0 ; i < instr_args.length() ; ++i ) {
    bool flag = false;
    for ( int j = i-1 ; j > -1 ; --j ) {
      if ( 0 == strcmp(instr_args[i],instr_args[j]) )
	flag = true;
    }
    if ( flag == true ) {
      warning (OPT_Wpragmas,
	       "[pragma] Function \"%s\" declared to be checked multiple times",
	       instr_args[i]);
      arg_used[i] = true;
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
  printf("[pragma] Testing if all %d functions were checked\n", 
	 arg_used.length());
  // Returns wrong file number as error
  for ( int i = 0 ; i < arg_used.length() ; ++i )
    if ( !arg_used[i] ) 
      warning (OPT_Wpragmas,
	       "[pragma] Function \"%s\" declared to analyze, but not found",
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
      printf("[pragma] Function \"%s\" recognized as needing analysis\n",current_string);

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
  check_instr_args_for_doubles();
  return;
}

static void register_vector_pragmas(void *even_data, void *data) {
  printf("[pragma] Registering vectorization pragmas\n");
  c_register_pragma("MIHPS", "vcheck", vcheck_pragma_handler);
}

/******* End pragma handling ********/

/****** Begin new pass *********************/

void placeholder(basic_block bb) {
  tree mystring_tree;

  char startstring[] = "[placeholder] This is a loop of %s\n";
  mystring_tree = fix_string_type( build_string(strlen(startstring)+1,
						startstring) );

  tree mystring_type = build_pointer_type (TREE_TYPE(
						     TREE_TYPE(mystring_tree)));
  tree mystring_args_tree = build1(ADDR_EXPR, mystring_type, mystring_tree);

  const char *fString = gimple_decl_printable_name(cfun->decl, 3);
  tree fString_tree = fix_string_type( build_string(strlen(fString)+1,
						    fString) );
  tree fString_type = build_pointer_type (
					  TREE_TYPE(TREE_TYPE(fString_tree)));
  tree fString_args_tree = build1(ADDR_EXPR, fString_type, fString_tree);

  tree fn = builtin_decl_implicit(BUILT_IN_PRINTF);
  gimple print_gimple = gimple_build_call(fn, 2, 
					  mystring_args_tree,
					  fString_args_tree);
  
  gimple_stmt_iterator gsi = gsi_start_bb(bb);
  gsi_insert_before(&gsi, print_gimple, GSI_NEW_STMT);

}

void analyze_fn_loops(struct function *fn) {
  const char *fn_name = fndecl_name(fn->decl);

  struct loop *cLoop = get_loop(fn, 0);
  FOR_EACH_LOOP(cLoop, 0) {
    printf("[%s] loop detected\n", fn_name);

    // Use ->src to add before start of loop, ->dest to add inside start of loop
    basic_block preLoop = loop_preheader_edge(cLoop)->src;
    placeholder(preLoop);
  }

  return;
}

const pass_data loop_analysis_pass_data =
  {
    GIMPLE_PASS, 
    "loop_analysis_pass",
    OPTGROUP_NONE,
    true,
    true,
    TV_OPTIMIZE,
    0,
    0,
    0,
    0,
    0
  };

class analysis_pass: public gimple_opt_pass {
public:
  analysis_pass (gcc::context *ctxt)
    : gimple_opt_pass(loop_analysis_pass_data, ctxt) {}

  bool gate() { 
    std::cerr << "[analysis_pass] Entering gate\n";
    const char *current_function = fndecl_name(cfun->decl);

    if ( function_to_check(current_function) ) {
      const char *current_function = fndecl_name(cfun->decl);
      printf("[analysis_pass] Analyzing function %s\n", current_function);

      return true;
    }
    else
      return false; 
  }

  unsigned int execute() {
    //struct loops *loop_start = loops_for_fn(cfun);
    analyze_fn_loops(cfun);

    return 0;
  }

  analysis_pass* clone() { return new analysis_pass(g); }
};

/******** End new pass **********/

int plugin_init (struct plugin_name_args *plugin_info,
		 struct plugin_gcc_version *version ) {
  printf("[init] Vectorization plugin loaded!\n");

  // Declare pragmas
  register_callback(plugin_info->base_name,
		    PLUGIN_PRAGMAS,
		    register_vector_pragmas,
		    NULL);

  // Insert loop analysis pass
  struct register_pass_info loop_analysis_pass_info;
  analysis_pass analyze(g);

  loop_analysis_pass_info.pass = &analyze;
  loop_analysis_pass_info.reference_pass_name = "*record_bounds";
  loop_analysis_pass_info.ref_pass_instance_number = 0;
  loop_analysis_pass_info.pos_op = PASS_POS_INSERT_AFTER;

  register_callback(plugin_info->base_name,
		    PLUGIN_PASS_MANAGER_SETUP,
		    NULL,
		    &loop_analysis_pass_info);


  // Check if all functions actually analyzed
  register_callback(plugin_info->base_name,
		    PLUGIN_FINISH,
		    test_if_all_used,
		    NULL);


  return 0;
}
