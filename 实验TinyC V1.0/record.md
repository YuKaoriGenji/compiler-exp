1.Correct the project:
    (1) Cannot compile and had some unknown characters:
        Pristine engineering project did not suit Linux and did not contain the symbol '\r'
        Adding the symbol '\r' in LexicalAnalysis.c as

         else if (c==' ' || c=='\t' || c=='\n'||c=='\r')

    (2) Cannot support Add-operation between instant numbers like 1+2:
        The design of Lexical Analysis Automachine is wrong and it just combine +/- with the following number

        change the LexTable[3][digit] from 4 into 103 and induce it into a terminal when +/e encounters number.

    (3) Cannot support comparative operation such as "i==j", "i<=j" etc.:
        if we turn the AnaTypeLex option on, we can notice the problem that Lex:15 follow with Lex:27, which indicate that the Lexical Analysis with symbol '==' might be wrong.
        So we can take a glimpse at LexicalAnalysis, suddenly we found that if we do not clean the variable prebuf, it would remain '=' as the next token the automachine read would be '=', so we just change code as:

        else if (tokenStr[0]=='=' && tokenStr[1]=='=') {prebuf=0;  return(SYN_EQ);}
        And all of those symbol with two charictars would have the same alteration.

    (4) Cannot support "WHILE":
        It is the problem that token while had been cast off after the file pointer come back behind while().

        We can just mark the position behind while and before ( which means sFile-1. Then constrain the token to be SYN_WHILE after one of the poccess terminated.

        file_index=ftell(sFile)-1;
        lookahead.token=SYN_WHILE; in if(run_status==1)

    (5) Do not support Logical operation, for instance "a&&b".:
        With the AnaTypeLex option on, the fact that symbol "&&" followed with nothing had been revealed. So we can just look at the Lexical Analysis.
        Just like case (3), put a "prebuf=0" before return operation.

2.Adding Char Type:
    Change the definition of tokenval with a new character item.
    add two new type in Lexical Analysis
    I have changed the Lexical automachine with a new item. 0->quatation->6->any->7->quatatuin->b6
    Add a new function to change string into char
    In 206 case, add the value to the token.
    in SyntaxAnalysis, I just change Prod_F with a new case called char and altered something in match function.
    You can read the code or Explanation in Notability of my Ipad for details.

3.Adding Logical Expression:
    Step1:  Change all int return value into EXPVAL in order to place B and above at the same value with E, therefore B and above all can be calculated. Simultaneously change the code of every functions for the sake of suiting the EXPVAL-model environment.
    Step2:  Adding three previous syntax law named K1,K2,K3,K,which were totally copied from the law of E1,TE,TE1 and E. The purpose of the step is to add operator to logical expression.

4.Adding break command:
    Lexical: Add break to function FoundKeyword:
         if (strcompare(tokenStr,"break")) {return(SYN_BREAK);}
    Syntax: Adding a new column SYN:S-->break;
            Adding some situation varibles to indicate whether it is in a loop or not. Use the number of is_loop to indicate the number of loop
            If is_break=1, skip the next Prod_S.
            Code is the best

5.Adding continue command:
    a.found a bug: if IF is in a while, then the while would never end. This is because the end of if has changed the run_status into 1.
    method to solve the problem is to use a tmp_status to save the initial status when enter a IF.
    it is quite same like break, but not to jump out. So it need to add a new situation when run_status=2 and is_continue=1;
    
