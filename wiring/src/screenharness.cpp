
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

/* Last time screen harness was verified as up. */
static uint32_t heartbeat_ms = 0;

/* How many successive respawn attempts have occurred. */
static uint32_t respawn_attempt_count = 0;

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
            return "^M"; // TODO not completely implemented in ncmpc yet
        case FIND_CURRENT_PLAYING:
            return "f"; // TODO not implemented in ncmpc yet
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
             return "a";
        case GOTO_CURRENT_PLAYLIST:
             return "2";
        default:
            return "";
    }
}

bool
screenharness::is_screen_up(uint32_t cur_ms)
{
    bool found = false;

    FILE *screen_proc = popen("su " SCREEN_USER " -c 'screen"
                              " -S " SCREEN_SESSION " -list'", "r");
    if (screen_proc == nullptr) {
        logger::error("couldn't check screen; popen() failed");
        return false;
    }

    char screen_output[128];
    while (fgets(screen_output, 128, screen_proc) != nullptr) {
        if (strstr(screen_output, SCREEN_SESSION)) {
            found = true;
            heartbeat_ms = cur_ms;
            break;
        }
    }

    fclose(screen_proc);

    return found;
}

bool
screenharness::spawn_screen(uint32_t cur_ms)
{
    pid_t pid = fork();
    if (pid < 0) {
        logger::error("can't spawn screen; fork() failed");
        return false;
    } else if (pid == 0) {
        setsid();
        execlp("su", "su", "-", SCREEN_USER, "-c",
               "LC_COLLATE=en_US.UTF8 screen -d -m -S " SCREEN_SESSION " ncmpc",
               nullptr);
    }
    waitpid(pid, nullptr, 0);

    if (screenharness::is_screen_up(cur_ms)) {
        logger::log("spawned new screen session");
        respawn_attempt_count = 0;
        return true;
    } else {
        logger::error("failed to spawn screen session");
        respawn_attempt_count++;
        return false;
    }
}

bool
screenharness::kill_screen(void)
{
    pid_t pid = fork();
    if (pid < 0) {
        logger::error("can't kill screen; fork() failed");
        return false;
    } else if (pid == 0) {
        execlp("su", "su", SCREEN_USER, "-c",
               "screen -X -S " SCREEN_SESSION " quit", nullptr);
    }
    int status;
    waitpid(pid, &status, 0);

    if (WIFEXITED(status)) {
        int exit_status = WEXITSTATUS(status);
        if (exit_status == 0) {
            logger::log("killed screen session");
            heartbeat_ms = 0; // you're a stone cold killer
            return true;
        } else {
            logger::error("failed to kill screen session");
            return false;
        }
    }

    return false;
}

void
screenharness::enqueue_command(screenharness::ScreenCommand cmd)
{
    std::string command_str = get_str_for_cmd(cmd);
    command_queue.append(command_str);
}

bool
screenharness::flush_commands(void)
{
    if (command_queue.empty())
        return false; // nothing to flush

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
    }
    command_queue.clear();

    int status;
    waitpid(pid, &status, 0);
    if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
        return true;

    return false;
}

uint32_t
screenharness::get_last_heartbeat_ms(void)
{
    return heartbeat_ms;
}

uint32_t
screenharness::get_respawn_retry_count(void)
{
    return respawn_attempt_count;
}

