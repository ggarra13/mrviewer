#if defined(__APPLE__)

# include <objc/runtime.h>
# include <objc/message.h>

/* Globals */
static int osx_latencycritical_count = 0;
static id osx_latencycritical_activity = nil;

/* Tell App Nap that this is latency critical */
void osx_latencycritical_start() {
    Class pic;      /* Process info class */
    SEL pisl;       /* Process info selector */
    SEL bawo;       /* Begin Activity With Options selector */
    id pi;          /* Process info */
    id str;         /* Reason string */

    if (osx_latencycritical_count++ != 0)
        return;

    /* Avoid triggering an exception when run on older OS X */
    if ((pic = (Class)objc_getClass("NSProcessInfo")) == nil)
        return;

    if (class_getClassMethod(pic, (pisl = sel_getUid("processInfo"))) == NULL)
        return;

    if (class_getInstanceMethod(pic,
          (bawo = sel_getUid("beginActivityWithOptions:reason:"))) == NULL)
        return;

    /* Get the process instance */
    if ((pi = objc_msgSend((id)pic, pisl)) == nil)
        return;

    /* Create a reason string */
    str = objc_msgSend(objc_getClass("NSString"), sel_getUid("alloc"));
    str = objc_msgSend(str, sel_getUid("initWithUTF8String:"), "Timing Crititcal");

    /* Start activity that tells App Nap to mind its own business: */
    /* NSActivityUserInitiatedAllowingIdleSystemSleep */
    /* | NSActivityLatencyCritical */
    osx_latencycritical_activity = objc_msgSend(pi, bawo,
                    0x00FFFFFFULL | 0xFF00000000ULL, str);
}

/* Done with latency critical */
void osx_latencycritical_end() {
    if (osx_latencycritical_count > 0) {
        osx_latencycritical_count--;
        if (osx_latencycritical_count == 0
         && osx_latencycritical_activity != nil) {
            objc_msgSend(
                         objc_msgSend(objc_getClass("NSProcessInfo"),
                                      sel_getUid("processInfo")),
                                      sel_getUid("endActivity:"),
                                      osx_latencycritical_activity);
            osx_latencycritical_activity = nil;
        }
    }
}

#endif /* __APPLE__ */
