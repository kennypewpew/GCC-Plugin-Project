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
#include <gimple-expr.h>

#include <tree-pretty-print.h>
#include <gimple-pretty-print.h>
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

// Introducing memory leak here
const char* isolate_name(const char *fn) {
  bool flag = false;
  int first, last;
  char space[] = " ";
  char open[] = "(";
  for ( int i = 0 ; i < strlen(fn) ; ++i )
    if ( fn[i] == space[0] ) {
      if ( !flag ) {
	flag = true;
	first = i+1;
      }
      else {
	last = i;
	break;
      }
    }
    else if ( fn[i] == open[0] ) {
      last = i;
      break;
    }

  if ( flag ) {
    int length = last - first;
    char *res = new char[length];
    memcpy(res, fn+first, length);
    return res;
  }
  /* 
  // make this else work properly to be able to free/delete memory afterwards
  else {
    int length = strlen(fn);
    char *res = new char[length];
    memcpy(res, fn, length);
    return res;
  }
  */
  return fn;
}

bool function_to_check(const char *fn) {
  for ( int i = 0 ; i < instr_args.length() ; ++i )
    if ( 0 == strcmp(isolate_name(fn), instr_args[i]) ) {
      arg_used[i] = true;
      return true;
    }
  return false;
}


static void test_if_all_used(void *event_data, void *data) {
  printf("[pragma] Testing if all %d functions were checked\n", 
	 arg_used.length());
  // Returns wrong file number as error
  bool unused = false;
  for ( int i = 0 ; i < arg_used.length() ; ++i )
    if ( !arg_used[i] ) {
      //warning (OPT_Wpragmas,
      printf(
      "[pragma] warning: Function \"%s\" declared to analyze, but not found\n",
	       instr_args[i]);
      unused = true;
    }
  if ( !unused ) printf("[pragma] Success!\n");
}
/******* End analysis storage/functions ******/

/******** Insert functions ********/
enum IO {
  RD = 0,
  WR = 1
};

tree insert_function(gimple_stmt_iterator &gsi) {
  gimple fn_call;

  // Return type of function to insert, followed by types of args
  tree fn_type = build_function_type_list(void_type_node,
					  NULL_TREE);
  // Operation to create declaration of function
  tree fn_decl = build_fn_decl("alloc_loop_vector", fn_type);

  fn_call = gimple_build_call(fn_decl, 0);
  gsi_insert_before(&gsi, fn_call, GSI_SAME_STMT);

  
  gimple next_fn_call;
  tree next_fn_type = build_function_type_list(void_type_node,
					       NULL_TREE);
  tree next_fn_decl = build_fn_decl("insert_info", next_fn_type);
  next_fn_call = gimple_build_call(next_fn_decl, 0);

  gsi_insert_before(&gsi, next_fn_call, GSI_NEW_STMT);

  
  return fn_type;
}

void insert_stock_fn(gimple_stmt_iterator &gsi, tree var, enum IO type) {
  gimple fn_call;

  // Return type of function to insert, followed by types of args
  tree fn_type = build_function_type_list(void_type_node,
					  ptr_type_node,
					  integer_type_node,
					  size_type_node,
					  NULL_TREE);
  // Operation to create declaration of function
  tree fn_decl = build_fn_decl("insert_info", fn_type);

  tree io_type = build_int_cst(integer_type_node, type);

  tree var_type = TREE_TYPE(var);
  if ( TREE_CODE(var_type) == POINTER_TYPE ) var_type = TREE_TYPE(var_type);
  fn_call = gimple_build_call(fn_decl, 3, var, io_type,
			      TYPE_SIZE(var_type));
  gsi_insert_before(&gsi, fn_call, GSI_SAME_STMT);


  return;
}

void insert_post_loop_treatment(gimple_stmt_iterator gsi) {
  gimple fn_call;

  tree fn_type = build_function_type_list(void_type_node,
					  NULL_TREE);
  tree fn_decl = build_fn_decl("analyze_loop", fn_type);

  fn_call = gimple_build_call(fn_decl, 0);
  gsi_insert_before(&gsi, fn_call, GSI_SAME_STMT);

  return;
}


void insert_print_var(gimple_stmt_iterator &gsi, tree var, tree var2) {
  char string[] = "[%s] lhs value is %d, rhs is %d\n";
  tree string_tree = fix_string_type(build_string(strlen(string)+1, string));

  tree string_type = build_pointer_type(TREE_TYPE(TREE_TYPE(string_tree)));
  tree string_args_tree = build1(ADDR_EXPR, string_type, string_tree);

  const char *fString = gimple_decl_printable_name(cfun->decl, 3);
  tree fString_tree = fix_string_type( build_string(strlen(fString)+1,
						    fString) );
  tree fString_type = build_pointer_type (
					  TREE_TYPE(TREE_TYPE(fString_tree)));
  tree fString_args_tree = build1(ADDR_EXPR, fString_type, fString_tree);

  tree print_fn = builtin_decl_implicit(BUILT_IN_PRINTF);
  gimple print_gimple = gimple_build_call(print_fn, 4,
					  string_args_tree,
					  fString_args_tree,
					  var, var2);
  
  gsi_insert_before(&gsi, print_gimple, GSI_SAME_STMT);
  
  return;
}
void insert_print_var(gimple_stmt_iterator &gsi, tree var, tree var2, tree var3) {
  char string[] = "[%s] lhs value is %d, rhs is %d, %d\n";
  tree string_tree = fix_string_type(build_string(strlen(string)+1, string));

  tree string_type = build_pointer_type(TREE_TYPE(TREE_TYPE(string_tree)));
  tree string_args_tree = build1(ADDR_EXPR, string_type, string_tree);

  const char *fString = gimple_decl_printable_name(cfun->decl, 3);
  tree fString_tree = fix_string_type( build_string(strlen(fString)+1,
						    fString) );
  tree fString_type = build_pointer_type (
					  TREE_TYPE(TREE_TYPE(fString_tree)));
  tree fString_args_tree = build1(ADDR_EXPR, fString_type, fString_tree);

  tree print_fn = builtin_decl_implicit(BUILT_IN_PRINTF);
  gimple print_gimple = gimple_build_call(print_fn, 5,
					  string_args_tree,
					  fString_args_tree,
					  var, var2, var3);
  
  gsi_insert_before(&gsi, print_gimple, GSI_SAME_STMT);
  
  return;
}
void insert_print_string(gimple_stmt_iterator &gsi, const char *string) {
  tree string_tree = fix_string_type(build_string(strlen(string)+1, string));

  tree string_type = build_pointer_type(TREE_TYPE(TREE_TYPE(string_tree)));
  tree string_args_tree = build1(ADDR_EXPR, string_type, string_tree);

  tree print_fn = builtin_decl_implicit(BUILT_IN_PRINTF);
  gimple print_gimple = gimple_build_call(print_fn, 1, string_args_tree);

  gsi_insert_before(&gsi, print_gimple, GSI_SAME_STMT);
  
  return;
}

// Note: don't really need to pass the stmt if you're already passing gsi
bool analyze_stmt(gimple stmt, gimple prev_stmt, gimple_stmt_iterator &gsi) {
  const_tree op;
  size_t i;

  //gimple stmt = gsi_stmt(gsi);

  tree lhs, rhs, rhs2;
  int flag = 1;
  
  if ( is_gimple_assign(stmt) ) {
    // make sure this includes mult, add, etc as well
    
    debug_gimple_stmt(stmt);
    for ( i = 0 ; i < gimple_num_ops(stmt) ; ++i ) {
      op = gimple_op(stmt, i);
      if ( op ) {
	if ( TREE_CODE(op) == POINTER_TYPE ) flag = 1;
	if ( TREE_CODE(op) == OFFSET_TYPE ) flag = 1;
	printf("%s\n", get_tree_code_name( TREE_CODE(op) ) );
	switch( TREE_CODE( op ) )
	  {
	  case ARRAY_REF: printf("ARRAY_REF\n"); break;
	  case ARRAY_RANGE_REF: printf("ARRAY_RANGE_REF\n"); break;
	  case TARGET_MEM_REF: printf("TARGET_MEM_REF\n"); break;
	  case ADDR_EXPR: printf("ADDR_EXPR\n"); break;
	  case INDIRECT_REF: printf("INDIRECT_REF\n"); break;
	  case MEM_REF: flag = 1;
	    printf("MEM_REF\n"); break;
	  case COMPONENT_REF: printf("COMPONENT_REF!\n");break;
	  case RECORD_TYPE: break;
	  default: break;
	  }
      }
    }
    printf("Assignment - %d operands\n", gimple_num_ops(stmt));

    if ( flag ) {

      print_gimple_stmt(stdout, stmt, 0, TDF_SLIM);
      lhs = gimple_assign_lhs(stmt);
      rhs = gimple_assign_rhs1(stmt);
      if ( gimple_num_ops(stmt) == 3 ) {
	rhs2 = gimple_assign_rhs2(stmt);
	insert_print_var(gsi, lhs, rhs, rhs2);
	if ( TREE_CODE(TREE_TYPE(rhs)) == POINTER_TYPE ) {
	  insert_stock_fn(gsi, lhs, WR);
	  char string[] = "Pointer being used: %p, %d bytes in\n";
	  tree string_tree = fix_string_type(build_string(strlen(string)+1,
							  string));
	  tree string_type = build_pointer_type(
					     TREE_TYPE(TREE_TYPE(string_tree)));
	  tree string_args_tree = build1(ADDR_EXPR, string_type, string_tree);
	  
	  tree print_fn = builtin_decl_implicit(BUILT_IN_PRINTF);
	  gimple print_gimple = gimple_build_call(print_fn, 3,
						  string_args_tree,
						  rhs, rhs2);
	  
	  gsi_insert_before(&gsi, print_gimple, GSI_NEW_STMT);
	}
      }
      else {
	insert_print_var(gsi, lhs, rhs);
	if ( TREE_CODE(TREE_TYPE(rhs)) == POINTER_TYPE ) {
	  insert_stock_fn(gsi, rhs, RD);
	  insert_print_string(gsi, "Pointer being used!\n");
	  char string[] = "Pointer being used: %p, %d'th element\n";
	  tree string_tree = fix_string_type(build_string(strlen(string)+1,
							  string));
	  tree string_type = build_pointer_type(
					     TREE_TYPE(TREE_TYPE(string_tree)));
	  tree string_args_tree = build1(ADDR_EXPR, string_type, string_tree);
	  
	  tree print_fn = builtin_decl_implicit(BUILT_IN_PRINTF);
	  gimple print_gimple = gimple_build_call(print_fn, 2,
						  string_args_tree,
						  rhs, rhs2);
	  
	  gsi_insert_before(&gsi, print_gimple, GSI_NEW_STMT);
	}
      }
    
    }


    return true;
  }

  return false;
}

void find_vars() {
  printf("Finding variables of %s\n", fndecl_name(cfun->decl));

  basic_block bb;
  gimple_stmt_iterator gsi;
  gimple stmt, prev_stmt;

  FOR_EACH_BB_FN(bb, cfun) {    
    gsi = gsi_start_bb(bb);
    while ( gsi.ptr != (gsi_last_bb(bb)).ptr ) {
      prev_stmt = stmt;
      stmt = gsi_stmt(gsi);
      gsi_next(&gsi);
      if ( stmt && prev_stmt ) {
	analyze_stmt(stmt, prev_stmt, gsi);
      }
    }
    gsi = gsi_last_bb(bb);
    //insert_print_string(gsi, "End basic block\n");
  }
}


/******** End Insert functions ********/


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
      const char *full_string = IDENTIFIER_POINTER (tmpTree) ;
      current_string = isolate_name(full_string);
      printf("[pragma] Function \"%s\" recognized as needing analysis\n",
	     current_string);

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

    // Is it really this easy to only touch innermost loops?
    if ( !cLoop->inner ) {
      printf("Innermost loop found!\n");
      basic_block preheader = loop_preheader_edge(cLoop)->src;
      gimple_stmt_iterator gsi_pre = gsi_start_bb(preheader);

      // initialize analysis data structures(gsi_pre/preheader)
      // inject logging functions(gsi_post/postheader)
      
      insert_print_string(gsi_pre,
			  "Entering innermost loop\n");
      insert_function(gsi_pre);

      //edge post_edge = loop_latch_edge(cLoop);
      edge post_edge = single_exit(cLoop);
      basic_block post_block = post_edge->dest;
      gimple_stmt_iterator gsi_post = gsi_start_bb(post_block);
      insert_print_string(gsi_post,
			  "Exiting loop\n");
      insert_post_loop_treatment(gsi_post);
    }
    // Use ->src to add before start of loop, ->dest to add inside start of loop
    //basic_block preLoop = loop_preheader_edge(cLoop)->src;
    //placeholder(preLoop);
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
    const char *current_function = fndecl_name(cfun->decl);
    printf("[analysis_pass] Entering gate: %s\n", current_function);

    if ( function_to_check(current_function) ) {
      const char *current_function = fndecl_name(cfun->decl);
      printf("[analysis_pass] Analyzing function %s\n", current_function);

      return true;
    }
    else
      return false; 
  }

  unsigned int execute() {

    // very rare crash somewhere in analyze_fn_loops
    analyze_fn_loops(cfun);

    find_vars();
    
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
