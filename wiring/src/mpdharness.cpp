
#include "mpdharness.hpp"

#include <queue>
#include <sstream>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "globals.hpp"
#include "logger.hpp"


static std::queue<mpdharness::MPDCommand> command_queue;

void
mpdharness::enqueue_command(mpdharness::MPDCommand cmd)
{
    command_queue.push(cmd);
}

static bool
run_command(mpdharness::MPDCommand cmd)
{
    std::string suffix;

    switch (cmd) {
        case mpdharness::PLAY_PAUSE:
            suffix = "toggle";
            break;
        case mpdharness::STOP:
            suffix = "stop";
            break;
        case mpdharness::CLEAR:
            suffix = "clear";
            break;
        case mpdharness::PREV:
            suffix = "prev";
            break;
        case mpdharness::NEXT:
            suffix = "next";
            break;
        case mpdharness::SEEK_BACK:
            suffix = "-" MPD_SEEK_PCT "%";
            break;
        case mpdharness::SEEK_BACK_FAST:
            suffix = "-" MPD_SEEK_FAST_PCT "%";
            break;
        case mpdharness::SEEK_FWD:
            suffix = "+" MPD_SEEK_PCT "%";
            break;
        case mpdharness::SEEK_FWD_FAST:
            suffix = "+" MPD_SEEK_FAST_PCT "%";
            break;
        default:
            return false;
    }

    pid_t pid = fork();
    if (pid < 0) {
        logger::error("can't send keys to mpc; fork() failed");
    } else if (pid == 0) {
        std::string cmd = "mpc ";
        cmd += suffix;
#ifndef DEBUG
        fclose(stderr);
        fclose(stdout);
#endif
        execlp("su", "su", MPD_USER, "-c", cmd.c_str(), nullptr);
    }

    int status;
    waitpid(pid, &status, 0);
    if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
        return true;

    return false;
}

bool
mpdharness::flush_commands(void)
{
    if (command_queue.empty())
        return false;

    while (!command_queue.empty()) {
        mpdharness::MPDCommand cmd = command_queue.front();
        command_queue.pop();
        run_command(cmd);
    }
    return true;
}

