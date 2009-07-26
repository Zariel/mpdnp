/*
Copyright (c) 2008 Chris Bannister,
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the author may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <libmpd-1.0/libmpd/libmpd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <signal.h>

char *path = NULL;

int fd = (int) NULL;
MpdObj *obj = NULL;

void tidyUp() {
    if(fd) {
        close(fd);
    }

    if(obj != NULL) {
        mpd_free(obj);
    }
}

void handler(int sig) {
    tidyUp();
}

void status_changed(MpdObj *obj, ChangedStatusType what) {
    mpd_Song *song = NULL;
    size_t size;
    char *str = NULL;

    if(what & MPD_CST_SONGID) {
        song = mpd_playlist_get_current_song(obj);

        if(song) {
            size = strlen(song->artist) + strlen(song->title) + 5;
            str = malloc(size);
            snprintf(str, size, "%s - %s\n", song->artist, song->title);
        }
    } else if(what & MPD_CST_STATE) {
        switch(mpd_player_get_state(obj)) {
            case MPD_PLAYER_STOP:
                str = malloc(2);
                snprintf(str, 2, "\n");
               break;
            case MPD_PLAYER_PLAY:
                song = mpd_playlist_get_current_song(obj);

                if(song) {
                    size = strlen(song->artist) + strlen(song->title) + 5;
                    str = malloc(size);
                    snprintf(str, size, "%s - %s\n", song->artist, song->title);
                }
                break;
            case MPD_PLAYER_PAUSE:
                song = mpd_playlist_get_current_song(obj);

                if(song) {
                    size = strlen(song->artist) + strlen(song->title) + 6;
                    str = malloc(size);
                    snprintf(str, size, "%s - %s*\n", song->artist, song->title);
                }
                break;
            default:
                break;

        }
    }

    if(str) {
        write(fd, str, size);
        free(str);
    }
};


int main() {
    if(daemon(0, 0) < 0) {
        exit(1);
    }

    signal(SIGHUP, handler);
    signal(SIGTERM, handler);
    signal(SIGINT, handler);
    signal(SIGQUIT, handler);

    char *home = getenv("HOME");

    int size = snprintf(NULL, 0, "%s/.mpdnp.pipe", home) + 1;
    path = malloc(size);
    snprintf(path, size, "%s/.mpdnp.pipe", home);

    mkfifo(path, 0666);
    fd = open(path, O_WRONLY | O_NONBLOCK);

    if(path != NULL) {
        free(path);
    }

    write(fd, " ", 2);

    char *host = getenv("MPD_HOST");
    char *port = getenv("MPD_PORT");

    if(host == NULL) {
        host = "localhost";
    }

    if(port == NULL) {
        port = "6600";
    }

    // New object
    obj = mpd_new(host, atoi(port), NULL);

    // Connect the signal
    mpd_signal_connect_status_changed(obj, (StatusChangedCallback) status_changed, NULL);

    // Timeout
    mpd_set_connection_timeout(obj, 10);

    if(!mpd_connect(obj)) {
        while(!usleep(100000)) {
            if(obj == NULL) {
                break;
            } else {
                mpd_status_update(obj);
            }
        }
    }

    tidyUp();

    return 0;
}
