/*
 Copyright (C) 1997-2001 Id Software, Inc.

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

 See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

 */

//Curl interface functions. OpenTDM can use libcurl to fetch and POST to the
//website, used for downloading configs and uploading stats (todo).
#include "g_local.h"
#include "g_tdm.h"

#ifdef HAVE_CURL

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif

#include <curl/curl.h>

typedef struct dlhandle_s {
    CURL *curl;
    size_t fileSize;
    size_t position;
    double speed;
    char filePath[1024];
    char URL[2048];
    char *tempBuffer;
    qboolean inuse;
    tdm_download_t *tdm_handle;

    // set for an HTTP_PostJSON request, owned by the slot until it finishes
    char *postData;
    struct curl_slist *postHeaders;
    struct curl_slist *postResolve;
} dlhandle_t;

//we need this high in case a sudden server switch causes a bunch of people
//to connect, we want to be able to download their configs
#define MAX_DOWNLOADS	16

//size limits for configs, must be power of two
#define MAX_DLSIZE	0x100000	// 1 MiB
#define MIN_DLSIZE	0x8000		// 32 KiB

dlhandle_t downloads[MAX_DOWNLOADS];

static CURLM *multi = NULL;
static unsigned handleCount = 0;

static char otdm_api_ip[16];
static char hostHeader[64];
static struct curl_slist *http_header_slist;

static time_t last_dns_lookup;

// cached DNS result for g_stats_url, handed to curl via CURLOPT_RESOLVE
static char stats_resolve_url[1024];
static char stats_resolve_entry[384];
static time_t stats_last_dns_lookup;

/**
 * Properly escapes a path with HTTP %encoding. libcurl's function
 * seems to treat '/' and such as illegal chars and encodes almost
 * the entire URL...
 */
static void HTTP_EscapePath(const char *filePath, char *escaped) {
    int i;
    size_t len;
    char *p;

    p = escaped;

    len = strlen(filePath);
    for (i = 0; i < len; i++) {
        if (!isalnum((unsigned char) filePath[i]) &&filePath[i] != ';' && filePath[i] != '/'
                && filePath[i] != '?' && filePath[i] != ':'
                && filePath[i] != '@' && filePath[i] != '&'
                && filePath[i] != '=' && filePath[i] != '+'
                && filePath[i] != '$' && filePath[i] != ','
                && filePath[i] != '[' && filePath[i] != ']'
                && filePath[i] != '-' && filePath[i] != '_'
                && filePath[i] != '.' && filePath[i] != '!'
                && filePath[i] != '~' && filePath[i] != '*'
                && filePath[i] != '\'' && filePath[i] != '('
                && filePath[i] != ')') {
            sprintf(p, "%%%02x", (unsigned char) filePath[i]);
            p += 3;
        } else {
            *p = filePath[i];
            p++;
        }
    }
    p[0] = 0;

    //using ./ in a url is legal, but all browsers condense the path and some IDS / request
    //filtering systems act a bit funky if http requests come in with uncondensed paths.
    len = strlen(escaped);
    p = escaped;
    while ((p = strstr(p, "./"))) {
        memmove(p, p + 2, len - (p - escaped) - 1);
        len -= 2;
    }
}

/**
 * libcurl callback.
 */
static size_t EXPORT HTTP_Recv(void *ptr, size_t size, size_t nmemb,
        void *stream) {
    dlhandle_t *dl;
    size_t new_size, bytes;

    dl = (dlhandle_t*) stream;

    if (!nmemb) {
        return 0;
    }

    if (size > SIZE_MAX / nmemb) {
        goto oversize;
    }

    if (dl->position > MAX_DLSIZE) {
        goto oversize;
    }

    bytes = size * nmemb;
    if (bytes >= MAX_DLSIZE - dl->position) {
        goto oversize;
    }

    //grow buffer in MIN_DLSIZE chunks. +1 for NUL.
    new_size = (dl->position + bytes + MIN_DLSIZE) & ~(MIN_DLSIZE - 1);
    if (new_size > dl->fileSize) {
        char *tmp;

        tmp = dl->tempBuffer;
        dl->tempBuffer = gi.TagMalloc((int) new_size, TAG_GAME);
        if (tmp) {
            memcpy(dl->tempBuffer, tmp, dl->fileSize);
            gi.TagFree(tmp);
        }
        dl->fileSize = new_size;
    }

    memcpy(dl->tempBuffer + dl->position, ptr, bytes);
    dl->position += bytes;
    dl->tempBuffer[dl->position] = 0;

    return bytes;

    oversize: gi.dprintf(
            "Suspiciously large file while trying to download %s!\n", dl->URL);
    return 0;
}

/**
 *
 */
int EXPORT CURL_Debug(CURL *c, curl_infotype type, char *data, size_t size,
        void *ptr) {
    if (type == CURLINFO_TEXT) {
        char buff[4096];
        if (size > sizeof(buff) - 1) {
            size = sizeof(buff) - 1;
        }
        Q_strncpy(buff, data, size);
        gi.dprintf("  OpenTDM HTTP DEBUG: %s", buff);
        if (!strchr(buff, '\n')) {
            gi.dprintf("\n");
        }
    }
    return 0;
}

/**
 * Resolve the g_http_domain and cache it, so we don't do DNS
 * lookups at critical times (eg mid match).
 */
void HTTP_ResolveOTDMServer(void) {
    if (!g_http_enabled->value) {
        return;
    }

    //re-resolve if its been more than one day since we last did it
    if (time(NULL) - last_dns_lookup > 86400) {
        gi.cprintf(NULL, PRINT_HIGH, "Resolving API server %s -> ",
                g_http_domain->string);
        struct hostent *h;
        h = gethostbyname(g_http_domain->string);

        if (!h) {
            otdm_api_ip[0] = '\0';
            gi.dprintf(
                    "WARNING: Could not resolve OpenTDM web API server '%s'. HTTP functions unavailable.\n",
                    g_http_domain->string);
            return;
        }

        time(&last_dns_lookup);

        Q_strncpy(otdm_api_ip, inet_ntoa(*(struct in_addr* )h->h_addr_list[0]),
                sizeof(otdm_api_ip) - 1);
        gi.cprintf(NULL, PRINT_HIGH, "%s\n", otdm_api_ip);
    }
}

/**
 * Resolve the host in g_stats_url and cache it. The bundled libcurl has no
 * asynchronous resolver, so letting curl look it up would block the server
 * frame. Called on game state reset and again before posting, it only does
 * a lookup when the URL changed or the cached result is a day old.
 */
void HTTP_ResolveStatsServer(void) {
    CURLU *u;
    char *host, *port;
    struct hostent *h;

    if (!g_send_stats->value || !g_stats_url->string[0]) {
        return;
    }

    if (!strcmp(stats_resolve_url, g_stats_url->string)
            && time(NULL) - stats_last_dns_lookup <= 86400) {
        return;
    }

    Q_strncpy(stats_resolve_url, g_stats_url->string,
            sizeof(stats_resolve_url) - 1);
    stats_resolve_entry[0] = '\0';
    stats_last_dns_lookup = 0;

    u = curl_url();
    if (!u) {
        return;
    }

    host = port = NULL;
    if (curl_url_set(u, CURLUPART_URL, g_stats_url->string, 0) != CURLUE_OK
            || curl_url_get(u, CURLUPART_HOST, &host, 0) != CURLUE_OK
            || curl_url_get(u, CURLUPART_PORT, &port, CURLU_DEFAULT_PORT)
                    != CURLUE_OK) {
        gi.dprintf("WARNING: g_stats_url '%s' is not a valid URL.\n",
                g_stats_url->string);
        goto done;
    }

    // IP literals need no lookup (IPv6 hosts come back bracketed)
    if (strchr(host, ':') || inet_addr(host) != INADDR_NONE) {
        goto done;
    }

    gi.cprintf(NULL, PRINT_HIGH, "Resolving stats server %s -> ", host);
    h = gethostbyname(host);
    if (!h) {
        // leave the cache empty, curl will try again itself when posting
        gi.dprintf("WARNING: Could not resolve stats server '%s'.\n", host);
        goto done;
    }

    Com_sprintf(stats_resolve_entry, sizeof(stats_resolve_entry), "%s:%s:%s",
            host, port, inet_ntoa(*(struct in_addr*) h->h_addr_list[0]));
    time(&stats_last_dns_lookup);
    gi.cprintf(NULL, PRINT_HIGH, "%s\n",
            inet_ntoa(*(struct in_addr*) h->h_addr_list[0]));

done:
    curl_free(host);
    curl_free(port);
    curl_url_cleanup(u);
}

/**
 * Prepare a slot's curl handle for a new request. Handles are reused between
 * downloads and posts, so start from a clean set of options each time.
 */
static void HTTP_SetupHandle(dlhandle_t *dl) {
    dl->tempBuffer = NULL;
    dl->speed = 0;
    dl->fileSize = 0;
    dl->position = 0;

    if (!dl->curl) {
        dl->curl = curl_easy_init();
    } else {
        curl_easy_reset(dl->curl);
    }

    if (g_http_debug->value) {
        curl_easy_setopt(dl->curl, CURLOPT_DEBUGFUNCTION, CURL_Debug);
        curl_easy_setopt(dl->curl, CURLOPT_VERBOSE, 1);
    }

    if (g_http_bind->string[0]) {
        curl_easy_setopt(dl->curl, CURLOPT_INTERFACE, g_http_bind->string);
    }

    if (g_http_proxy->string[0]) {
        curl_easy_setopt(dl->curl, CURLOPT_PROXY, g_http_proxy->string);
    }

    curl_easy_setopt(dl->curl, CURLOPT_NOPROGRESS, 1);
    curl_easy_setopt(dl->curl, CURLOPT_WRITEDATA, dl);
    curl_easy_setopt(dl->curl, CURLOPT_WRITEFUNCTION, HTTP_Recv);
    curl_easy_setopt(dl->curl, CURLOPT_USERAGENT,
            "OpenTDM (" OPENTDM_VERSION ")");
}

/**
 * Actually starts a download by adding it to the curl multihandle. Returns
 * false if it couldn't be started, the slot is released in that case.
 */
static qboolean HTTP_StartDownload(dlhandle_t *dl) {
    cvar_t *hostname;
    char escapedFilePath[1024 * 3];

    hostname = gi.cvar("hostname", NULL, 0);
    if (!hostname) {
        TDM_Error("HTTP_StartDownload: Couldn't get hostname cvar");
    }

    HTTP_SetupHandle(dl);

    HTTP_EscapePath(dl->filePath, escapedFilePath);

    Com_sprintf(dl->URL, sizeof(dl->URL), "http://%s%s%s", otdm_api_ip,
            g_http_path->string, escapedFilePath);

    curl_easy_setopt(dl->curl, CURLOPT_HTTPHEADER, http_header_slist);
    curl_easy_setopt(dl->curl, CURLOPT_ENCODING, "");
    curl_easy_setopt(dl->curl, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt(dl->curl, CURLOPT_MAXREDIRS, 5);
    curl_easy_setopt(dl->curl, CURLOPT_REFERER, hostname->string);
    curl_easy_setopt(dl->curl, CURLOPT_URL, dl->URL);

    if (curl_multi_add_handle(multi, dl->curl) != CURLM_OK) {
        gi.dprintf("HTTP_StartDownload: curl_multi_add_handle: error\n");
        dl->inuse = false;
        return false;
    }

    handleCount++;
    return true;
}

/**
 * Init libcurl
 */
void HTTP_Init(void) {
    curl_global_init(CURL_GLOBAL_NOTHING);
    multi = curl_multi_init();

    Com_sprintf(hostHeader, sizeof(hostHeader), "Host: %s",
            g_http_domain->string);
    http_header_slist = curl_slist_append(http_header_slist, hostHeader);

    gi.dprintf("%s initialized.\n", curl_version());
}

/**
 *
 */
void HTTP_Shutdown(void) {
    if (multi) {
        curl_multi_cleanup(multi);
        multi = NULL;
    }
    curl_slist_free_all(http_header_slist);
    curl_global_cleanup();
}

/**
 * Release everything a post request holds and free its slot.
 */
static void HTTP_ReleasePost(dlhandle_t *dl) {
    free(dl->postData);
    dl->postData = NULL;
    curl_slist_free_all(dl->postHeaders);
    dl->postHeaders = NULL;
    curl_slist_free_all(dl->postResolve);
    dl->postResolve = NULL;

    if (dl->tempBuffer) {
        gi.TagFree(dl->tempBuffer);
        dl->tempBuffer = NULL;
    }

    dl->inuse = false;
}

/**
 * A post request finished. Nobody waits on the result, just log it.
 */
static void HTTP_FinishPost(dlhandle_t *dl, CURLcode result) {
    long responseCode;

    if (result != CURLE_OK) {
        gi.dprintf("HTTP Error: POST %s: %s\n", dl->URL,
                curl_easy_strerror(result));
    } else {
        curl_easy_getinfo(dl->curl, CURLINFO_RESPONSE_CODE, &responseCode);
        if (responseCode >= 200 && responseCode < 300) {
            gi.dprintf("HTTP: POST %s: %ld\n", dl->URL, responseCode);
        } else {
            gi.dprintf("HTTP Error: POST %s: server returned %ld\n", dl->URL,
                    responseCode);
        }
    }

    curl_multi_remove_handle(multi, dl->curl);
    HTTP_ReleasePost(dl);
}

/**
 * Asynchronously POST a JSON document to url, sending token (if not empty) as
 * a bearer token. Takes ownership of json (which must be malloc'd) and frees
 * it in all cases. Returns false if the request couldn't be started.
 */
qboolean HTTP_PostJSON(const char *url, const char *token, char *json,
        size_t len) {
    unsigned i;
    dlhandle_t *dl;
    char auth[MAX_STRING_CHARS];

    for (i = 0; i < MAX_DOWNLOADS; i++) {
        if (!downloads[i].inuse) {
            break;
        }
    }

    if (i == MAX_DOWNLOADS) {
        gi.dprintf("HTTP_PostJSON: no free request slots, not sending to %s\n",
                url);
        free(json);
        return false;
    }

    dl = &downloads[i];
    HTTP_SetupHandle(dl);

    dl->inuse = true;
    dl->tdm_handle = NULL;
    dl->postData = json;
    Q_strncpy(dl->URL, url, sizeof(dl->URL) - 1);

    // use the pre-resolved address so curl doesn't block on DNS
    HTTP_ResolveStatsServer();
    if (stats_resolve_entry[0] && !strcmp(stats_resolve_url, url)) {
        dl->postResolve = curl_slist_append(NULL, stats_resolve_entry);
        curl_easy_setopt(dl->curl, CURLOPT_RESOLVE, dl->postResolve);
    }

    dl->postHeaders = curl_slist_append(NULL,
            "Content-Type: application/json");
    if (token && token[0]) {
        Com_sprintf(auth, sizeof(auth), "Authorization: Bearer %s", token);
        dl->postHeaders = curl_slist_append(dl->postHeaders, auth);
    }
    curl_easy_setopt(dl->curl, CURLOPT_HTTPHEADER, dl->postHeaders);

    curl_easy_setopt(dl->curl, CURLOPT_URL, dl->URL);
    curl_easy_setopt(dl->curl, CURLOPT_POST, 1L);
    curl_easy_setopt(dl->curl, CURLOPT_POSTFIELDS, dl->postData);
    curl_easy_setopt(dl->curl, CURLOPT_POSTFIELDSIZE_LARGE, (curl_off_t) len);

    // don't verify https certificates, the static libcurl's CA path may not
    // exist on the server
    curl_easy_setopt(dl->curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(dl->curl, CURLOPT_SSL_VERIFYHOST, 0L);

    // don't let a dead stats server hold a slot forever
    curl_easy_setopt(dl->curl, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(dl->curl, CURLOPT_CONNECTTIMEOUT, 10L);
    curl_easy_setopt(dl->curl, CURLOPT_TIMEOUT, 30L);

    if (curl_multi_add_handle(multi, dl->curl) != CURLM_OK) {
        gi.dprintf("HTTP_PostJSON: curl_multi_add_handle: error\n");
        HTTP_ReleasePost(dl);
        return false;
    }

    handleCount++;
    return true;
}

/**
 * A download finished, find out what it was, whether there were any errors and
 * if so, how severe. If none, rename file and other such stuff.
 */
static void HTTP_FinishDownload(void) {
    int msgs_in_queue;
    CURLMsg *msg;
    CURLcode result;
    dlhandle_t *dl;
    CURL *curl;
    long responseCode;
    double timeTaken;
    curl_off_t fileSize;
    unsigned i;

    do {
        msg = curl_multi_info_read(multi, &msgs_in_queue);

        if (!msg) {
            gi.dprintf("HTTP_FinishDownload: Odd, no message for us...\n");
            return;
        }

        if (msg->msg != CURLMSG_DONE) {
            gi.dprintf("HTTP_FinishDownload: Got some weird message...\n");
            continue;
        }

        curl = msg->easy_handle;

        for (i = 0; i < MAX_DOWNLOADS; i++) {
            if (downloads[i].curl == curl)
                break;
        }

        if (i == MAX_DOWNLOADS) {
            TDM_Error("HTTP_FinishDownload: Handle not found!");
        }

        dl = &downloads[i];

        result = msg->data.result;

        if (dl->postData) {
            HTTP_FinishPost(dl, result);
            continue;
        }

        switch (result) {
        //for some reason curl returns CURLE_OK for a 404...
        case CURLE_HTTP_RETURNED_ERROR:
        case CURLE_OK:

            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);
            if (responseCode == 404) {
                TDM_HandleDownload(dl->tdm_handle, NULL, 0, responseCode);
                gi.dprintf("HTTP: %s: 404 File Not Found\n", dl->URL);
                curl_multi_remove_handle(multi, dl->curl);
                dl->inuse = false;
                continue;
            } else if (responseCode == 200) {
                TDM_HandleDownload(dl->tdm_handle, dl->tempBuffer, dl->position,
                        responseCode);
                // an empty response never allocates a buffer
                if (dl->tempBuffer) {
                    gi.TagFree(dl->tempBuffer);
                }
            } else {
                TDM_HandleDownload(dl->tdm_handle, NULL, 0, responseCode);
                if (dl->tempBuffer) {
                    gi.TagFree(dl->tempBuffer);
                }
            }
            break;

            //fatal error
        default:
            TDM_HandleDownload(dl->tdm_handle, NULL, 0, 0);
            gi.dprintf("HTTP Error: %s: %s\n", dl->URL,
                    curl_easy_strerror(result));
            curl_multi_remove_handle(multi, dl->curl);
            dl->inuse = false;
            continue;
        }

        //show some stats
        curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &timeTaken);
        curl_easy_getinfo(curl, CURLINFO_SIZE_DOWNLOAD_T, &fileSize);

        //FIXME:
        //technically i shouldn't need to do this as curl will auto reuse the
        //existing handle when you change the URL. however, the handleCount goes
        //all weird when reusing a download slot in this way. if you can figure
        //out why, please let me know.
        curl_multi_remove_handle(multi, dl->curl);

        dl->inuse = false;

        gi.dprintf("HTTP: Finished %s: %.f bytes, %.2fkB/sec\n", dl->URL,
                (double) fileSize, ((double) fileSize / 1024.0) / timeTaken);
    } while (msgs_in_queue > 0);
}

/**
 *
 */
qboolean HTTP_QueueDownload(tdm_download_t *d) {
    unsigned i;

    if (handleCount == MAX_DOWNLOADS) {
        if (d->type == DL_CONFIG) {
            gi.cprintf(d->initiator, PRINT_HIGH,
                    "Another download is already pending, please try again later.\n");
        }
        return false;
    }

    if (!g_http_enabled->value) {
        if (d->type == DL_CONFIG) {
            gi.cprintf(d->initiator, PRINT_HIGH,
                    "HTTP functions are disabled on this server.\n");
        }
        return false;
    }

    if (!otdm_api_ip[0]) {
        if (d->type == DL_CONFIG) {
            gi.cprintf(d->initiator, PRINT_HIGH,
                    "This server failed to resolve the OpenTDM web API server.\n");
        }
        return false;
    }

    for (i = 0; i < MAX_DOWNLOADS; i++) {
        if (!downloads[i].inuse) {
            break;
        }
    }

    if (i == MAX_DOWNLOADS) {
        if (d->type == DL_CONFIG) {
            gi.cprintf(d->initiator, PRINT_HIGH,
                    "The server is too busy to download configs right now.\n");
        }
        return false;
    }

    downloads[i].tdm_handle = d;
    downloads[i].inuse = true;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-truncation"
    Q_strncpy(downloads[i].filePath, d->path,
            sizeof(downloads[i].filePath) - 1);
#pragma GCC diagnostic pop
    return HTTP_StartDownload(&downloads[i]);
}

/**
 *
 */
void HTTP_RunDownloads(void) {
    int newHandleCount;
    CURLMcode ret;

    if (!handleCount) {
        return;
    }

    do {
        ret = curl_multi_perform(multi, &newHandleCount);
        if (newHandleCount < handleCount) {
            HTTP_FinishDownload();
            handleCount = newHandleCount;
        }
    } while (ret == CURLM_CALL_MULTI_PERFORM);

    if (ret != CURLM_OK) {
        gi.dprintf("HTTP_RunDownloads: curl_multi_perform error.\n");
    }
}
#else

/**
 *
 */
void HTTP_RunDownloads(void) {
}

/**
 *
 */
void HTTP_Init(void) {
    gi.dprintf(
            "WARNING: OpenTDM was built without libcurl. Some features will be unavailable.\n");
}

/**
 *
 */
qboolean HTTP_QueueDownload(tdm_download_t *d) {
    if (d->type == DL_CONFIG)
        gi.cprintf(d->initiator, PRINT_HIGH,
                "HTTP functions are not compiled on this server.\n");
    return false;
}

/**
 *
 */
void HTTP_ResolveOTDMServer(void) {
}

/**
 *
 */
void HTTP_ResolveStatsServer(void) {
}

/**
 *
 */
qboolean HTTP_PostJSON(const char *url, const char *token, char *json,
        size_t len) {
    gi.dprintf("Not sending match stats, OpenTDM was built without libcurl.\n");
    free(json);
    return false;
}
#endif
