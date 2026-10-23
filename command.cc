
/*
 * 
 *
 * Template file.
 * You will need to add more code here to execute the command table.
 *
 * NOTE: You are responsible for fixing any bugs this code may have!
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>
#include <glob.h>
#include <wordexp.h>
#include <vector>
#include <string>
#include <cctype>
#include <iostream>
#include <sstream>
#include <fstream>
#include "command.h"
#include "tokenizer.h"


void parse(std::vector<Token> &tokens)
{
    Command &command = Command::_currentCommand;
    SimpleCommand *&simplecommand = Command::_currentSimpleCommand;
    simplecommand = new SimpleCommand();

    for (size_t i = 0; i < tokens.size(); i++)
    {
        Token &token = tokens[i];

        switch (token.type)
        {
            case TOKEN_COMMAND:
            case TOKEN_ARGUMENT:
                simplecommand->insertArgument(strdup(token.value.c_str()));
                break;
            case TOKEN_PIPE:
                command.insertSimpleCommand(simplecommand);
                simplecommand = new SimpleCommand();
                break;
            case TOKEN_REDIRECT:
                if (i + 1 >= tokens.size() || tokens[i+1].type != TOKEN_ARGUMENT && tokens[i+1].type != TOKEN_COMMAND) {
                    fprintf(stderr,"ERROR: INVALID TOKEN.\n");
                    command.insertSimpleCommand(simplecommand);
                    command.clear();
                    return;
                }
                else {
                    if(command._outFile != nullptr) free(command._outFile);
                    command._outFile = strdup(tokens[i+1].value.c_str());
                    command._append = 0;
                    i++;
                    break;
                }
            case TOKEN_APPEND:
                if (i + 1 >= tokens.size() || tokens[i+1].type != TOKEN_ARGUMENT && tokens[i+1].type != TOKEN_COMMAND) {
                    fprintf(stderr,"ERROR: INVALID TOKEN.\n");
                    command.insertSimpleCommand(simplecommand);
                    command.clear();
                    return;
                }
                else {
                    if(command._outFile != nullptr) free(command._outFile);
                    command._outFile = strdup(tokens[i+1].value.c_str());
                    command._append = 1;
                    i++;
                    break;
                }
            case TOKEN_INPUT:
                if (i + 1 >= tokens.size() || tokens[i+1].type != TOKEN_ARGUMENT && tokens[i+1].type != TOKEN_COMMAND) {
                    fprintf(stderr,"ERROR: INVALID TOKEN.\n");
                    command.insertSimpleCommand(simplecommand);
                    command.clear();
                    return;
                }
                else {
                    if (command._inputFile != nullptr) free(command._inputFile);
                    command._inputFile = strdup(tokens[i+1].value.c_str());
                    i++;
                    break;
                }
            case TOKEN_ERROR:
                if (i + 1 >= tokens.size() || tokens[i+1].type != TOKEN_ARGUMENT && tokens[i+1].type != TOKEN_COMMAND) {
                    fprintf(stderr,"ERROR: INVALID TOKEN.\n");
                    command.insertSimpleCommand(simplecommand);
                    command.clear();
                    return;
                }
                else {
                    if (command._errFile != nullptr) free(command._errFile);
                    command._errFile = strdup(tokens[i+1].value.c_str());
                    i++;
                    break;
                }
            case TOKEN_REDIRECT_AND_ERROR:
                if (i + 1 >= tokens.size() || tokens[i+1].type != TOKEN_ARGUMENT && tokens[i+1].type != TOKEN_COMMAND) {
                    fprintf(stderr,"ERROR: INVALID TOKEN.\n");
                    command.insertSimpleCommand(simplecommand);
                    command.clear();
                    return;
                }
                else {
                    if (command._outFile != nullptr) free(command._outFile);
                    if (command._errFile != nullptr) free(command._errFile);
                    command._errFile = strdup(tokens[i+1].value.c_str());
                    command._outFile = strdup(tokens[i+1].value.c_str());
                    i++;
                    command._out_error = 1;
                    command._append = 1;
                    break;
                }
            case TOKEN_BACKGROUND:
                command._background = 1;
                break;
            case TOKEN_EOF:
                if (simplecommand->_numberOfArguments > 0)
                {
                    command.insertSimpleCommand(simplecommand);
                }
                else
                {
                    free(simplecommand->_arguments);
                    delete simplecommand;
                }
                simplecommand = nullptr;
                break;
        }
    }
    command.execute();
}

SimpleCommand::SimpleCommand()
{
    _numberOfAvailableArguments = 5;
    _numberOfArguments = 0;
    _arguments = (char **)malloc(_numberOfAvailableArguments * sizeof(char *));
}

void SimpleCommand::insertArgument(char *argument)
{
    if (_numberOfAvailableArguments == _numberOfArguments + 1)
    {
        _numberOfAvailableArguments *= 2;
        _arguments = (char **)realloc(_arguments,
                                      _numberOfAvailableArguments * sizeof(char *));
    }

    _arguments[_numberOfArguments] = argument;
    _arguments[_numberOfArguments + 1] = NULL;

    _numberOfArguments++;
}

Command::Command()
{
    _numberOfAvailableSimpleCommands = 1;
    _simpleCommands = (SimpleCommand **)
        malloc(_numberOfAvailableSimpleCommands * sizeof(SimpleCommand *));

    _numberOfSimpleCommands = 0;
    _outFile = 0;
    _inputFile = 0;
    _errFile = 0;
    _background = 0;
    _out_error = 0;
    _append = 0;
}

void Command::insertSimpleCommand(SimpleCommand *simpleCommand)
{
    if (_numberOfAvailableSimpleCommands == _numberOfSimpleCommands)
    {
        _numberOfAvailableSimpleCommands *= 2;
        _simpleCommands = (SimpleCommand **)realloc(_simpleCommands,
                                                    _numberOfAvailableSimpleCommands * sizeof(SimpleCommand *));
    }

    _simpleCommands[_numberOfSimpleCommands] = simpleCommand;
    _numberOfSimpleCommands++;
}

void Command::clear()
{
    for (int i = 0; i < _numberOfSimpleCommands; i++)
    {
        for (int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++)
        {
            free(_simpleCommands[i]->_arguments[j]);
        }

        free(_simpleCommands[i]->_arguments);
        delete _simpleCommands[i];
    }

    if (_outFile)
    {
        free(_outFile);
    }

    if (_inputFile)
    {
        free(_inputFile);
    }

    if (_errFile)
    {
        free(_errFile);
    }

    _numberOfSimpleCommands = 0;
    _outFile = 0;
    _inputFile = 0;
    _errFile = 0;
    _background = 0;
}

void Command::print()
{
    printf("\n\n");
    printf("              COMMAND TABLE                \n");
    printf("\n");
    printf("  #   Simple Commands\n");
    printf("  --- ----------------------------------------------------------\n");

    for (int i = 0; i < _numberOfSimpleCommands; i++)
    {
        printf("  %-3d ", i);
        for (int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++)
        {
            printf("\"%s\" \t", _simpleCommands[i]->_arguments[j]);
        }
        printf("\n");
    }

    printf("\n\n");
    printf("  Output       Input        Error        Err&Out       Background\n");
    printf("  ------------ ------------ ------------ ------------ ------------\n");
    printf("  %-12s %-12s %-12s %-12s %-12s\n", _outFile ? _outFile : "default",
           _inputFile ? _inputFile : "default", _errFile ? _errFile : "default", _out_error == 1 ? _errFile : "default"
           ,_background ? "YES" : "NO");
    printf("\n\n");
}

void Command::execute()
{
    print();

    for (int i =0; i < _numberOfSimpleCommands; i++) {
        pid_t process = fork();
        if (process < 0) {
            fprintf(stderr, "FAILED TO FORK\n");
            break;
        }
        else if (process == 0) {
            execvp(_simpleCommands[i]->_arguments[0], _simpleCommands[i]->_arguments);
            fprintf(stderr,"UNKNOWN COMMAND\n");
            exit(1);
        }
        else {
            if (_background == 0) {
                int status;
                waitpid(process, &status, 0);
            }
        }
        clear();
        prompt();
    }
}

void Command::prompt()
{
    printf("myshell>");
    fflush(stdout);
    std::string input;
    while (true)
    {
        std::getline(std::cin, input); 
        std::vector<Token> tokens = tokenize(input);
        parse(tokens);
    }
}

Command Command::_currentCommand;
SimpleCommand *Command::_currentSimpleCommand;

int main()
{
    Command::_currentCommand.prompt();
    return 0;
}
