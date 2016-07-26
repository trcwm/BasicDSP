#BasicDSP 2.0 
## Grammar Definition

program &rarr; statement-list _EOF_  

statement-list &rarr; statement statement-list  
statement-list &rarr; _EMPTY_    
  
statement &rarr; assignment (LF | CR | SEMICOLON)  

assignment &rarr; IDENT '=' expr

expr &rarr; term + expr  
expr &rarr; term - expr  
expr &rarr; term  

term &rarr; '-' factor  
term &rarr; factor * term   
term &rarr; factor

factor &rarr; FUNCTION '(' expr ')'
factor &rarr; '(' expr ')'    
factor &rarr; INTEGER  
factor &rarr; FLOAT  
factor &rarr; IDENT  
