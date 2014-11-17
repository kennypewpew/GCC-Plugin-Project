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
#include <vector>

int plugin_is_GPL_compatible;

/******* Begin analysis storage/functions ******/
std::vector<const char*> instr_args;
std::vector<bool> arg_used;

// Implement and fill these for better user feedback
//std::vector<int> line_no;
//std::vector<const char*> file_name;

void check_instr_args_for_doubles() {
  bool flag;
  for ( int i = 0 ; i < instr_args.size() ; ++i ) {
    for ( int j = i-1 ; j > -1 ; --j ) {
      if ( 0 == strcmp(instr_args[i],instr_args[j]) )
	flag = true;
    }
    if ( flag == true ) {
      warning (OPT_Wpragmas,
	       "[pragma] Function \"%s\" declared to be checked multiple times",
	       instr_args[i]);
      
      instr_args.erase(instr_args.begin() + i);
      arg_used.erase(arg_used.begin() + i);
      i--; // because the next element replaces the current element
    }
    flag = false;
    }
  
  return;
}

// Introducing memory leak here (test with languages other than C++)
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
    size_t length = last - first + 1;
    char *res = new char[length];
    memcpy(res, fn+first, length);
    char eol[] = "\0";
    memcpy(res+length-1, eol, 1);
    return res;
  }

  // make this `else` work properly to be able to free/delete memory afterwards
  else {
    int length = strlen(fn)+1;
    char *res = new char[length];
    memcpy(res, fn, length);
    char eol[] = "\0";
    memcpy(res+length-1, eol, 1);
    return res;
  }

  return fn;
}

bool function_to_check(const char *fn) {
  for ( int i = 0 ; i < instr_args.size() ; ++i ) {
    if ( 0 == strcmp(isolate_name(fn), instr_args[i]) ) {
      arg_used[i] = true;
      return true;
    }
  }
  return false;
}


static void test_if_all_used(void *event_data, void *data) {
  // Returns wrong line number as error
  for ( int i = 0 ; i < arg_used.size() ; ++i )
    if ( !arg_used[i] ) {
      //warning (OPT_Wpragmas,
      printf(
      "[pragma] warning: Function \"%s\" declared to analyze, but not found\n",
	       instr_args[i]);
      instr_args.erase(instr_args.begin() + i);
      arg_used.erase(arg_used.begin() + i);
      i--; // because the next element replaces the current element
    }
  printf("Functions that will be analyzed:  ");
  for ( int i = 0 ; i < instr_args.size()-1 ; ++i ) 
    printf("%s, ", instr_args[i]);
  printf("%s\n", instr_args[instr_args.size()-1]);
}
/******* End analysis storage/functions ******/

/******** Insert functions ********/
enum IO {
  RD = 0,
  WR = 1
};

void insert_alloc_function(gimple_stmt_iterator &gsi) {
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
  return;
}

void insert_stock_fn(gimple_stmt_iterator &gsi,
		     tree var, tree index,
		     enum IO type) {
  gimple fn_call;
  
  // Return type of function to insert, followed by types of args
  tree fn_type = build_function_type_list(void_type_node,
					  ptr_type_node,
					  integer_type_node,
					  integer_type_node,
					  size_type_node,
					  NULL_TREE);
  // Operation to create declaration of function
  tree fn_decl = build_fn_decl("insert_info", fn_type);
  tree io_type = build_int_cst(integer_type_node, type);

  tree var_type = TREE_TYPE(var);
  if ( TREE_CODE(var_type) == POINTER_TYPE )
    var_type = TREE_TYPE(var_type);
  if ( TREE_CODE(var_type) == ARRAY_TYPE )
    var_type = TREE_TYPE(var_type);
  
  fn_call = gimple_build_call(fn_decl, 4, var, index, io_type,
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


static void insert_print_var(gimple_stmt_iterator &gsi, tree var, tree var2) {
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

static void insert_print_var(gimple_stmt_iterator &gsi, tree var, tree var2,
			     tree var3) {
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
static void insert_print_string(gimple_stmt_iterator &gsi, const char *string) {
  tree string_tree = fix_string_type(build_string(strlen(string)+1, string));

  tree string_type = build_pointer_type(TREE_TYPE(TREE_TYPE(string_tree)));
  tree string_args_tree = build1(ADDR_EXPR, string_type, string_tree);

  tree print_fn = builtin_decl_implicit(BUILT_IN_PRINTF);
  gimple print_gimple = gimple_build_call(print_fn, 1, string_args_tree);

  gsi_insert_before(&gsi, print_gimple, GSI_SAME_STMT);
  
  return;
}

void analyze_gimple_op(gimple_stmt_iterator &gsi,
		       tree op,
		       enum IO io_type) {

  tree zero_index;
  switch( TREE_CODE( op ) )
    {
    case INTEGER_CST: 
    case REAL_CST: 
    case VAR_DECL:
    case PARM_DECL:
    case CONST_DECL:
    case STRING_CST:
      // Non-variables. Don't think we need to handle these
      break;
    case SSA_NAME:
      // Deal with non-array variables here. Can also be done statically
      break;
    case ADDR_EXPR: 
    case MEM_REF: 
      zero_index = build_int_cst(integer_type_node, 0);
      insert_stock_fn(gsi, TREE_OPERAND(op,0), zero_index, io_type);
      break;
    case ARRAY_REF:
      insert_stock_fn(gsi,
		      TREE_OPERAND(TREE_OPERAND(op,0),0),
		      TREE_OPERAND(op,1),
		      io_type);
      break;
    default: break;
    } // end switch: TREE_CODE

  return;
}

// Note: don't really need to pass the stmt if you're already passing gsi
// Also, prev_stmt is not really needed at all anymore
void analyze_stmt(gimple stmt, gimple prev_stmt, gimple_stmt_iterator &gsi) {
  tree op;
  size_t i;

  if ( is_gimple_assign(stmt) ) {
    for ( i = 0 ; i < gimple_num_ops(stmt) ; ++i ) {
      op = gimple_op(stmt, i);
      if ( op )
	analyze_gimple_op(gsi, op, i ? RD : WR);
    } // end for: ops
  } // end if: assign stmt
    
  return;
}

void find_vars(basic_block bb) {
  gimple_stmt_iterator gsi;
  gimple stmt, prev_stmt;

  gsi = gsi_start_bb(bb);
  while ( gsi.ptr != (gsi_last_bb(bb)).ptr ) {
    stmt = gsi_stmt(gsi);
    analyze_stmt(stmt, prev_stmt, gsi);
    gsi_next(&gsi);
  } // end while: inside basic block
  gsi = gsi_last_bb(bb);
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

      instr_args.push_back(current_string);
      arg_used.push_back(false);

      // ideally recover information from here for better feedback
      //line_no.push_back(0);
      //file_name.push_back(NULL);
      
      tmpType = pragma_lex (&tmpTree);
      while ( tmpType == CPP_COMMA)
	tmpType = pragma_lex (&tmpTree);
    } // end while: possible function names being read

    if ( need_close_paren ) {
      if ( tmpType == CPP_CLOSE_PAREN )
	tmpType = pragma_lex (&tmpTree);
      else
	warning (OPT_Wpragmas,
		 "%<#pragma instrument function (string [,string]...)%> "
		 "does not have final %<)%>");
    } // end if: seeking closing paren
  } // end if: valid syntax
  
  check_instr_args_for_doubles();
  return;
}

static void register_vector_pragmas(void *even_data, void *data) {
  //printf("[pragma] Registering vectorization pragmas\n");
  c_register_pragma("MIHPS", "vcheck", vcheck_pragma_handler);
}

/******* End pragma handling ********/

/****** Begin new pass *********************/
void analyze_fn_loops(struct function *fn) {
  const char *fn_name = fndecl_name(fn->decl);

  struct loop *cLoop = get_loop(fn, 0);
  FOR_EACH_LOOP(cLoop, 0) {
    if ( !cLoop->inner ) {
      basic_block *body = get_loop_body(cLoop);
      unsigned nBlocks = cLoop->num_nodes;
      for ( unsigned i = 0 ; i < nBlocks ; ++i )
	find_vars(body[i]);

      basic_block loop_start = loop_preheader_edge(cLoop)->dest;
      gimple_stmt_iterator gsi_start = gsi_start_bb(loop_start);
      insert_alloc_function(gsi_start);

      vec<edge> all_exits = get_loop_exit_edges (cLoop);
      for ( unsigned i = 0 ; i < all_exits.length() ; ++i ) {
	edge post_edge = all_exits[i];
	basic_block post_block = post_edge->dest;
	gimple_stmt_iterator gsi_post = gsi_start_bb(post_block);
	insert_post_loop_treatment(gsi_post);
      } // end for: all exits of loop
    } // end if: inner loop
  } // end for: all loop

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
    if ( function_to_check(current_function) )
      return true;
    else
      return false; 
  }

  unsigned int execute() {
    analyze_fn_loops(cfun);

    return 0;
  }

  analysis_pass* clone() { return new analysis_pass(g); }
};

/******** End new pass **********/


int plugin_init (struct plugin_name_args *plugin_info,
		 struct plugin_gcc_version *version ) {
  printf("-----------------------------------------------\n");
  printf("           Initializing plugin                 \n");
  printf("-----------------------------------------------\n");

  register_callback(plugin_info->base_name,
		    PLUGIN_PRAGMAS,
		    register_vector_pragmas,
		    NULL);

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


  register_callback(plugin_info->base_name,
		    PLUGIN_FINISH,
		    test_if_all_used,
		    NULL);

  return 0;
}
