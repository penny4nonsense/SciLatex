#ifdef _WIN32
#include <windows.h>
bool isCapsLockActive() {
    return (GetKeyState(VK_CAPITAL) & 0x0001) != 0;
}
#elif __linux__
#include <X11/XKBlib.h>
bool isCapsLockActive() {
    Display *display = XOpenDisplay(nullptr);
    if (!display) return false;

    unsigned int state;
    XkbGetIndicatorState(display, XkbUseCoreKbd, &state);
    bool capsLock = (state & 0x01) != 0;
    XCloseDisplay(display);
    return capsLock;
}
#elif __APPLE__
#include <CoreGraphics/CoreGraphics.h>
bool isCapsLockActive() {
    return (CGEventSourceFlagsState(kCGEventSourceStateHIDSystemState) & kCGEventFlagMaskAlphaShift) != 0;
}
#else
bool isCapsLockActive() {
    return false; // Default to off if unsupported
}
#endif
