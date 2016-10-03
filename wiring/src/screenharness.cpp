
#include "screenharness.hpp"

#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

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

void
screenharness::enqueue_command(screenharness::ScreenCommand cmd)
{
    std::string command_str = get_str_for_cmd(cmd);
    command_queue.append(command_str);
}

void
screenharness::flush_commands(void)
{
    pid_t pid = fork();
    if (pid < 0) {
        logger::error("can't send keys to screen; fork() failed");
    } else if (pid == 0) {
        std::string cmd = ("screen -d -S " SCREEN_SESSION " -p 0 -X stuff '" +
                           command_queue + "'");
        execlp("su", "su", SCREEN_USER, "-c", cmd.c_str(), nullptr);
    } else {
        waitpid(pid, nullptr, 0);
        command_queue.clear();
    }
}

