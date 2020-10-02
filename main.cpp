// C Program to design a shell in Linux
#include <cstdio>
#include <cstring>
#include <string>
#include <fstream>
#include <cstdlib>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<readline/readline.h>
#include<readline/history.h>
#include <boost/filesystem.hpp>
#include <boost/range/iterator_range.hpp>
#include <fnmatch.h>

#include "builtIn/builtIn.h"

#define MAXCOM 1000 // max number of letters to be supported
#define MAXLIST 100 // max number of commands to be supported
#define MAXENV 1024 // max namber of environment variables
#define clearShell() printf("\033[H\033[J")
#define _GNU_SOURCE 1

// Function to print current directory.
void printCurrDir()
{
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    printf("%s", cwd);
}

// Function to take user input.
int getInput(char* str)
{
    char* buf;
    printCurrDir();
    buf = readline(" $ ");
    if (strlen(buf) != 0) {
        add_history(buf);
        strcpy(str, buf);
        return 0;
    } else {
        return 1;
    }
}

// Function to execute received args.
void executeArgs(char** parsed, char** env)
{
    pid_t pid = fork();

    if (pid == -1) {
        std::cout << "\nFailed forking child.." << std::endl;
        return;
    } else if (pid == 0) {
        if (execvpe(parsed[0], parsed, env) < 0) {
            std::cout << "Could not execute command.." << std::endl;
        }
        exit(0);
    } else {
        // waiting for child to terminate
        wait(NULL);
        return;
    }
}

// Function to execute builtin commands
int executeBuiltInComs(char** parsed, int size, char **env)
{
    int NoOfOwnCmds = 6, i, switchOwnArg = 0;
    char* ListOfOwnCmds[NoOfOwnCmds];
    char* username;

    ListOfOwnCmds[0] = "mexit";
    ListOfOwnCmds[1] = "mcd";
    ListOfOwnCmds[2] = "mpwd";
    ListOfOwnCmds[3] = "merrno";
    ListOfOwnCmds[4] = "mecho";
    ListOfOwnCmds[5] = "mexport";

    for (i = 0; i < NoOfOwnCmds; i++) {
        if (strcmp(parsed[0], ListOfOwnCmds[i]) == 0) {
            switchOwnArg = i + 1;
            break;
        }
    }

    switch (switchOwnArg) {
        case 1:
            builtIn::handleMexit(parsed, size);
            return 1;
        case 2:
            builtIn::handleMcd(parsed, size);
            return 1;
        case 3:
            builtIn::handleMpwd(parsed, size);
            return 1;
        case 4:
            std::cout << builtIn::merrno << std::endl;
            return 1;
        case 5:
            builtIn::handleMecho(parsed, size, env);
            return 1;
        case 6:
            builtIn::handleMexport(parsed, size, env);
            return 1;
        default:
            break;
    }

    return 0;
}

int checkMatch(const char *pattern, const char *filename) {
    if (fnmatch(pattern, filename, FNM_EXTMATCH) != FNM_NOMATCH){
//        printf("\nMATCH %s %s", pattern, filename);
        return 1;
    }
    return 0;

}


int checkWildcards(char** parsed, int argsAmount) {
    int addedArgsCount = 0;
    // iterating through command line args
    for( int i = 0; i < argsAmount; i++) {
        // searching args which include wildcards and filepaths
        if (parsed[i] != NULL) {
            if ((strchr(parsed[i], '?') != NULL) or (strchr(parsed[i], '*') != NULL)
                or (strchr(parsed[i], '[') != NULL)) {
                char *wildcard;
                int pathLen = strlen(parsed[i]);
                char pathToSearch[100];
                char arg[pathLen];

                if ((strchr(parsed[i], '/') != NULL)) {
                    // copy file path for further dir path extracting

                    std::strcpy(pathToSearch, parsed[i]);
                    std::strcpy(arg, parsed[i]);

                    // splitting by slash
                    char *token = strtok(arg, "/");
                    //            // saving last splited part as a wildcard

                    while (token != NULL) {
                        wildcard = token;
                        token = strtok(NULL, "/");
                    }

                    int wildLen = strlen(wildcard);
                    pathToSearch[pathLen - wildLen] = '\0';
                } else {
                    std::strcpy(wildcard, parsed[i]);
                    getcwd(pathToSearch, sizeof(pathToSearch));
                }

                // iterating through dir
                if (boost::filesystem::is_directory(pathToSearch)) {
                    int matchFound = 0;
                    // checking wildcard match for files in dir
                    for (auto &entry : boost::make_iterator_range(boost::filesystem::directory_iterator(pathToSearch),
                                                                  {})) {
                        auto path = entry.path().c_str();

                        // detecting files matchig with wildcards
                        if (checkMatch(wildcard, path)) {
                            if (!matchFound) {
                                strcpy(parsed[i], path);
                                matchFound = 1;
                            }
                            else {

                                parsed[argsAmount + addedArgsCount] = static_cast<char *>(malloc(strlen(path) + 1));
                                strcpy(parsed[argsAmount + addedArgsCount], path);
                                addedArgsCount += 1;
                            }
                        }
                    }
                }
            }
        }
    }

    return argsAmount+addedArgsCount;
};

// function for parsing command words
int parseSpace(char* str, char** parsed)
{
    int i;

    for (i = 0; i < MAXLIST; i++) {
        parsed[i] = strsep(&str, " ");

        if (parsed[i] == nullptr || parsed[i][0] == '#') {
            parsed[i] = nullptr;
            return i;
        }
        if (strlen(parsed[i]) == 0)
            i--;
    }
    return MAXLIST;
}

int processString(char* str, char** parsed, char **env)
{
    auto size = parseSpace(str, parsed);
    size = checkWildcards(parsed, size);

    if (size == 0 || executeBuiltInComs(parsed, size, env)) {
        return 0;
    }
    else {
        return 1;
    }
}

void runCommand(char* inputString, char** parsedArgs, char** envVars) {
    // process
    int execFlag = processString(inputString, parsedArgs, envVars);

    // execute
    if (execFlag == 1)
        executeArgs(parsedArgs, envVars);
}

bool isExternalScript(char *inputString) {

    // remove spaces before
    int i = 0;
    while (inputString[i] == ' ') ++i;
    memmove(inputString, inputString + i, strlen(inputString) - i);
    inputString[strlen(inputString) - i] = '\0';

    int size  = strlen(inputString);
    return size > 6 && inputString[0] == '.' && inputString[1] == '/' && inputString[size - 4] == '.'
           && inputString[size - 3] == 'm' && inputString[size - 2] == 's' && inputString[size - 1] == 'h';
}

bool isInternalScript(char *inputString) {

    // remove spaces before
    int i = 0;
    while (inputString[i] == ' ') ++i;
    memmove(inputString, inputString + i, strlen(inputString) - i);
    inputString[strlen(inputString) - i] = '\0';

    int size  = strlen(inputString);
    return size > 6 && inputString[0] == '.' && inputString[1] == ' ' && inputString[size - 4] == '.'
           && inputString[size - 3] == 'm' && inputString[size - 2] == 's' && inputString[size - 1] == 'h';
}

void runInternalScript(char* inputString, char** parsedArgs, char** envVars) {

    std::fstream newfile;
    newfile.open(inputString, std::ios::in);
    if (newfile.is_open()){
        std::string tp;
        while(getline(newfile, tp)){
            auto input = static_cast<char *>(malloc(strlen(tp.c_str()) + 1));
            strcpy(input, tp.c_str());
            runCommand(input, parsedArgs, envVars);
            free(input);
        }
        newfile.close();
        builtIn::merrno = builtIn::MERRNO_SUCCESS;
        return;
    }
    builtIn::merrno = builtIn::MERRNO_FAILURE;
}

int main(int argc, char *argv[])
{
    clearShell();
    char inputString[MAXCOM], *parsedArgs[MAXLIST], *envVars[MAXENV];

    char **env = environ;
    int i = 0;
    while (env[i])
    {
        envVars[i] = env[i];
        ++i;
    }
    if (argc > 2) return EXIT_FAILURE;
    // script from command line
    else if (argc == 2) {
        int size  = strlen(argv[1]);
        if (size > 4 && argv[1][size - 4] == '.' && argv[1][size - 3] == 'm'
            && argv[1][size - 2] == 's' && argv[1][size - 1] == 'h') {
            runInternalScript(argv[1], parsedArgs, envVars);
        }
    }

    while (1) {

        if (getInput(inputString))
            continue;



        if (isInternalScript(inputString)) {
            runInternalScript(inputString + 2, parsedArgs, envVars);
            continue;
        }

        if (isExternalScript(inputString)) {
            char *myshell = "./myshell ";
            char input[strlen(myshell) + strlen(inputString)];
            int i = 0;
            while (i < strlen(myshell)) {
                input[i] = myshell[i];
                ++i;
            }
            while (i - strlen(myshell) < strlen(inputString)) {
                input[i] = inputString[i - strlen(myshell)];
                ++i;
            }
            input[i] = '\0';
            runCommand(input, parsedArgs, envVars);
            continue;
        }

        runCommand(inputString, parsedArgs, envVars);
    }
    return EXIT_SUCCESS;
}