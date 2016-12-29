
#include "mpdharness.hpp"

#include <mpd/client.h>
#include <sstream>

#include "logger.hpp"


static void
handle_mpd_error(struct mpd_connection *conn)
{
    enum mpd_error err = mpd_connection_get_error(conn);
    if (err != MPD_ERROR_SUCCESS) {
        logger::error(mpd_connection_get_error_message(conn));
    } else {
        logger::error("attempting to handle mpd error, but none exists");
    }
    mpd_connection_free(conn);
}

static struct mpd_connection *
connect(void)
{
    struct mpd_connection *conn = mpd_connection_new(nullptr, 0, 100);
    if (mpd_connection_get_error(conn) != MPD_ERROR_SUCCESS) {
        handle_mpd_error(conn);
        return nullptr;
    }

    return conn;
}

static void
disconnect(struct mpd_connection *conn)
{
    if (!conn) {
        logger::error("attempted to close null mpd connection");
        return;
    }

    if (mpd_connection_get_error(conn) != MPD_ERROR_SUCCESS
        || !mpd_response_finish(conn))
        handle_mpd_error(conn);
}

static void
run_mpd_cmd(bool (*mpd_func)(struct mpd_connection *))
{
    struct mpd_connection *conn = connect();
    if (!conn)
        return;

    mpd_func(conn);

    disconnect(conn);
};

void
mpdharness::playpause(void)
{
    run_mpd_cmd(mpd_send_toggle_pause);
};

void
mpdharness::stop(void)
{
    run_mpd_cmd(mpd_send_stop);
}

void
mpdharness::clear(void)
{
    run_mpd_cmd(mpd_send_clear);
}

void
mpdharness::previous(void)
{
    run_mpd_cmd(mpd_send_previous);
}

void
mpdharness::next(void)
{
    run_mpd_cmd(mpd_send_next);
}

void
mpdharness::seek(int32_t offset_s)
{
}


