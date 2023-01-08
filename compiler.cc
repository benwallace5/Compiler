#include <iostream>
#include "lexer.h"
#include "execute.h"
#include <string>
#include <map>

using namespace std;

//var declaration
LexicalAnalyzer mylexer;
map<string, int> table;

//function declaration
InstructionNode* parse_body();
InstructionNode* parse_stmt_list();
void parse_var_section();
void parse_inputs();
void parse_id_list();
int location(string);
int parse_primary();
InstructionNode* parse_stmt();
InstructionNode* parse_assign_stmt();
InstructionNode* parse_if_stmt();
InstructionNode* parse_for_stmt();
InstructionNode* parse_while_stmt();
InstructionNode* parse_switch_stmt();
InstructionNode* parse_output_stmt();
InstructionNode* parse_input_stmt();
InstructionNode* parse_condition();
InstructionNode* parse_switch_stmt();
InstructionNode* parse_default_case();
InstructionNode* parse_case_list(string);
InstructionNode* parse_for_stmt();


InstructionNode* parse_generate_intermediate_representation(){

  InstructionNode* head = new InstructionNode();

  parse_var_section();

  head = parse_body();

  parse_inputs();

  return head;
  
}
  
void parse_var_section(){

  parse_id_list();

  mylexer.GetToken(); //Consumes SEMICOLON in var_section -> id_list SEMICOLON
}

//parse_id_list() -> ID COMMA id_list | ID
void parse_id_list(){

  Token t1, t2;
  string id;
  
  t1 = mylexer.peek(1);
  t2 = mylexer.peek(2);

  if((t1.token_type == ID) && (t2.token_type == COMMA)){
    t1 = mylexer.GetToken(); // Consumes ID
    mylexer.GetToken(); // Consumes COMMA
    id = t1.lexeme;

    mem[next_available] = 0;
    table.insert({id, next_available});
    next_available++;

    parse_id_list();
  }else{
    t1 = mylexer.GetToken(); //id_list -> ID
    id = t1.lexeme;
    
    mem[next_available] = 0;
    table.insert({id, next_available});
    next_available++;
  }
}

//body -> LBRACE stmt_list RBRACE
InstructionNode* parse_body(){
  struct InstructionNode* stmtList = new InstructionNode();
  
  mylexer.GetToken(); // LBRACE

  stmtList = parse_stmt_list();

  mylexer.GetToken(); //RBRACE

  return stmtList;
  
}
  
InstructionNode* parse_stmt_list(){
  InstructionNode *stmt = new InstructionNode();
  InstructionNode *stmtList = new InstructionNode();
  InstructionNode *temp = new InstructionNode();  
  Token t;

  stmt = parse_stmt();
  t = mylexer.peek(1);

  if((t.token_type == ID) || (t.token_type == WHILE) || (t.token_type == IF) || (t.token_type == SWITCH) || (t.token_type == OUTPUT) || (t.token_type == INPUT) || (t.token_type == FOR)) {// stmt_list -> stmt stmt_list

    stmtList = parse_stmt_list();

    temp = stmt;

    while(temp->next != nullptr){
      
      if((temp->type == CJMP) && (temp->next->type == NOOP)) //Case inst
	{
	  temp = temp->cjmp_inst.target; //body of cases, since cjmp->next = end of case body, target = next
	}
      else
	{
	  temp = temp->next;
	}
    }

    temp->next = stmtList;
  }

  return stmt;
  
}


InstructionNode* parse_stmt(){
  Token t;
  InstructionNode *stmt = new InstructionNode();
  
  t = mylexer.peek(1);

  if(t.token_type == ID){
    
  stmt = parse_assign_stmt();
    
  }else if(t.token_type == WHILE){

    stmt = parse_while_stmt();
    
  }else if(t.token_type == SWITCH){

    stmt = parse_switch_stmt();
    
  }else if(t.token_type == OUTPUT){

    stmt = parse_output_stmt();
    
  }else if(t.token_type == IF){

    stmt = parse_if_stmt();
    
  }else if(t.token_type == INPUT){

    stmt = parse_input_stmt();

  }else if(t.token_type == FOR){

    stmt = parse_for_stmt();
    
  }

  return stmt;
}

//Case 1: assign_stmt -> ID EQUAL primary SEMICOLON
//primary -> ID | NUM
//Case 2: assign_stmt -> ID EQUAL expr SEMICOLON
//expr ->primary op primary
InstructionNode* parse_assign_stmt(){
  Token lhsID;
  Token t2;
  int op1_index;
  int op2_index;
  InstructionNode* stmt = new InstructionNode();

  lhsID = mylexer.GetToken(); //ID
  mylexer.GetToken(); //EQUAL
  t2 = mylexer.peek(2);

  if(t2.token_type == SEMICOLON){//Case 1

    op1_index = parse_primary();
    
    stmt->type = ASSIGN;
    stmt->assign_inst.operand1_index = op1_index;
    stmt->assign_inst.left_hand_side_index = location(lhsID.lexeme);
    stmt->assign_inst.op = OPERATOR_NONE;
    
  }else {

    op1_index = parse_primary();
    t2 = mylexer.GetToken(); //operator
    
    if(t2.token_type == PLUS){

      stmt->assign_inst.op = OPERATOR_PLUS;
      
    }else if(t2.token_type == MINUS){

      stmt->assign_inst.op = OPERATOR_MINUS;      

    }else if(t2.token_type == DIV){

      stmt->assign_inst.op = OPERATOR_DIV;      
      
    }else if(t2.token_type == MULT){

      stmt->assign_inst.op = OPERATOR_MULT;      

    }

    op2_index = parse_primary();
    
    stmt->type = ASSIGN;    
    stmt->assign_inst.operand1_index = op1_index;
    stmt->assign_inst.operand2_index = op2_index;    
    stmt->assign_inst.left_hand_side_index = location(lhsID.lexeme);

  }

  mylexer.GetToken(); //SEMICOLON
  return stmt;
}

int location(string a){
  int index;
  auto i = table.find(a);

  if(i != table.end()){ //FOUND

    index = i->second;
    
  }else{

    table.insert({a,next_available});
    index = next_available;
    mem[next_available] = 0;
    next_available++;
    
  }

  return index;
}

int parse_primary(){
  Token t;
  int index, value;
  InstructionNode* temp = new InstructionNode();

  t = mylexer.GetToken();

  if(t.token_type == ID){

    index = location(t.lexeme);

  }else{

    value = stoi(t.lexeme);
    mem[next_available] = value;
    index = next_available;
    next_available++;
    
  }

  return index;
}

InstructionNode* parse_input_stmt(){
  Token t;
  InstructionNode* stmt = new InstructionNode();

  mylexer.GetToken(); //input
  t = mylexer.GetToken(); //ID
  mylexer.GetToken(); //SEMICOLON

  stmt->type = IN;
  stmt->input_inst.var_index = location(t.lexeme);
  stmt->next = nullptr;

  return stmt;
}

InstructionNode* parse_output_stmt(){
  Token t;
  InstructionNode* stmt = new InstructionNode();

  mylexer.GetToken();//output
  t = mylexer.GetToken();//ID
  mylexer.GetToken(); //SEMICOLON

  stmt->type = OUT;
  stmt->output_inst.var_index = location(t.lexeme);
  stmt->next = nullptr;
  
  return stmt;
}

//while_stmt -> WHILE condition body
InstructionNode* parse_while_stmt(){
  Token t;
  InstructionNode* stmt = new InstructionNode();
  InstructionNode* temp = new InstructionNode();
  InstructionNode* jump = new InstructionNode();  
  InstructionNode* noop = new InstructionNode();
  
  mylexer.GetToken(); //WHILE

  temp = parse_condition();//condition

  stmt->type = CJMP;//initializing fields for the while condition
  stmt->next = nullptr;
  stmt->cjmp_inst.condition_op = temp->cjmp_inst.condition_op;
  stmt->cjmp_inst.operand1_index = temp->cjmp_inst.operand1_index;
  stmt->cjmp_inst.operand2_index = temp->cjmp_inst.operand2_index;
  
  noop->type = NOOP;//initializing noop
  noop->next = nullptr;

  stmt->next = parse_body();//While body

  jump->type = JMP; //creating a JMP stmt with ->next = noop
  jump->jmp_inst.target = stmt;
  jump->next = noop;

  temp = stmt; //setting equal to head of while body

  while(temp->next != nullptr){//traversing while body until we hit last element, a->next = nullptr

    //    temp = temp->next;
    if((temp->type == CJMP) && (temp->next->type == NOOP)) //Case inst
      {
	temp = temp->cjmp_inst.target; //body of cases, since cjmp->next = end of case body, target = next
      }
    else
      {
	temp = temp->next;
      }
  }

  temp->next = jump; //appending jump to end of while body, which is pointed at by temp
  stmt->cjmp_inst.target = noop; //where to break if condition fails

  return stmt;
}

//if -> condition body
InstructionNode* parse_if_stmt(){//LEFT OFF NEEDING TO TEST THIS FUNCTION
  InstructionNode* stmt = new InstructionNode();
  InstructionNode* temp = new InstructionNode();
  InstructionNode* noop = new InstructionNode();  
  Token t;

  mylexer.GetToken();// IF

  temp = parse_condition(); // condition

  stmt->type = CJMP;
  stmt->cjmp_inst.condition_op = temp->cjmp_inst.condition_op;
  stmt->cjmp_inst.operand1_index = temp->cjmp_inst.operand1_index;
  stmt->cjmp_inst.operand2_index = temp->cjmp_inst.operand2_index;

  stmt->next = parse_body();
  
  noop->type = NOOP;
  noop->next = nullptr;

  temp = stmt->next;

  while(temp->next != nullptr)
    {
      if((temp->type == CJMP) && (temp->next->type == NOOP)) //Case inst
	{
	  temp = temp->cjmp_inst.target; //body of cases, since cjmp->next = end of case body, target = next
	}
      else
	{
	  temp = temp->next;
	}
    }

  temp->next = noop;
  stmt->cjmp_inst.target = noop;

  return stmt;
}

void parse_inputs(){
  Token t;
  int value;

  t = mylexer.GetToken(); //NUM
  value = stoi(t.lexeme);
  inputs.push_back(value);
  t = mylexer.peek(1);

  if(t.token_type == NUM)
    {
      parse_inputs();
    }
}

InstructionNode* parse_condition(){
  int index1, index2;
  Token t;
  InstructionNode* stmt = new InstructionNode();

  index1 = parse_primary();
  t = mylexer.GetToken(); // relop

  if(t.token_type == GREATER){
    
    stmt->cjmp_inst.condition_op = CONDITION_GREATER;
    
  }else if(t.token_type == LESS){
    
    stmt->cjmp_inst.condition_op = CONDITION_LESS;
      
  }else if(t.token_type == NOTEQUAL){
    
    stmt->cjmp_inst.condition_op = CONDITION_NOTEQUAL;
    
  }
  index2 = parse_primary();
  
  stmt->type = CJMP;
  stmt->cjmp_inst.operand1_index = index1;
  stmt->cjmp_inst.operand2_index = index2;
  stmt->cjmp_inst.target = nullptr;
  
  return stmt;
}

//switch_stmt -> SWITCH ID LBRACE case_list RBRACE
//switch_stmt -> SWITCH ID LBRACE case_list default_case RBRACE
InstructionNode* parse_switch_stmt(){
  InstructionNode* default_case = new InstructionNode();
  InstructionNode* caseList = new InstructionNode();
  InstructionNode* temp = new InstructionNode();    
  Token t, t1;

  mylexer.GetToken(); //SWITCH
  t = mylexer.GetToken(); //ID
  mylexer.GetToken(); //LBRACE
  t1 = mylexer.peek(1);

  if(t1.token_type == CASE)
    {
      caseList = parse_case_list(t.lexeme); //getting case_list
      t1 = mylexer.peek(1);
      
      if(t1.token_type == DEFAULT)
	{
	  default_case = parse_default_case();
      
	  //append default_case to caseList
	  temp = caseList;  //head of switch_stmt
      
	  //appending noop2 to end of switch stmt     
	  while(temp->next->next != nullptr){  //traversing switch stmt
    
	    if((temp->type == CJMP) && (temp->next->type == NOOP)) //Case inst
	      {
		temp = temp->cjmp_inst.target; //body of cases, since cjmp->next = end of case body, target = next
	      }
	    else
	      {
		temp = temp->next;
	      }
	  }
	  temp->next = default_case; // temp->next points to second to last noop at end of list. This inserts it at end of case list, before label
	}
    }
  else if(t1.token_type == DEFAULT) //SWITCH -> SWITCH ID LBRACE default_case RBRACE
    {
      caseList = parse_default_case();
    }

    
  //Consumed SWITCH ID LBRACE case_list
  //Need to check if default_case exists

  mylexer.GetToken(); //RBRACE
  
  return caseList;
}

InstructionNode* parse_case_list(string id){
  
  InstructionNode* cases = new InstructionNode();
  InstructionNode* caseList = new InstructionNode();
  InstructionNode* noop1 = new InstructionNode();
  InstructionNode* noop2 = new InstructionNode();  
  InstructionNode* jmp = new InstructionNode();
  InstructionNode* cjmp = new InstructionNode();
  InstructionNode* temp = new InstructionNode();        
  Token t;
  int val;
  int op2_index;

  //initializing noop1 and noop2 nodes
  //noop1 = cases(cjmp).target
  //noop2 = jmp.target
  //noop2 = noop1.next
  noop1->type = NOOP;
  noop1->next = nullptr;
  noop2->type = NOOP;
  noop2->next = nullptr;
  
  //parse case
  mylexer.GetToken(); // CASE
  op2_index = parse_primary(); // NUM
  mylexer.GetToken(); // COLON
  cases = parse_body(); //body of case[i]
  
  //traversing body stmtlist to last stmt
  temp = cases;//head of case body

  while(temp->next != nullptr){

    temp = temp->next;

  }

  jmp->type = JMP; //initializing jmp inst
  jmp->jmp_inst.target = nullptr;
  temp->next = jmp;//if the condition is satisfied, jmp will be at the end of body, pointing outside switch statement.
  jmp->next = noop1;
  cjmp->type = CJMP;
  cjmp->cjmp_inst.condition_op = CONDITION_NOTEQUAL;
  cjmp->cjmp_inst.operand1_index = location(id);
  cjmp->cjmp_inst.operand2_index = op2_index;
  cjmp->cjmp_inst.target = cases; //if not notequal aka equal, move to case body
  cjmp->next = noop1; //otherwise, if notequal skip case body to noop, with noop->next = next case condition
  
  //check if more cases following
  t = mylexer.peek(1);

  if(t.token_type == CASE){

    caseList = parse_case_list(id);//case_list

    temp = cjmp; // head of case

    while(temp ->next != nullptr){//traversing case condition and body to last element

      if((temp->type == CJMP) && (temp->next->type == NOOP)){//Looking for cjmp that are the case statements
	temp = temp->cjmp_inst.target; //body of cases, since cjmp->next = end of case body, target = head of case body
    }
    else{
      temp = temp->next;
    }
    }

    temp->next = caseList; //appending caseList to cases
    
  }

  //need to add noop2 to end of switch stmt and set each case body's last stmt,jmp.target, equal to noop2

  temp = cjmp;//head of case
  //appending noop2 to end of switch stmt
  while(temp->next != nullptr){//traversing switch stmt
    
    if((temp->type == CJMP) && (temp->next->type == NOOP)) //Case inst
      {
	temp = temp->cjmp_inst.target; //body of cases, since cjmp->next = end of case body, target = next
      }
    else
      {
      temp = temp->next;
      }
  }

  temp->next = noop2; //appending noop2 to switch_stmt;

  temp = cjmp; //head of case
  //each jmp needs to have target = noop2
  while(temp->next != nullptr){

    if((temp->type == JMP) && (temp->next->type == NOOP) && (temp->jmp_inst.target == nullptr)){
      
      temp->jmp_inst.target = noop2;// each case body ends with jmp followed by noop. jmp needs to point to outside of switch to exit
      temp = temp->next;
      }
    else if((temp->type == CJMP) && (temp->next->type == NOOP))
      {
	temp = temp->cjmp_inst.target; //body of cases, since cjmp->next = end of case body, target = next
      }
    else{
      temp = temp->next;
    }
  }

  return cjmp;
}

//default_case -> DEFAULT COLON body
InstructionNode* parse_default_case()
{
  InstructionNode* default_case = new InstructionNode();
  Token t;

  mylexer.GetToken(); //DEFAULT
  mylexer.GetToken(); //COLON
  default_case = parse_body();

  return default_case;
}

//for_stmt -> FOR LPAREN assign_stmt condition SEMICOLON assign_stmt RPAREN body
InstructionNode* parse_for_stmt()
{
  Token t;
  InstructionNode *assign_stmt1 = new InstructionNode();
  InstructionNode *assign_stmt2 = new InstructionNode();
  InstructionNode *body = new InstructionNode();
  InstructionNode *noop = new InstructionNode();
  InstructionNode *jmp = new InstructionNode();
  InstructionNode *cjmp = new InstructionNode();    
  InstructionNode *condition = new InstructionNode();
  InstructionNode *temp = new InstructionNode();
  
  jmp->type = JMP;
  noop->type = NOOP;
  
  
  mylexer.GetToken(); //FOR
  mylexer.GetToken(); //LPAREN
  assign_stmt1 = parse_assign_stmt();
  
  condition = parse_condition();//condition contains while condition information
  cjmp->type = CJMP;
  cjmp->cjmp_inst.condition_op = condition->cjmp_inst.condition_op;
  cjmp->cjmp_inst.operand1_index = condition->cjmp_inst.operand1_index;
  cjmp->cjmp_inst.operand2_index = condition->cjmp_inst.operand2_index;
  
  mylexer.GetToken(); //SEMICOLON
  
  assign_stmt2 = parse_assign_stmt(); //assign stmt2
  mylexer.GetToken(); //RPAREN 
  body = parse_body();//body

  assign_stmt1->next = cjmp; //assign_stmt1->cjmp->body->assign_stmt2->jmp->noop
  cjmp->next = body;
  assign_stmt2->next = jmp;
  jmp->next = noop;

  //traverse to end of while body in order to append assign_stmt2 to it
  temp = body; // head of while body
  
  while(temp->next != nullptr)//traversing switch stmt to second to last element
    {    
    if((temp->type == CJMP) && (temp->next->type == NOOP)) //Case inst
      {
	temp = temp->cjmp_inst.target; //body of cases, since cjmp->next = end of case body, target = next
      }
    else
      {
	temp = temp->next;
      }
  }
  temp->next = assign_stmt2;

  jmp->jmp_inst.target = cjmp; //jump at end of while loop body. Jumps back to while condition. ->next = noop
  cjmp->cjmp_inst.target = noop; //if while condition fails, jump to noop at the end of while body

  return assign_stmt1;
}
