/*
    espFramework.c -- ESP Web Framework API

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

/********************************** Includes **********************************/

#include    "esp.h"

/************************************* Code ***********************************/

PUBLIC void espAddPak(HttpRoute *route, cchar *name, cchar *version)
{
    if (!version || !*version || smatch(version, "0.0.0")) {
        version = "*";
    }
    mprSetJson(route->config, sfmt("dependencies.%s", name), version);
}


/*
    Add a http header if not already defined
 */
PUBLIC void espAddHeader(HttpConn *conn, cchar *key, cchar *fmt, ...)
{
    va_list     vargs;

    assert(key && *key);
    assert(fmt && *fmt);

    va_start(vargs, fmt);
    httpAddHeaderString(conn, key, sfmt(fmt, vargs));
    va_end(vargs);
}


/*
    Add a header string if not already defined
 */
PUBLIC void espAddHeaderString(HttpConn *conn, cchar *key, cchar *value)
{
    httpAddHeaderString(conn, key, value);
}


PUBLIC void espAddParam(HttpConn *conn, cchar *var, cchar *value)
{
    if (!httpGetParam(conn, var, 0)) {
        httpSetParam(conn, var, value);
    }
}


/*
   Append a header. If already defined, the value is catenated to the pre-existing value after a ", " separator.
   As per the HTTP/1.1 spec.
 */
PUBLIC void espAppendHeader(HttpConn *conn, cchar *key, cchar *fmt, ...)
{
    va_list     vargs;

    assert(key && *key);
    assert(fmt && *fmt);

    va_start(vargs, fmt);
    httpAppendHeaderString(conn, key, sfmt(fmt, vargs));
    va_end(vargs);
}


/*
   Append a header string. If already defined, the value is catenated to the pre-existing value after a ", " separator.
   As per the HTTP/1.1 spec.
 */
PUBLIC void espAppendHeaderString(HttpConn *conn, cchar *key, cchar *value)
{
    httpAppendHeaderString(conn, key, value);
}


PUBLIC void espAutoFinalize(HttpConn *conn)
{
    EspReq  *req;

    req = conn->reqData;
    if (req->autoFinalize) {
        httpFinalize(conn);
    }
}


PUBLIC int espCache(HttpRoute *route, cchar *uri, int lifesecs, int flags)
{
    httpAddCache(route, NULL, uri, NULL, NULL, 0, lifesecs * MPR_TICKS_PER_SEC, flags);
    return 0;
}


PUBLIC cchar *espCreateSession(HttpConn *conn)
{
    HttpSession *session;

    if ((session = httpCreateSession(getConn())) != 0) {
        return session->id;
    }
    return 0;
}


PUBLIC void espDefineAction(HttpRoute *route, cchar *target, void *action)
{
    Esp     *esp;

    assert(route);
    assert(target && *target);
    assert(action);

    esp = MPR->espService;
    if (target) {
        mprAddKey(esp->actions, mprJoinPath(httpGetDir(route, "controllers"), target), action);
    }
}


/*
    The base procedure is invoked prior to calling any and all actions on this route
 */
PUBLIC void espDefineBase(HttpRoute *route, EspProc baseProc)
{
    HttpRoute   *rp;
    EspRoute    *er;
    int         next;

    for (ITERATE_ITEMS(route->host->routes, rp, next)) {
        if ((er = route->eroute) != 0) {
            if (smatch(httpGetDir(rp, "controllers"), httpGetDir(route, "controllers"))) {
                er->commonController = baseProc;
            }
        }
    }
}


/*
    Path should be an app-relative path to the view file (relative-path.esp)
 */
PUBLIC void espDefineView(HttpRoute *route, cchar *path, void *view)
{
    Esp         *esp;

    assert(path && *path);
    assert(view);

    esp = MPR->espService;
    if (route) {
        path = mprGetPortablePath(mprJoinPath(route->documents, path));
    }
    mprAddKey(esp->views, path, view);
}


PUBLIC void espDestroySession(HttpConn *conn)
{
    httpDestroySession(conn);
}


PUBLIC void espFinalize(HttpConn *conn)
{
    httpFinalize(conn);
}


PUBLIC void espFlush(HttpConn *conn)
{
    httpFlush(conn);
}


PUBLIC HttpAuth *espGetAuth(HttpConn *conn)
{
    return conn->rx->route->auth;
}


PUBLIC cchar *espGetConfig(HttpRoute *route, cchar *key, cchar *defaultValue)
{
    cchar       *value;

    if ((value = mprGetJson(route->config, key)) != 0) {
        return value;
    }
    return defaultValue;
}


PUBLIC MprOff espGetContentLength(HttpConn *conn)
{
    return httpGetContentLength(conn);
}


PUBLIC cchar *espGetContentType(HttpConn *conn)
{
    return conn->rx->mimeType;
}


PUBLIC cchar *espGetCookie(HttpConn *conn, cchar *name)
{
    return httpGetCookie(conn, name);
}


PUBLIC cchar *espGetCookies(HttpConn *conn)
{
    return httpGetCookies(conn);
}


PUBLIC void *espGetData(HttpConn *conn)
{
    EspReq  *req;

    req = conn->reqData;
    return req->data;
}


PUBLIC Edi *espGetDatabase(HttpConn *conn)
{
    HttpRx      *rx;
    EspReq      *req;
    EspRoute    *eroute;
    Edi         *edi;

    rx = conn->rx;
    req = conn->reqData;
    edi = req ? req->edi : 0;
    if (edi == 0 && rx && rx->route) {
        if ((eroute = rx->route->eroute) != 0) {
            edi = eroute->edi;
        }
    }
    if (edi == 0) {
        httpError(conn, 0, "Cannot get database instance");
        return 0;
    }
    return edi;
}


PUBLIC cchar *espGetDocuments(HttpConn *conn)
{
    return conn->rx->route->documents;
}


PUBLIC EspRoute *espGetEspRoute(HttpConn *conn)
{
    return conn->rx->route->eroute;
}


PUBLIC cchar *espGetFlash(HttpConn *conn, cchar *kind)
{
    EspReq      *req;
    MprKey      *kp;
    cchar       *msg;

    req = conn->reqData;
    if (kind == 0 || req->flash == 0 || mprGetHashLength(req->flash) == 0) {
        return 0;
    }
    for (kp = 0; (kp = mprGetNextKey(req->flash, kp)) != 0; ) {
        msg = kp->data;
        if (smatch(kind, kp->key) || smatch(kind, "all")) {
            return msg;
        }
    }
    return 0;
}


PUBLIC cchar *espGetFeedback(HttpConn *conn, cchar *kind)
{
    EspReq      *req;
    MprKey      *kp;
    cchar       *msg;

    req = conn->reqData;
    if (kind == 0 || req == 0 || req->feedback == 0 || mprGetHashLength(req->feedback) == 0) {
        return 0;
    }
    for (kp = 0; (kp = mprGetNextKey(req->feedback, kp)) != 0; ) {
        msg = kp->data;
        if (smatch(kind, kp->key) || smatch(kind, "all")) {
            return msg;
        }
    }
    return 0;
}


PUBLIC EdiGrid *espGetGrid(HttpConn *conn)
{
    return conn->grid;
}


PUBLIC cchar *espGetHeader(HttpConn *conn, cchar *key)
{
    return httpGetHeader(conn, key);
}


PUBLIC MprHash *espGetHeaderHash(HttpConn *conn)
{
    return httpGetHeaderHash(conn);
}


PUBLIC char *espGetHeaders(HttpConn *conn)
{
    return httpGetHeaders(conn);
}


PUBLIC int espGetIntParam(HttpConn *conn, cchar *var, int defaultValue)
{
    return httpGetIntParam(conn, var, defaultValue);
}


PUBLIC cchar *espGetMethod(HttpConn *conn)
{
    return conn->rx->method;
}


PUBLIC cchar *espGetParam(HttpConn *conn, cchar *var, cchar *defaultValue)
{
    return httpGetParam(conn, var, defaultValue);
}


PUBLIC MprJson *espGetParams(HttpConn *conn)
{
    return httpGetParams(conn);
}


PUBLIC cchar *espGetQueryString(HttpConn *conn)
{
    return httpGetQueryString(conn);
}


PUBLIC char *espGetReferrer(HttpConn *conn)
{
    if (conn->rx->referrer) {
        return conn->rx->referrer;
    }
    return httpLink(conn, "~");
}


PUBLIC HttpRoute *espGetRoute(HttpConn *conn)
{
    return conn->rx->route;
}


PUBLIC Edi *espGetRouteDatabase(HttpRoute *route)
{
    EspRoute    *eroute;

    eroute = route->eroute;
    if (eroute == 0 || eroute->edi == 0) {
        return 0;
    }
    return eroute->edi;
}


PUBLIC cchar *espGetSessionID(HttpConn *conn, int create)
{
    HttpSession *session;

    if ((session = httpGetSession(getConn(), create)) != 0) {
        return session->id;
    }
    return 0;
}


PUBLIC int espGetStatus(HttpConn *conn)
{
    return httpGetStatus(conn);
}


PUBLIC char *espGetStatusMessage(HttpConn *conn)
{
    return httpGetStatusMessage(conn);
}


PUBLIC MprList *espGetUploads(HttpConn *conn)
{
    return conn->rx->files;
}


PUBLIC cchar *espGetUri(HttpConn *conn)
{
    return conn->rx->uri;
}


PUBLIC bool espHasPak(HttpRoute *route, cchar *name)
{
    return mprGetJsonObj(route->config, sfmt("dependencies.%s", name)) != 0;
}


PUBLIC bool espHasGrid(HttpConn *conn)
{
    return conn->grid != 0;
}


PUBLIC bool espHasRec(HttpConn *conn)
{
    EdiRec  *rec;

    rec = conn->record;
    return (rec && rec->id) ? 1 : 0;
}


PUBLIC bool espIsEof(HttpConn *conn)
{
    return httpIsEof(conn);
}


PUBLIC bool espIsFinalized(HttpConn *conn)
{
    return httpIsFinalized(conn);
}


PUBLIC bool espIsSecure(HttpConn *conn)
{
    return conn->secure;
}


PUBLIC bool espMatchParam(HttpConn *conn, cchar *var, cchar *value)
{
    return httpMatchParam(conn, var, value);
}


/*
    Read rx data in non-blocking mode. Use standard connection timeouts.
 */
PUBLIC ssize espReceive(HttpConn *conn, char *buf, ssize len)
{
    return httpRead(conn, buf, len);
}


PUBLIC void espRedirect(HttpConn *conn, int status, cchar *target)
{
    httpRedirect(conn, status, target);
}


PUBLIC void espRedirectBack(HttpConn *conn)
{
    if (conn->rx->referrer) {
        espRedirect(conn, HTTP_CODE_MOVED_TEMPORARILY, conn->rx->referrer);
    }
}


PUBLIC ssize espRender(HttpConn *conn, cchar *fmt, ...)
{
    va_list     vargs;
    char        *buf;

    va_start(vargs, fmt);
    buf = sfmtv(fmt, vargs);
    va_end(vargs);
    return espRenderString(conn, buf);
}


PUBLIC ssize espRenderBlock(HttpConn *conn, cchar *buf, ssize size)
{
    return httpWriteBlock(conn->writeq, buf, size, HTTP_BUFFER);
}


PUBLIC ssize espRenderCached(HttpConn *conn)
{
    return httpWriteCached(conn);
}


PUBLIC ssize espRenderError(HttpConn *conn, int status, cchar *fmt, ...)
{
    va_list     args;
    HttpRx      *rx;
    ssize       written;
    cchar       *msg, *title, *text;

    va_start(args, fmt);

    rx = conn->rx;
    written = 0;

    if (!httpIsFinalized(conn)) {
        if (status == 0) {
            status = HTTP_CODE_INTERNAL_SERVER_ERROR;
        }
        title = sfmt("Request Error for \"%s\"", rx->pathInfo);
        msg = mprEscapeHtml(sfmtv(fmt, args));
        if (rx->route->flags & HTTP_ROUTE_SHOW_ERRORS) {
            text = sfmt(\
                "<!DOCTYPE html>\r\n<html>\r\n<head><title>%s</title></head>\r\n" \
                "<body>\r\n<h1>%s</h1>\r\n" \
                "    <pre>%s</pre>\r\n" \
                "    <p>To prevent errors being displayed in the browser, " \
                "       set <b>ShowErrors off</b> in the appweb.conf file.</p>\r\n" \
                "</body>\r\n</html>\r\n", title, title, msg);
            httpSetHeader(conn, "Content-Type", "text/html");
            written += espRenderString(conn, text);
            espFinalize(conn);
            httpTrace(conn, "esp.error", "error", "msg=\"%s\", status=%d, uri=\"%s\"", msg, status, rx->pathInfo);
        }
    }
    va_end(args);
    return written;
}


PUBLIC ssize espRenderFile(HttpConn *conn, cchar *path)
{
    MprFile     *from;
    ssize       count, written, nbytes;
    char        buf[ME_MAX_BUFFER];

    if ((from = mprOpenFile(path, O_RDONLY | O_BINARY, 0)) == 0) {
        return MPR_ERR_CANT_OPEN;
    }
    written = 0;
    while ((count = mprReadFile(from, buf, sizeof(buf))) > 0) {
        if ((nbytes = espRenderBlock(conn, buf, count)) < 0) {
            return nbytes;
        }
        written += nbytes;
    }
    mprCloseFile(from);
    return written;
}


PUBLIC void espRenderFlash(HttpConn *conn, cchar *kinds)
{
    EspReq      *req;
    MprKey      *kp;
    cchar       *msg;

    req = conn->reqData;
    if (kinds == 0 || req->flash == 0 || mprGetHashLength(req->flash) == 0) {
        return;
    }
    for (kp = 0; (kp = mprGetNextKey(req->flash, kp)) != 0; ) {
        msg = kp->data;
        if (strstr(kinds, kp->key) || strstr(kinds, "all")) {
            espRender(conn, "<span class='feedback-%s animate'>%s</span>", kp->key, msg);
        }
    }
}


PUBLIC void espRemoveCookie(HttpConn *conn, cchar *name)
{
    httpSetCookie(conn, name, "", "/", NULL, -1, 0);
}


PUBLIC void espSetConn(HttpConn *conn)
{
    mprSetThreadData(((Esp*) MPR->espService)->local, conn);
}


static void espNotifier(HttpConn *conn, int event, int arg)
{
    EspReq      *req;

    if ((req = conn->reqData) != 0) {
        espSetConn(conn);
        (req->notifier)(conn, event, arg);
    }
}


PUBLIC void espSetNotifier(HttpConn *conn, HttpNotifier notifier)
{
    EspReq      *req;

    if ((req = conn->reqData) != 0) {
        req->notifier = notifier;
        httpSetConnNotifier(conn, espNotifier);
    }
}


PUBLIC ssize espRenderSafe(HttpConn *conn, cchar *fmt, ...)
{
    va_list     args;
    cchar       *s;

    va_start(args, fmt);
    s = mprEscapeHtml(sfmtv(fmt, args));
    va_end(args);
    return espRenderBlock(conn, s, slen(s));
}


PUBLIC ssize espRenderSafeString(HttpConn *conn, cchar *s)
{
    s = mprEscapeHtml(s);
    return espRenderBlock(conn, s, slen(s));
}


PUBLIC ssize espRenderString(HttpConn *conn, cchar *s)
{
    return espRenderBlock(conn, s, slen(s));
}


/*
    Render a request variable. If a param by the given name is not found, consult the session.
 */
PUBLIC ssize espRenderVar(HttpConn *conn, cchar *name)
{
    cchar   *value;

    if ((value = espGetParam(conn, name, 0)) == 0) {
        value = httpGetSessionVar(conn, name, "");
    }
    return espRenderSafeString(conn, value);
}


PUBLIC int espRemoveHeader(HttpConn *conn, cchar *key)
{
    assert(key && *key);
    if (conn->tx == 0) {
        return MPR_ERR_CANT_ACCESS;
    }
    return mprRemoveKey(conn->tx->headers, key);
}


PUBLIC void espRemoveSessionVar(HttpConn *conn, cchar *var)
{
    httpRemoveSessionVar(conn, var);
}


PUBLIC int espSaveConfig(HttpRoute *route)
{
    cchar       *path;

    path = mprJoinPath(route->documents, ME_ESP_PACKAGE);
#if KEEP
    mprBackupLog(path, 3);
#endif
    return mprSaveJson(route->config, path, MPR_JSON_PRETTY | MPR_JSON_QUOTES);
}


PUBLIC ssize espSendGrid(HttpConn *conn, EdiGrid *grid, int flags)
{
    httpAddHeaderString(conn, "Content-Type", "application/json");
    if (grid) {
        return espRender(conn, "{\n  \"data\": %s, \"schema\": %s}\n", ediGridAsJson(grid, flags), ediGetGridSchemaAsJson(grid));
    }
    return espRender(conn, "{}");
}


PUBLIC ssize espSendRec(HttpConn *conn, EdiRec *rec, int flags)
{
    httpAddHeaderString(conn, "Content-Type", "application/json");
    if (rec) {
        return espRender(conn, "{\n  \"data\": %s, \"schema\": %s}\n", ediRecAsJson(rec, flags), ediGetRecSchemaAsJson(rec));
    }
    return espRender(conn, "{}");
}


PUBLIC void espSendResult(HttpConn *conn, bool success)
{
    EspReq      *req;
    EdiRec      *rec;

    req = conn->reqData;
    rec = getRec();
    if (rec && rec->errors) {
        espRender(conn, "{\"error\": %d, \"feedback\": %s, \"fieldErrors\": %s}", !success,
            req->feedback ? mprSerialize(req->feedback, MPR_JSON_QUOTES) : "{}",
            mprSerialize(rec->errors, MPR_JSON_QUOTES));
    } else {
        espRender(conn, "{\"error\": %d, \"feedback\": %s}", !success,
            req->feedback ? mprSerialize(req->feedback, MPR_JSON_QUOTES) : "{}");
    }
    espFinalize(conn);
}


PUBLIC bool espSetAutoFinalizing(HttpConn *conn, bool on)
{
    EspReq  *req;
    bool    old;

    req = conn->reqData;
    old = req->autoFinalize;
    req->autoFinalize = on;
    return old;
}


PUBLIC int espSetConfig(HttpRoute *route, cchar *key, cchar *value)
{
    return mprSetJson(route->config, key, value);
}


PUBLIC void espSetContentLength(HttpConn *conn, MprOff length)
{
    httpSetContentLength(conn, length);
}


PUBLIC void espSetCookie(HttpConn *conn, cchar *name, cchar *value, cchar *path, cchar *cookieDomain, MprTicks lifespan,
        bool isSecure)
{
    httpSetCookie(conn, name, value, path, cookieDomain, lifespan, isSecure);
}


PUBLIC void espSetContentType(HttpConn *conn, cchar *mimeType)
{
    httpSetContentType(conn, mimeType);
}


PUBLIC void espSetData(HttpConn *conn, void *data)
{
    EspReq  *req;

    req = conn->reqData;
    req->data = data;
}


PUBLIC void espSetFeedback(HttpConn *conn, cchar *kind, cchar *fmt, ...)
{
    va_list     args;

    va_start(args, fmt);
    espSetFeedbackv(conn, kind, fmt, args);
    va_end(args);
}


PUBLIC void espSetFeedbackv(HttpConn *conn, cchar *kind, cchar *fmt, va_list args)
{
    EspReq      *req;
    cchar       *prior, *msg;

    if ((req = conn->reqData) == 0) {
        return;
    }
    msg = sfmtv(fmt, args);

    if (req->feedback == 0) {
        req->feedback = mprCreateHash(0, MPR_HASH_STABLE);
    }
    if ((prior = mprLookupKey(req->feedback, kind)) != 0) {
        mprAddKey(req->feedback, kind, sjoin(prior, ", ", msg, NULL));
    } else {
        mprAddKey(req->feedback, kind, sclone(msg));
    }
}


PUBLIC void espSetFlash(HttpConn *conn, cchar *kind, cchar *fmt, ...)
{
    va_list     args;

    va_start(args, fmt);
    espSetFlashv(conn, kind, fmt, args);
    va_end(args);
}


PUBLIC void espSetFlashv(HttpConn *conn, cchar *kind, cchar *fmt, va_list args)
{
    EspReq      *req;
    cchar       *msg;

    req = conn->reqData;
    msg = sfmtv(fmt, args);

    if (req->flash == 0) {
        req->flash = mprCreateHash(0, MPR_HASH_STABLE);
    }
    mprAddKey(req->flash, kind, sclone(msg));
    /*
        Create a session as early as possible so a Set-Cookie header can be omitted.
     */
    httpGetSession(conn, 1);
}


PUBLIC EdiGrid *espSetGrid(HttpConn *conn, EdiGrid *grid)
{
    return conn->grid = grid;
}


/*
    Set a http header. Overwrite if present.
 */
PUBLIC void espSetHeader(HttpConn *conn, cchar *key, cchar *fmt, ...)
{
    va_list     vargs;

    assert(key && *key);
    assert(fmt && *fmt);

    va_start(vargs, fmt);
    httpSetHeaderString(conn, key, sfmtv(fmt, vargs));
    va_end(vargs);
}


PUBLIC void espSetHeaderString(HttpConn *conn, cchar *key, cchar *value)
{
    httpSetHeaderString(conn, key, value);
}


PUBLIC void espSetIntParam(HttpConn *conn, cchar *var, int value)
{
    httpSetIntParam(conn, var, value);
}


PUBLIC void espSetParam(HttpConn *conn, cchar *var, cchar *value)
{
    httpSetParam(conn, var, value);
}


PUBLIC EdiRec *espSetRec(HttpConn *conn, EdiRec *rec)
{
    return conn->record = rec;
}


PUBLIC int espSetSessionVar(HttpConn *conn, cchar *var, cchar *value)
{
    return httpSetSessionVar(conn, var, value);
}


PUBLIC void espSetStatus(HttpConn *conn, int status)
{
    httpSetStatus(conn, status);
}


PUBLIC void espShowRequest(HttpConn *conn)
{
    MprHash     *env;
    MprJson     *params, *param;
    MprKey      *kp;
    MprJson     *jkey;
    HttpRx      *rx;
    int         i;

    rx = conn->rx;
    httpAddHeaderString(conn, "Cache-Control", "no-cache");
    httpCreateCGIParams(conn);
    espRender(conn, "\r\n");

    /*
        Query
     */
    for (ITERATE_JSON(rx->params, jkey, i)) {
        espRender(conn, "PARAMS %s=%s\r\n", jkey->name, jkey->value ? jkey->value : "null");
    }
    espRender(conn, "\r\n");

    /*
        Http Headers
     */
    env = espGetHeaderHash(conn);
    for (ITERATE_KEYS(env, kp)) {
        espRender(conn, "HEADER %s=%s\r\n", kp->key, kp->data ? kp->data: "null");
    }
    espRender(conn, "\r\n");

    /*
        Server vars
     */
    for (ITERATE_KEYS(conn->rx->svars, kp)) {
        espRender(conn, "SERVER %s=%s\r\n", kp->key, kp->data ? kp->data: "null");
    }
    espRender(conn, "\r\n");

    /*
        Form vars
     */
    if ((params = espGetParams(conn)) != 0) {
        for (ITERATE_JSON(params, param, i)) {
            espRender(conn, "FORM %s=%s\r\n", param->name, param->value);
        }
        espRender(conn, "\r\n");
    }

#if KEEP
    /*
        Body
     */
    q = conn->readq;
    if (q->first && rx->bytesRead > 0 && scmp(rx->mimeType, "application/x-www-form-urlencoded") == 0) {
        buf = q->first->content;
        mprAddNullToBuf(buf);
        if ((numKeys = getParams(&keys, mprGetBufStart(buf), (int) mprGetBufLength(buf))) > 0) {
            for (i = 0; i < (numKeys * 2); i += 2) {
                value = keys[i+1];
                espRender(conn, "BODY %s=%s\r\n", keys[i], value ? value: "null");
            }
        }
        espRender(conn, "\r\n");
    }
#endif
}


PUBLIC bool espTestConfig(HttpRoute *route, cchar *key, cchar *desired)
{
    cchar       *value;

    if ((value = mprGetJson(route->config, key)) != 0) {
        return smatch(value, desired);
    }
    return 0;
}


PUBLIC void espUpdateCache(HttpConn *conn, cchar *uri, cchar *data, int lifesecs)
{
    httpUpdateCache(conn, uri, data, lifesecs * MPR_TICKS_PER_SEC);
}


PUBLIC cchar *espUri(HttpConn *conn, cchar *target)
{
    return httpLink(conn, target);
}


PUBLIC int espEmail(HttpConn *conn, cchar *to, cchar *from, cchar *subject, MprTime date, cchar *mime, cchar *message, MprList *files)
{
    MprList         *lines;
    MprCmd          *cmd;
    cchar           *body, *boundary, *contents, *encoded, *file;
    char            *out, *err;
    ssize           length;
    int             i, next, status;

    if (!from || !*from) {
        from = "anonymous";
    }
    if (!subject || !*subject) {
        subject = "Mail message";
    }
    if (!mime || !*mime) {
        mime = "text/plain";
    }
    if (!date) {
        date = mprGetTime();
    }
    boundary = sjoin("esp.mail=", mprGetMD5("BOUNDARY"), NULL);
    lines = mprCreateList(0, 0);

    mprAddItem(lines, sfmt("To: %s", to));
    mprAddItem(lines, sfmt("From: %s", from));
    mprAddItem(lines, sfmt("Date: %s", mprFormatLocalTime(0, date)));
    mprAddItem(lines, sfmt("Subject: %s", subject));
    mprAddItem(lines, "MIME-Version: 1.0");
    mprAddItem(lines, sfmt("Content-Type: multipart/mixed; boundary=%s", boundary));
    mprAddItem(lines, "");

    boundary = sjoin("--", boundary, NULL);

    mprAddItem(lines, boundary);
    mprAddItem(lines, sfmt("Content-Type: %s", mime));
    mprAddItem(lines, "");
    mprAddItem(lines, "");
    mprAddItem(lines, message);

    for (ITERATE_ITEMS(files, file, next)) {
        mprAddItem(lines, boundary);
        if ((mime = mprLookupMime(NULL, file)) == 0) {
            mime = "application/octet-stream";
        }
        mprAddItem(lines, "Content-Transfer-Encoding: base64");
        mprAddItem(lines, sfmt("Content-Disposition: inline; filename=\"%s\"", mprGetPathBase(file)));
        mprAddItem(lines, sfmt("Content-Type: %s; name=\"%s\"", mime, mprGetPathBase(file)));
        mprAddItem(lines, "");
        contents = mprReadPathContents(file, &length);
        encoded = mprEncode64Block(contents, length);
        for (i = 0; i < length; i += 76) {
            mprAddItem(lines, snclone(&encoded[i], i + 76));
        }
    }
    mprAddItem(lines, sfmt("%s--", boundary));

    body = mprListToString(lines, "\n");
    httpTraceContent(conn, "esp.email", "context", body, slen(body), 0);

    cmd = mprCreateCmd(conn->dispatcher);
    if (mprRunCmd(cmd, "sendmail -t", NULL, body, &out, &err, 0, 0) < 0) {
        return MPR_ERR_CANT_OPEN;
    }
    if (mprWaitForCmd(cmd, ME_ESP_EMAIL_TIMEOUT) < 0) {
        httpTrace(conn, "esp.email.error", "error", 
            "msg=\"Timeout waiting for command to complete\", timeout=%d, command=\"%s\"",
            ME_ESP_EMAIL_TIMEOUT, cmd->argv[0]);
        return MPR_ERR_CANT_COMPLETE;
    }
    if ((status = mprGetCmdExitStatus(cmd)) != 0) {
        httpTrace(conn, "esp.email.error", "error", "msg=\"Sendmail failed\", status=%d, error=\"%s\"", status, err);
        return MPR_ERR_CANT_WRITE;
    }
    return 0;
}


//  TODO - rename espSingularClear (and trace)
PUBLIC void espClearCurrentSession(HttpConn *conn)
{
    EspRoute    *eroute;

    eroute = conn->rx->route->eroute;
    if (eroute->currentSession) {
        httpTrace(conn, "esp.singular.clear", "context", "session=%s", eroute->currentSession);
    }
    eroute->currentSession = 0;
}


/*
    Remember this connections session as the current session. Use for single login tracking.
 */
PUBLIC void espSetCurrentSession(HttpConn *conn)
{
    EspRoute    *eroute;

    eroute = conn->rx->route->eroute;
    eroute->currentSession = httpGetSessionID(conn);
    httpTrace(conn, "esp.singular.set", "context", "msg=\"Set singluar user\", session=%s", eroute->currentSession);
}


/*
    Test if this connection is the current session. Use for single login tracking.
 */
PUBLIC bool espIsCurrentSession(HttpConn *conn)
{
    EspRoute    *eroute;

    eroute = conn->rx->route->eroute;
    if (eroute->currentSession) {
        if (smatch(httpGetSessionID(conn), eroute->currentSession)) {
            return 1;
        }
        if (httpLookupSessionID(eroute->currentSession)) {
            /* Session is still current */
            return 0;
        }
        /* Session has expired */
        eroute->currentSession = 0;
    }
    return 1;
}

/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2014. All Rights Reserved.

    This software is distributed under commercial and open source licenses.
    You may use the Embedthis Open Source license or you may acquire a
    commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.md distributed with
    this software for full details and other copyrights.

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */
