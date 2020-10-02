#include "builtIn/builtIn.h"

void builtIn::print(char **arr) {
    int i = 0;
    while (arr[i]) {
        std::cout << arr[i] <<std::endl;
        i++;
    }
}

char* builtIn::getenvvar(char *var, char **arr) {
    int i = 0;
    char res[1024];
    while (arr[i]) {

        char* temp = static_cast<char *>(malloc(strlen(arr[i]) + 1));
        strcpy(temp, arr[i]);

        char delim[] = "=";
        char *token = strtok(temp, delim);
        if (strcmp(token, var) == 0) {
            token = strtok(NULL, delim);
            return token;
        }

        i++;
    }
    return res;
}

bool builtIn::checkHelpOption(char **parsed, int size) {

    for (int i = 1; i < size; i++) {
        if ( strcmp(parsed[i], "-h") == 0 || strcmp(parsed[i], "--help") == 0 ) {
            return true;
        }
    }
    return false;
}

void builtIn::handleMpwd(char **parsed, int size) {

    if (checkHelpOption(parsed, size)) {
        std::cout << "PWD help message" << std::endl;
        builtIn::merrno = builtIn::MERRNO_SUCCESS;
        return;
    }
    if (size == 1) {
        char cwd[1024];
        getcwd(cwd, sizeof(cwd));
        std::cout << cwd << std::endl;
        builtIn::merrno = builtIn::MERRNO_SUCCESS;
    }
    else {
        builtIn::merrno = builtIn::MERRNO_FAILURE;
    }
}

void builtIn::handleMcd(char **parsed, int size) {

    if (checkHelpOption(parsed, size)) {
        std::cout << "MCD help message" << std::endl;
        builtIn::merrno = builtIn::MERRNO_SUCCESS;
        return;
    }
    if (size == 2) {
        builtIn::merrno = chdir(parsed[1]);
    }
    else {
        builtIn::merrno = builtIn::MERRNO_FAILURE;
    }
}

void builtIn::handleMexit(char **parsed, int size) {

    if (checkHelpOption(parsed, size)) {
        std::cout << "MEXIT help message" << std::endl;
        builtIn::merrno = builtIn::MERRNO_SUCCESS;
        return;
    }
    if (size == 1)  {
        _exit(EXIT_SUCCESS);
    }
    else if (size == 2) {
        try {
            auto exitCode = atoi(parsed[1]);
            _exit(exitCode);
        }
        catch (std::invalid_argument&) {
            builtIn::merrno = builtIn::MERRNO_FAILURE;
        }
    }
    else {
        builtIn::merrno = builtIn::MERRNO_FAILURE;
    }
}

void builtIn::handleMecho(char **parsed, int size, char **env) {

    builtIn::merrno = builtIn::MERRNO_SUCCESS;

    if (checkHelpOption(parsed, size)) {
        std::cout << "MECHO help message" << std::endl;
        return;
    }

    for (int i = 1; i < size; i++) {
        if (parsed[i][0] != '$') {
            std::cout << parsed[i] << std::endl;
        }
        else {
//            // delete $
            int sz = strlen(parsed[i]);
            char *varName = static_cast<char *>(malloc(strlen(parsed[i])));;
            memmove(varName, parsed[i] + 1, sz - 1);
            varName[sz - 1] = '\0';
            std::cout << getenvvar(varName, env) << std::endl;
            free(varName);
        }
    }
}

void builtIn::handleMexport(char **parsed, int size, char **env) {

    int i = 0;
    while (env[i]) {
        ++i;
    }
    env[i] = static_cast<char *>(malloc(strlen(parsed[1]) + 1));
    strcpy(env[i], parsed[1]);
    builtIn::merrno = builtIn::MERRNO_SUCCESS;
}