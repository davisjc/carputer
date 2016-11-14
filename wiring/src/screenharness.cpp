
#include "screenharness.hpp"

#include <sstream>
#include <stdio.h>
#include <string.h>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "globals.hpp"
#include "logger.hpp"


static std::string command_queue;

static std::string
get_str_for_cmd(screenharness::ScreenCommand cmd)
{
    using namespace screenharness;

    switch (cmd) {
        case SELECT_UP:
            return "k";
        case SELECT_DOWN:
            return "j";
        case SELECT_LEFT:
            return "h";
        case SELECT_RIGHT:
            return "l";
        case SELECT_UP_PAGE:
            return "^U";
        case SELECT_DOWN_PAGE:
            return "^D";
        case SELECT_TOP:
            return "g";
        case SELECT_BOTTOM:
            return "G";
        case PLAY_SELECTED:
            return "^M";
        case FIND_CURRENT_PLAYING:
            return "f";
        case CLEAR:
            return "c";
        case MOVE_UP:
             return "K";
        case MOVE_DOWN:
             return "J";
        case REMOVE:
             return "d";
        case GOTO_BROWSE:
             return "4";
        case ADD_TO_PLAYING:
             return " ";
        case GOTO_CURRENT_PLAYLIST:
             return "1";
        default:
            return "";
    }
}

bool
screenharness::is_screen_up(void)
{
    bool found = false;

    FILE* screen_proc = popen("su " SCREEN_USER " -c 'screen"
                              " -S " SCREEN_SESSION " -list'", "r");
    if (screen_proc == nullptr) {
        logger::error("couldn't check screen; popen() failed");
        return false;
    }

    char screen_output[128];
    while (fgets(screen_output, 128, screen_proc) != nullptr) {
        if (strstr(screen_output, SCREEN_SESSION)) {
            found = true;
            break;
        }
    }

    fclose(screen_proc);

    return found;
}

bool
screenharness::spawn_screen(void)
{
    pid_t pid = fork();
    if (pid < 0) {
        logger::error("can't spawn screen; fork() failed");
        return false;
    } else if (pid == 0) {
        execlp("su", "su", "-", SCREEN_USER, "-c",
               "LC_COLLATE=en_US.UTF8 screen -D -m -S " SCREEN_SESSION " ncmpc",
               nullptr);
    }
    return true;
}

void
screenharness::enqueue_command(screenharness::ScreenCommand cmd)
{
    std::string command_str = get_str_for_cmd(cmd);
    command_queue.append(command_str);
}

void
screenharness::flush_commands(void)
{
    if (command_queue.empty())
        return; // nothing to flush

    pid_t pid = fork();
    if (pid < 0) {
        logger::error("can't send keys to screen; fork() failed");
    } else if (pid == 0) {
        std::string cmd = ("screen -d -S " SCREEN_SESSION " -p 0 -X stuff '" +
                           command_queue + "'");
#ifndef DEBUG
        fclose(stderr);
        fclose(stdout);
#endif
        execlp("su", "su", SCREEN_USER, "-c", cmd.c_str(), nullptr);
    } else {
        waitpid(pid, nullptr, 0);
        command_queue.clear();
    }
}

