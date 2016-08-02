#BasicDSP 2.0 
## Grammar Definition

program &rarr; statement-list _EOF_  

statement-list &rarr; statement statement-list  
statement-list &rarr; _EMPTY_    
  
statement &rarr; assignment (LF | CR | SEMICOLON)  

assignment &rarr; IDENT '=' expr

expr &rarr; expr + term  
expr &rarr; expr - term  
expr &rarr; term  
  
term &rarr; term * factor  
term &rarr; term / factor   
term &rarr; factor

factor &rarr; FUNCTION '(' expr ')'  
factor &rarr; '(' expr ')'      
factor &rarr; - factor  
factor &rarr; INTEGER    
factor &rarr; FLOAT  
factor &rarr; IDENT  


## Grammar with left recursion removed

program &rarr; statement-list _EOF_  

statement-list &rarr; statement statement-list  
statement-list &rarr; _EMPTY_    
  
statement &rarr; assignment (LF | CR | SEMICOLON)  

assignment &rarr; IDENT '=' expr

expr &rarr; term expr'  
expr' &rarr; - term expr' | + term expr' | _e_  
  
term &rarr; factor term'  
term' &rarr; * factor term' | / factor term' | _e_  

factor &rarr; FUNCTION '(' expr ')'  
factor &rarr; '(' expr ')'      
factor &rarr; - factor  
factor &rarr; INTEGER    
factor &rarr; FLOAT  
factor &rarr; IDENT  

## more information about grammars

[Compiler patterns](http://www.codeproject.com/Articles/286121/Compiler-Patterns)

