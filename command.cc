
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

#include <csignal>
#include <cstdlib>

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

void myexit() {
    printf("\nGood Bye\n");
    exit(0);
}

void myCd(char *directory) {
    char *home = getenv("HOME");
    if (directory == nullptr) {
        if (chdir(home) == -1) fprintf(stderr,"FAILED TO CD ");
    }
    else if (chdir(directory) == -1) {
        fprintf(stderr, "FAILED TO CD\n");
    }
}

void logchild(pid_t pid,int status) {
    int log = open("logs.txt", O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (log == -1) {
        fprintf(stderr, "FAILED TO OPEN LOG FILE\n");
        return;
    }
    time_t t = time(NULL);
    std::string time(ctime(&t));
    std::string message = time + "Child with pid " + std::to_string(pid) + " was terminated with status " + std::to_string(WEXITSTATUS(status)) + "\n";
    write(log,message.c_str(),message.length());
    close(log);

}

void hanldesignaal(int sig) {
    int saved_errno = errno;
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) logchild(pid,status);
    saved_errno = errno;
}

void Command::execute()
{
    print();
    char *command = _simpleCommands[0]->_arguments[0];
    if (strcmp(command, "exit") == 0) myexit();
    else if (strcmp(command, "cd") == 0) {
        myCd(_simpleCommands[0]->_arguments[1]);
        clear();
        prompt();
    }
    int lastread = 0;
    int fd[2];
    for (int i =0; i < _numberOfSimpleCommands; i++) {
        if (i != _numberOfSimpleCommands - 1){
        if (pipe(fd) == -1) {
            fprintf(stderr, "FAILED TO PIPE\n");
            break;
            }
        }
        pid_t process = fork();
        if (process < 0) {
            fprintf(stderr, "FAILED TO FORK\n");
            if (i != _numberOfSimpleCommands - 1) {
                close(fd[1]);
                close(fd[0]);
            }
            break;
        }
        else if (process == 0) {
            if (i != 0) {
                if (dup2(lastread,0) == -1) fprintf(stderr, "FAILED TO READ FROM PIPE\n");
                close(lastread);
            }
            if (i != _numberOfSimpleCommands - 1) {
                close(fd[0]);
                dup2(fd[1],1);
                close(fd[1]);
            }
            if (_outFile && i == _numberOfSimpleCommands - 1) {
                int outputF;
                if (_append) outputF = open(_outFile, O_WRONLY | O_APPEND | O_CREAT, 0644);
                else outputF = open(_outFile, O_WRONLY | O_TRUNC |O_CREAT, 0644);
                if (outputF == -1) {
                    fprintf(stderr, "FAILED TO OPEN OUTPUT FILE\n");
                    exit(1);
                }
                if (dup2(outputF, 1) == -1) fprintf(stderr, "FAILED TO WRITE IN OUTPUT FILE\n");
                if (_out_error) if (dup2(outputF,2 ) == -1) fprintf(stderr, "FAILED TO WRITE IN OUTPUT FILE\n");
                close(outputF);
            }
            if (_inputFile && i == 0) {
                int inputF = open(_inputFile, O_RDONLY);
                if (inputF == -1) {
                    fprintf(stderr, "FAILED TO OPEN INPUT FILE\n");
                    exit(1);
                }
                if (dup2(inputF,0) == -1) fprintf(stderr, "FAILED TO READ FROM INPUT FILE\n");
                close(inputF);
            }
            if (_errFile && !_out_error) {
                int errorF = open(_errFile, O_WRONLY | O_TRUNC | O_CREAT, 0644);
                if (errorF == -1) {
                    fprintf(stderr, "FAILED TO OPEN ERROR FILE\n");
                    exit(1);
                }
                if (dup2(errorF,2) == -1) fprintf(stderr, "FAILED TO WRITE IN ERROR FILE\n");
                close(errorF);
            }
            execvp(_simpleCommands[i]->_arguments[0], _simpleCommands[i]->_arguments);
            fprintf(stderr,"UNKNOWN COMMAND\n");
            exit(1);
        }
        else {
            if (lastread != 0) close(lastread);
            if (i != _numberOfSimpleCommands - 1) {
                close(fd[1]);
                lastread = fd[0];
            }
            if (_background == 0) {
                int status;
                pid_t pid = waitpid(process, &status, 0);
                if (pid) logchild(pid,status);
            }
        }
    }
    clear();
    prompt();
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
    signal(SIGINT, SIG_IGN);
    signal(SIGCHLD,hanldesignaal);
    Command::_currentCommand.prompt();
    return 0;
}
