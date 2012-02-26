/*
 *  Copyright (c) 2009-2011, NVIDIA Corporation
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *      * Redistributions of source code must retain the above copyright
 *        notice, this list of conditions and the following disclaimer.
 *      * Redistributions in binary form must reproduce the above copyright
 *        notice, this list of conditions and the following disclaimer in the
 *        documentation and/or other materials provided with the distribution.
 *      * Neither the name of NVIDIA Corporation nor the
 *        names of its contributors may be used to endorse or promote products
 *        derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 *  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 *  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once
#include "base/Array.hpp"
#include "base/String.hpp"
#include "base/Math.hpp"
#include "base/Hash.hpp"
#include "gui/Keys.hpp"
#include "base/DLLImports.hpp"
#include "gpu/GLContext.hpp"

#ifdef FW_QT
class QWindow;
#endif

namespace FW
{
//------------------------------------------------------------------------

class Window
{
public:

    //------------------------------------------------------------------------

    enum EventType
    {
        EventType_AddListener,      /* Listener has been added to a window. */
        EventType_RemoveListener,   /* Listener has been removed from a window. */
        EventType_Close,            /* User has tried to close the window. */
        EventType_Resize,           /* The window has been resized. */
        EventType_KeyDown,          /* User has pressed a key (or mouse button). */
        EventType_KeyUp,            /* User has released a key (or mouse button). */
        EventType_Char,             /* User has typed a character. */
        EventType_Mouse,            /* User has moved the mouse. */
        EventType_Paint,            /* Window contents need to be painted. */
        EventType_PrePaint,         /* Before processing EventType_Paint. */
        EventType_PostPaint,        /* After processing EventType_Paint. */
    };

    //------------------------------------------------------------------------

    struct Event
    {
        EventType           type;
        String              key;            /* Empty unless EventType_KeyDown or EventType_KeyUp. */
        S32                 keyUnicode;     /* 0 unless EventType_KeyDown or EventType_KeyUp or if special key. */
        S32                 chr;            /* Zero unless EventType_Text. */
        bool                mouseKnown;     /* Unchanged unless EventType_Mouse. */
        Vec2i               mousePos;       /* Unchanged unless EventType_Mouse. */
        Vec2i               mouseDelta;     /* Zero unless EventType_Mouse. */
        bool                mouseDragging;  /* One or more mouse buttons are down. */
        Window*             window;
    };

    //------------------------------------------------------------------------

    class Listener
    {
    public:
                            Listener            (void)          {}
        virtual             ~Listener           (void)          {}

        virtual bool        handleEvent         (const Event& ev) = 0;
    };

    //------------------------------------------------------------------------

public:
                            Window              (void);
                            ~Window             (void);

    void                    setTitle            (const String& title);

    void                    setSize             (const Vec2i& size);
    Vec2i                   getSize             (void) const;

    void                    setVisible          (bool visible);
    bool                    isVisible           (void) const    { return m_isVisible; }
    void                    setFullScreen       (bool isFull);
    bool                    isFullScreen        (void) const    { return m_isFullScreen; }
    void                    toggleFullScreen    (void)          { setFullScreen(!isFullScreen()); }
    void                    realize             (void);

    void                    setGLConfig         (const GLContext::Config& config);
    const GLContext::Config& getGLConfig        (void) const    { return m_glConfig; }
    GLContext*              getGL               (void); // also makes the context current

    void                    repaint             (void);
    void                    repaintNow          (void);
    void                    requestClose        (void);

    void                    addListener         (Listener* listener);
    void                    removeListener      (Listener* listener);
    void                    removeListeners     (void);

    bool                    isKeyDown           (const String& key) const { return m_keysDown.contains(key); }
    bool                    isMouseKnown        (void) const    { return m_mouseKnown; }
    bool                    isMouseDragging     (void) const    { return (m_mouseDragCount != 0); }
    const Vec2i&            getMousePos         (void) const    { return m_mousePos; }

    void                    showMessageDialog   (const String& title, const String& text);
    String                  showFileDialog      (const String& title, bool save, const String& filters = "", const String& initialDir = "", bool forceInitialDir = false); // filters = "ext:Title,foo;bar:Title"
    String                  showFileLoadDialog  (const String& title, const String& filters = "", const String& initialDir = "", bool forceInitialDir = false) { return showFileDialog(title, false, filters, initialDir, forceInitialDir); }
    String                  showFileSaveDialog  (const String& title, const String& filters = "", const String& initialDir = "", bool forceInitialDir = false) { return showFileDialog(title, true, filters, initialDir, forceInitialDir); }

    void                    showModalMessage    (const String& msg);

    static void             staticInit          (void);
    static void             staticDeinit        (void);
#ifdef FW_QT
    bool                    event(bool activate = false);
    bool                    event(QEvent* ev);
#else
    static HWND             createHWND          (void);
    static UPTR             setWindowLong       (HWND hwnd, int idx, UPTR value);
#endif
    static int              getNumOpen          (void)          { return (s_inited) ? s_open->getSize() : 0; }
    static void             realizeAll          (void);
    static void             pollMessages        (void);

private:
    Event                   createSimpleEvent   (EventType type) { return createGenericEvent(type, "", 0, m_mouseKnown, m_mousePos); }
    Event                   createKeyEvent      (bool down, const String& key) { return createGenericEvent((down) ? EventType_KeyDown : EventType_KeyUp, key, 0, m_mouseKnown, m_mousePos); }
    Event                   createCharEvent     (S32 chr)       { return createGenericEvent(EventType_Char, "", chr, m_mouseKnown, m_mousePos); }
    Event                   createMouseEvent    (bool mouseKnown, const Vec2i& mousePos) { return createGenericEvent(EventType_Mouse, "", 0, mouseKnown, mousePos); }
    Event                   createGenericEvent  (EventType type, const String& key, S32 chr, bool mouseKnown, const Vec2i& mousePos);
    void                    postEvent           (const Event& ev);

    void                    create              (void);
    void                    destroy             (void);
    void                    recreate            (void);

#ifndef FW_QT
    static LRESULT CALLBACK staticWindowProc    (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT                 windowProc          (UINT uMsg, WPARAM wParam, LPARAM lParam);
#endif

private:
                            Window              (const Window&); // forbidden
    Window&                 operator=           (const Window&); // forbidden

private:
    static bool             s_inited;
    static Array<Window*>*  s_open;
    static bool             s_ignoreRepaint;    // Prevents re-entering repaintNow() on Win32 or OpenGL failure.

#ifdef FW_QT
    QWindow*                m_window;
#else
    HWND                    m_hwnd;
    HDC                     m_hdc;
#endif

    GLContext::Config       m_glConfig;
    bool                    m_glConfigDirty;
    GLContext*              m_gl;

    bool                    m_isRealized;
    bool                    m_isVisible;
    Array<Listener*>        m_listeners;

    String                  m_title;
    bool                    m_isFullScreen;
    Vec2i                   m_pendingSize;
#ifndef FW_QT
    DWORD                   m_origStyle;
    RECT                    m_origRect;
#endif
    Set<String>             m_keysDown;
    bool                    m_pendingKeyFlush;

    bool                    m_mouseKnown;
    Vec2i                   m_mousePos;
    S32                     m_mouseDragCount;
    S32                     m_mouseWheelAcc;

    Vec2i                   m_prevSize;
};

//------------------------------------------------------------------------
}
