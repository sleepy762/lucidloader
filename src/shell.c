#include "shell.h"

/* Main shell functions */
static int8_t ShellLoop(char_t** currPathPtr);
static int8_t ProcessCommand(char_t buffer[], char_t** currPathPtr);

/* Command and argument processing */
static char_t* GetCommandFromBuffer(char_t buffer[]);
static int8_t ParseArgs(char_t* inputArgs, cmd_args_s** outputArgs);
static int8_t SplitArgsString(char_t buffer[], cmd_args_s** outputArgs);
static cmd_args_s* InitializeArgsNode(void);
static void AppendArgsNode(cmd_args_s* head, cmd_args_s* node);
static void FreeArgs(cmd_args_s* args);


int8_t StartShell(void)
{
    // If the shell is password protected, ask to login
    if (ShellLoginWithPassword() == FALSE)
    {
        // Login failed
        return -1;
    }

    Log(LL_INFO, 0, "Starting the shell.");
    ST->ConOut->ClearScreen(ST->ConOut);
    ST->ConOut->EnableCursor(ST->ConOut, TRUE);
    printf("Welcome to the bootloader shell!\n"
           "Type `help` to get a list of commands.\n"
           "Type `help cmd` for info on a command.\n\n");

    // 2 is the initial size for the root dir "\" and null string terminator
    char_t* currPath = malloc(2 * sizeof(char_t));
    if (currPath == NULL)
    {
        Log(LL_ERROR, 0, "Failed to allocate memory for the path during shell initialization.");
        return CMD_OUT_OF_MEMORY;
    }

    // Initializing the default starting path
    currPath[0] = '\\';
    currPath[1] = CHAR_NULL;

    ST->ConIn->Reset(ST->ConIn, 0); // Reset the input buffer
    
    if (ShellLoop(&currPath) == 1)
    {
        return CMD_OUT_OF_MEMORY;
    }

    // Cleanup
    Log(LL_INFO, 0, "Closing the shell.");
    free(currPath);
    ST->ConOut->EnableCursor(ST->ConOut, FALSE);
    ST->ConOut->ClearScreen(ST->ConOut);
    return 0;
}

static int8_t ShellLoop(char_t** currPathPtr)
{
    while (TRUE)
    {
        char_t buffer[SHELL_MAX_INPUT] = {0};
        printf("> ");

        GetInputString(buffer, SHELL_MAX_INPUT, FALSE);

        if (strcmp(buffer, SHELL_EXIT_STR) == 0)
        {
            break;
        }
        if (ProcessCommand(buffer, currPathPtr) == 1)
        {
            return CMD_OUT_OF_MEMORY;
        }
    }
    return CMD_SUCCESS;
}

static int8_t ProcessCommand(char_t buffer[], char_t** currPathPtr)
{
    buffer = TrimSpaces(buffer);

    char_t* args = buffer;
    char_t* cmd = GetCommandFromBuffer(buffer);
    if (cmd == NULL)
    {
        return 0;
    }

    // Parse the arguments into a linked list (if there are any)
    cmd_args_s* cmdArgs = NULL;
    if (args != NULL)
    {
        int8_t res = ParseArgs(args, &cmdArgs);
        if (res != CMD_SUCCESS)
        {
            PrintCommandError(cmd, args, res);
            return res;
        }
    }

    const uint8_t totalCmds = CommandCount();
    for (uint8_t i = 0; i < totalCmds; i++)
    {   
        // Find the right command and execute the command function
        if (strcmp(cmd, commands[i].commandName) == 0)
        {
            // Pass a pointer to the head of the linked list because it may be modified
            commands[i].CommandFunction(&cmdArgs, currPathPtr);
            break;
        }
        else if (i + 1 == totalCmds)
        {
            printf("Command '%s' not found.\n", cmd);
        }
    }

    free(cmd);
    FreeArgs(cmdArgs);
    return CMD_SUCCESS;
}

static char_t* GetCommandFromBuffer(char_t buffer[])
{
    size_t bufferLen = strlen(buffer);
    if (bufferLen == 0)
    {
        return NULL;
    }

    int32_t cmdLen = 0;
    int32_t cmdOffset = GetValueOffset(buffer, ' ');
    if (cmdOffset == -1)
    {
        cmdLen = bufferLen + 1;
    }
    else
    {
        cmdLen = cmdOffset;
    }

    char_t* cmd = malloc(cmdLen);
    memcpy(cmd, buffer, cmdLen);
    cmd[cmdLen - 1] = CHAR_NULL;

    return cmd;
}

static int8_t ParseArgs(char_t* inputArgs, cmd_args_s** outputArgs)
{
    if (inputArgs == NULL)
    {
        return CMD_SUCCESS;
    }

    const size_t argsLen = strlen(inputArgs);
    char_t tempBuffer[SHELL_MAX_INPUT] = {0};
    uint8_t bufferIdx = 0; // Current index of the buffer where the next char will be stored

    boolean_t qoutationMarkOpened = FALSE;
    for (size_t i = 0; i <= argsLen; i++)
    {
        if (inputArgs[i] == QUOTATION_MARK)
        {
            // Only if the quotation mark is the last character in the argument
            if (qoutationMarkOpened && (inputArgs[i + 1] == SPACE || inputArgs[i + 1] == CHAR_NULL))
            {
                int8_t res = SplitArgsString(tempBuffer, outputArgs);
                if (res != CMD_SUCCESS)
                {
                    return res;
                }
                qoutationMarkOpened = FALSE;
            }
            // Only if the quotation mark is the first character in the argument
            else if (i == 0 || inputArgs[i - 1] == SPACE)
            {
                qoutationMarkOpened = TRUE;
            }
            else // Add the quotation mark if its in the middle of the arg string
            {
                goto addChar;
            }
        }
        // Split the args
        else if (inputArgs[i] == SPACE)
        {
            // If quotation marks were opened and we hit a space, we include that space in the string
            if (qoutationMarkOpened)
            {
                goto addChar;
            }
            else
            {
                int8_t res = SplitArgsString(tempBuffer, outputArgs);
                if (res != CMD_SUCCESS)
                {
                    return res;
                }
                bufferIdx = 0; // Reset the buffer index
            }
        }
        else if (inputArgs[i] == CHAR_NULL)
        {
            // Quotation mark wasn't closed
            if (qoutationMarkOpened)
            {
                return CMD_QUOTATION_MARK_OPEN;
            }
            else
            {
                int8_t res = SplitArgsString(tempBuffer, outputArgs);
                if (res != CMD_SUCCESS)
                {
                    return res;
                }
            }
        }
        else
        {
        addChar:
            tempBuffer[bufferIdx] = inputArgs[i];
            bufferIdx++;
        }
    }
    return CMD_SUCCESS;
}

static int8_t SplitArgsString(char_t buffer[], cmd_args_s** outputArgs)
{
    // If the buffer is empty don't do anything
    if (buffer[0] == CHAR_NULL)
    {
        return CMD_SUCCESS;
    }

    cmd_args_s* node = InitializeArgsNode();
    if (node == NULL)
    {
        return CMD_OUT_OF_MEMORY;
    }

    // Allocate memory for the argument string and copy the buffer into it
    const size_t bufferLen = strlen(buffer);
    node->argString = malloc(bufferLen + 1);
    if (node->argString == NULL)
    {
        Log(LL_ERROR, 0, "Failed to allocate memory for the argument string pointer.");
        return CMD_OUT_OF_MEMORY;
    }
    memcpy(node->argString, buffer, bufferLen);
    node->argString[bufferLen] = CHAR_NULL;

    // Append to the linked list or set the node as the head if it hasn't been initialized yet
    if (*outputArgs == NULL)
    {
        *outputArgs = node;
    }
    else
    {
        AppendArgsNode(*outputArgs, node);
    }

    memset(buffer, 0, SHELL_MAX_INPUT); // Reset the buffer
    return CMD_SUCCESS;
}

static cmd_args_s* InitializeArgsNode(void)
{
    cmd_args_s* node = malloc(sizeof(cmd_args_s));
    if (node == NULL)
    {
        Log(LL_ERROR, 0, "Failed to initialize argument node.");
    }
    else
    {
        node->argString = NULL;
        node->next = NULL;
    }
    return node;
}

// Add a new node to the end of the linked list
static void AppendArgsNode(cmd_args_s* head, cmd_args_s* node)
{
    cmd_args_s* copy = head;
    while (copy->next != NULL)
    {
        copy = copy->next;
    }
    copy->next = node;
}

// Freeing args without recursion
static void FreeArgs(cmd_args_s* args)
{
    while (args != NULL)
    {
        cmd_args_s* next = args->next;

        free(args->argString);
        free(args);

        args = next;
    }
}
