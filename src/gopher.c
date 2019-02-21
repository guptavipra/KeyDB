/*
 * Copyright (c) 2019, Salvatore Sanfilippo <antirez at gmail dot com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Redis nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "server.h"

/* Emit an item in Gopher directory listing format:
 * <type><descr><TAB><selector><TAB><hostname><TAB><port>
 * If descr or selector are NULL, then the "(NULL)" string is used instead. */
void addReplyGopherItem(client *c, const char *type, const char *descr,
                        const char *selector, const char *hostname, int port)
{
    sds item = sdscatfmt(sdsempty(),"%s%s\t%s\t%s\t%i\r\n",
                         type, descr,
                         selector ? selector : "(NULL)",
                         hostname ? hostname : "(NULL)",
                         port);
    addReplyProto(c,item,sdslen(item));
    sdsfree(item);
}

/* This is called by processInputBuffer() when an inline request is processed
 * with Gopher mode enabled, and the request happens to have zero or just one
 * argument. In such case we get the relevant key and reply using the Gopher
 * protocol. */
void processGopherRequest(client *c) {
    robj *keyname = c->argc == 0 ? createStringObject("/",1) : c->argv[1];
    robj *o = lookupKeyRead(c->db,keyname);

    /* If there is no such key, return with a Gopher error. */
    if (o == NULL || o->type != OBJ_STRING) {
        char *errstr;
        if (o == NULL)
            errstr = "Error: no content at the specified key";
        else
            errstr = "Error: selected key type is invalid "
                     "for Gopher output";
        addReplyGopherItem(c,"i",errstr,NULL,NULL,0);
        addReplyGopherItem(c,"i","Redis Gopher server",NULL,NULL,0);
    } else {
    }

    /* Cleanup, also make sure to emit the final ".CRLF" line. Note that
     * the connection will be closed immediately after this because the client
     * will be flagged with CLIENT_CLOSE_AFTER_REPLY, in accordance with the
     * Gopher protocol. */
    if (c->argc == 0) decrRefCount(keyname);
    addReplyProto(c,".\r\n",3);
}
