/**
 * This is an example minimal parser in just flex for the grammar of Shell part 1.
 * 
 * This does not contain any logic for executing the commands, but it should serve
 * as a basis on top of which you can easily continue. Throughout the file, we have
 * included some comments and instructions on how to expand.
 * 
 * Floris Westerman, 2022
 */

/* Here we enter a section that is copied verbatim to the output */
%{
// Headers for use in this file
using namespace std;

#include "functions.h"
extern "C" int yylex();

vector<vector<string>> chains;
vector<string> command;
vector<string> operators;

//////////// Here you can put some helper functions and code, but make sure to properly
//////////// separate your code in logical "entities" in different files! This helps
//////////// us grade your code as well.

%}

/**
 * Parsing in flex is done based on a series of regexes. Below, we list these regexes
 * in order, flex will try to match the input with these in order. As soon as any regex
 * matches, it will run the code associated with it. Because of this order, we should
 * put our more specific regexes at the top: put a match for "exit" before a general
 * string regex.
 *
 * Optionally, you can specify a "start condition" (you can see it as some sort of
 * context) for a rule. This causes the regex to only be tried when the parsing is
 * in this context. You can control the current context using BEGIN(). The default
 * start condition is "INITIAL". So below, as soon as we hit a " char, we enter the
 * "STRING" context that will enable us to match any character until the next ",
 * after which we return to the "INITIAL" context.
 *
 * You could decide to hook up Bison to this part as well, as can be seen in many
 * tutorials online. Then you would store/return tokens from the actions in these rules.
 * However, for the scope of this simple grammar for the shell, that is really not
 * necessary and a few state variables should suffice to get the functionality you need!
 */

/* Here we inform flex that we have two additional "start conditions", besides INITIAL */
%x string error

/* Here we inform flex to not "look ahead" in stdin beyond what is necessary, to prevent
 * issues with passing stdin to another executable. */
%option always-interactive

%%
    /* From here on, comments must be indented! */

    /* Here we start with the rules. The highest priority rules are those to accept any
     * symbol when we are reading a string (so in the STRING context). */
    
    /* The first rule is to match the closing " char */
<string>"\""        BEGIN(INITIAL); /* Return to normal parsing */

    /* The second rule takes the longest string of characters not being " */
<string>[^\"]+      {
                        /* Here we match any entire string. We should either make this
                         * the command to execute, or store this as an option, or it is
                         * a filename, depending on the current state! */

                        // Entire string value is available in yytext with length yyleng
                        // Make sure to copy it!
                        //printf("STRING: %s\n", yytext);

                        command.push_back(yytext);
                    }
<string><<EOF>>     {
                      printf("Error: invalid syntax!\n");
                      command.clear();
                      BEGIN(INITIAL);
                    }

    /* Here we have cases for error handling - when we have encountered an invalid syntax,
     * we want to discard the entire line so we continue until the next newline symbol. */
<error>\n           BEGIN(INITIAL); /* Return to normal parsing */
<error>.

    /* From here on, we have only "normal" rules for our parsing */

    /* Built-in commands */
"exit"              {
                        /* For built-in command names, make sure that we are not currently
                         * parsing options or some other place where we cannot reasonably
                         * come across an actual "exit" command. So only perform some action
                         * here if you are sure that it is safe and valid to do so, otherwise
                         * use REJECT to let flex continue to the next match (as a string,
                         * later on). */
                        
                        exitFlag = 0;

                        command.push_back("exit");
                        chains.push_back(command);
                        prepareForExec(chains, operators);
                        command.clear();
                        operators.clear();
                        chains.clear();

                        if (exitFlag == 1) {
                            yyterminate();
                        } 
                    }
"cd"                {
                        command.push_back("cd");
                    } 
    /* Other grammar parts */
"\""                BEGIN(string); /* We start reading a string until the next " char */
"&&"                {
                        chains.push_back(command);
                        operators.push_back("&&");
                        command.clear();
                    }

"&"                 {
                        chains.push_back(command);
                        operators.push_back("&");
                        command.clear();
                    }   

"||"                {
                        chains.push_back(command);
                        operators.push_back("||");
                        command.clear();
                    }

"|"                 {
                        chains.push_back(command);
                        operators.push_back("|");
                        command.clear();
                    }

;                   { 

                        chains.push_back(command);
                        operators.push_back(";");
                        //printf("executare din ;\n");
                        prepareForExec(chains, operators);
                        command.clear();
                        operators.clear();
                        chains.clear();
                    }

\n                  {
                        if(!command.empty()) {
                            chains.push_back(command);
                            operators.push_back("\n");
                            //printf("executare din newline\n");
                            prepareForExec(chains, operators);
                            command.clear();
                            chains.clear();
                            operators.clear();
                        }

                        if(chains.size() != 0 && chains.size() == operators.size() && operators.back().compare("&") == 0) {   
                            BEGIN(INITIAL);
                        }
                    }
[ \t]+              /* Ignore whitespace */
[^ ;\t\n\"\^|]+     {
                        /* Here we match any sequence of characters without whitespace as a
                         * "word" or so. We should either make this the command to execute,
                         * or store this as an option, or it is a filename, depending on the
                         * current state! */

                        command.push_back(yytext);
                        
                    }
<<EOF>>             {
                        if(!command.empty()) {
                            // cout << "adaugam comanda asta : \n";
                            // for (int i = 0; i < command.size(); i++) {
                            //     cout << command[i] << " ";
                            // }
                            // cout << "\n";
                            chains.push_back(command);
                            //printf("executare din EOF\n");
                            prepareForExec(chains, operators);
                            command.clear();
                            chains.clear();
                            operators.clear();
                        }
                        /* At EOF we should unconditionally terminate! */
                        yyterminate();
                    }
.                   {
                        /* Error: unknown character! (probably doesn't happen) */
                        fprintf(stdout, "Unrecognized character: %s\n", yytext );
                        BEGIN(error);
                    }

%%

/* All code after the second pair of %% is just plain C where you typically
 * write your main function and such. */

int main() {
    // Initialize program
    setbuf(stdin, NULL);
    setbuf(stdout, NULL);

    // Set up some state

    // Start parsing process
    yylex();

    // for (int i = 0; i < chains.size(); i++) {
    //     for (int j = 0; j < chains[i].size(); j++){
    //         cout << chains[i][j] << ", ";
    //         if (j == chains[i].size() - 1)
    //             cout << "|";
    //     }
    // }
    // cout << "\n";

    // for (int i = 0; i < operators.size(); i++) {
    //     cout << operators[i] << " ";
    // }   

    //prepareForExec(chains, operators);

    // Cleanup
    fclose(yyin);
    yylex_destroy();

    return EXIT_SUCCESS;
}