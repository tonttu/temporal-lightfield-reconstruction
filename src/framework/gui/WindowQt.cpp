#include "gui/Window.hpp"
#include "gpu/GLContext.hpp"
#include "base/Thread.hpp"

#include <QMessageBox>
#include <QFileDialog>
#include <QApplication>

#include <QWheelEvent>

#include <cassert>

//------------------------------------------------------------------------

namespace
{

const char* const    s_defaultTitle      = "Anonymous window";
const FW::Vec2i      s_defaultSize       = FW::Vec2i(1024, 768);
const bool           s_defaultFullScreen = false;

}

//------------------------------------------------------------------------

class QWindow : public QGLWidget
{
public:
    QWindow(FW::Window & window);

protected:
    virtual void paintGL();
    virtual bool event(QEvent* ev);
    virtual void initializeGL()
    {
        glewInit();
    }

    FW::Window & m_window;
};

//------------------------------------------------------------------------

namespace FW
{

bool            Window::s_inited        = false;
Array<Window*>* Window::s_open          = NULL;
bool            Window::s_ignoreRepaint = false;

//------------------------------------------------------------------------

Window::Window(void)
:   m_glConfigDirty     (false),
    m_gl                (NULL),

    m_isRealized        (false),
    m_isVisible         (true),

    m_title             (s_defaultTitle),
    m_isFullScreen      (false),
    m_pendingSize       (-1),

    m_pendingKeyFlush   (false),

    m_mouseKnown        (false),
    m_mousePos          (0),
    m_mouseDragCount    (0),
    m_mouseWheelAcc     (0),

    m_prevSize          (-1)
{
    create();
    setSize(s_defaultSize);
    setFullScreen(s_defaultFullScreen);

    FW_ASSERT(s_open);
    s_open->add(this);
}

//------------------------------------------------------------------------

Window::~Window(void)
{
    destroy();

    FW_ASSERT(s_open);
    s_open->removeItem(this);
}

//------------------------------------------------------------------------

void Window::setTitle(const String& title)
{
    if (m_title != title)
    {
        m_title = title;
        m_window->setWindowTitle(title.getPtr());
    }
}

//------------------------------------------------------------------------

void Window::setSize(const Vec2i& size)
{
    FW_ASSERT(size.x >= 0 && size.y >= 0);

    if (m_isFullScreen)
    {
        m_pendingSize = size;
        return;
    }

    m_window->resize(size.x, size.y);
}

//------------------------------------------------------------------------

Vec2i Window::getSize(void) const
{
    return Vec2i(m_window->width(), m_window->height());
}

//------------------------------------------------------------------------

void Window::setVisible(bool visible)
{
    if (m_isRealized && m_isVisible != visible)
        m_window->setVisible(visible);
    m_isVisible = visible;
}

//------------------------------------------------------------------------

void Window::setFullScreen(bool isFull)
{
    if (m_isFullScreen == isFull)
        return;

    m_isFullScreen = isFull;
    if (isFull)
    {
        m_window->showFullScreen();
    }
    else
    {
        m_window->showNormal();
        if (m_pendingSize.x != -1)
        {
            setSize(m_pendingSize);
            m_pendingSize = -1;
        }
    }
}

//------------------------------------------------------------------------

void Window::realize(void)
{
    if (!m_isRealized)
    {
        m_window->setVisible(m_isVisible);
        m_isRealized = true;
    }
}

//------------------------------------------------------------------------

void Window::setGLConfig(const GLContext::Config& config)
{
    m_glConfig = config;
    m_glConfigDirty = (m_gl && memcmp(&m_glConfig, &m_gl->getConfig(), sizeof(GLContext::Config)) != 0);
}

//------------------------------------------------------------------------

GLContext* Window::getGL(void)
{
    if (!m_gl)
    {
        m_gl = new GLContext(const_cast<QGLContext*>(m_window->context()));
        m_gl->setView(0, getSize());
    }
    m_gl->makeCurrent();
    return m_gl;
}

//------------------------------------------------------------------------

void Window::repaint(void)
{
    m_window->update();
}

//------------------------------------------------------------------------

void Window::repaintNow(void)
{
    if (s_ignoreRepaint)
        return;
    s_ignoreRepaint = true;

    if (m_glConfigDirty)
    {
        m_glConfigDirty = false;
        recreate();
    }

    Vec2i size = getSize();
    if (size.x > 0 && size.y > 0)
    {
        getGL()->setView(0, size);
        if (!getDiscardEvents())
        {
            s_ignoreRepaint = false;
            if (m_prevSize != size)
            {
                m_prevSize = size;
                postEvent(createSimpleEvent(EventType_Resize));
            }
            postEvent(createSimpleEvent(EventType_PrePaint));
            postEvent(createSimpleEvent(EventType_Paint));
            postEvent(createSimpleEvent(EventType_PostPaint));
            s_ignoreRepaint = true;
        }
        getGL()->swapBuffers();
    }

    Thread::yield();
    s_ignoreRepaint = false;
}

//------------------------------------------------------------------------

void Window::requestClose(void)
{
    m_window->close();
}

//------------------------------------------------------------------------

void Window::addListener(Listener* listener)
{
    if (!listener || m_listeners.contains(listener))
        return;

    m_listeners.add(listener);
    listener->handleEvent(createSimpleEvent(EventType_AddListener));
}

//------------------------------------------------------------------------

void Window::removeListener(Listener* listener)
{
    if (!m_listeners.contains(listener))
        return;

    m_listeners.removeItem(listener);
    listener->handleEvent(createSimpleEvent(EventType_RemoveListener));
}

//------------------------------------------------------------------------

void Window::removeListeners(void)
{
    while (m_listeners.getSize())
        removeListener(m_listeners.getLast());
}

//------------------------------------------------------------------------

void Window::showMessageDialog(const String& title, const String& text)
{
    bool old = setDiscardEvents(true);
    QMessageBox::information(m_window, title.getPtr(), text.getPtr());
    setDiscardEvents(old);
}

//------------------------------------------------------------------------

String Window::showFileDialog(const String& title, bool save, const String& filters, const String& initialDir, bool /*forceInitialDir*/)
{
    QStringList filterStrs;
    QStringList allExtensions;
    foreach (const QString& filter, QString(filters.getPtr()).split(",", QString::SkipEmptyParts)) {
        QStringList parts = filter.split(":", QString::SkipEmptyParts);
        QStringList extensions = parts[0].split(";", QString::SkipEmptyParts);
        allExtensions += extensions;
        assert(parts.size() == 2);
        filterStrs << QString("%1 Files (*.%2)").arg(parts[1], extensions.join(" *."));
    }

    // Add "All Supported Formats" and "All Files".

    if (!filterStrs.isEmpty() && !save)
    {
        filterStrs.insert(0, QString("All Supported Formats (*.%1)").arg(allExtensions.join(" *.")));
    }
    filterStrs << QString("All Files (*)");
    QString filter = filterStrs.join(";;");

    bool old = setDiscardEvents(true);
    QString out;
    QDir cwd = QDir::current();
    if (initialDir.getLength())
        QDir::setCurrent(initialDir.getPtr());
    if (save) {
        out = QFileDialog::getSaveFileName(m_window, title.getPtr(), QString(), filter);
    } else {
        out = QFileDialog::getOpenFileName(m_window, title.getPtr(), QString(), filter);
    }
    if (initialDir.getLength())
        QDir::setCurrent(cwd.absolutePath());
    setDiscardEvents(old);
    getGL()->swapBuffers();

    return out.toUtf8().data();
}

//------------------------------------------------------------------------

void Window::showModalMessage(const String& msg)
{
    if (!m_isRealized || !m_isVisible)
        return;

    GLContext* gl = getGL();
    for (int i = 0; i < 3; i++)
    {
        gl->drawModalMessage(msg);
        gl->swapBuffers();
    }
}

//------------------------------------------------------------------------

void Window::staticInit(void)
{
    if (s_inited)
        return;
    s_inited = true;

    // Create list of open windows.

    FW_ASSERT(!s_open);
    s_open = new Array<Window*>;
}

//------------------------------------------------------------------------

void Window::staticDeinit(void)
{
    if (!s_inited)
        return;
    s_inited = false;

    FW_ASSERT(s_open);
    while (s_open->getSize())
        delete s_open->getLast();
    delete s_open;
    s_open = NULL;
}

//------------------------------------------------------------------------

void Window::realizeAll(void)
{
    if (s_inited)
        for (int i = 0; i < s_open->getSize(); i++)
            s_open->get(i)->realize();
}

//------------------------------------------------------------------------

void Window::pollMessages(void)
{
    bool old = setDiscardEvents(true);

    QApplication::processEvents();

    setDiscardEvents(old);
}

//------------------------------------------------------------------------

Window::Event Window::createGenericEvent(EventType type, const String& key, S32 chr, bool mouseKnown, const Vec2i& mousePos)
{
    Event ev;
    ev.type             = type;
    ev.key              = key;
    ev.keyUnicode       = keyToUnicode(key);
    ev.chr              = chr;
    ev.mouseKnown       = mouseKnown;
    ev.mousePos         = mousePos;
    ev.mouseDelta       = (mouseKnown && m_mouseKnown) ? mousePos - m_mousePos : 0;
    ev.mouseDragging    = isMouseDragging();
    ev.window           = this;
    return ev;
}

//------------------------------------------------------------------------

void Window::postEvent(const Event& ev)
{
    m_mouseKnown = ev.mouseKnown;
    m_mousePos = ev.mousePos;

    if (ev.type == EventType_KeyDown ||
        ev.type == EventType_KeyUp ||
        ev.type == EventType_Char ||
        ev.type == EventType_Mouse)
    {
        for (int i = m_listeners.getSize() - 1; i >= 0; i--)
            if (hasError() || m_listeners[i]->handleEvent(ev))
                break;
    }
    else
    {
        for (int i = 0; i < m_listeners.getSize(); i++)
            if (hasError() || m_listeners[i]->handleEvent(ev))
                break;
    }

    failIfError();
}

//------------------------------------------------------------------------

void Window::create(void)
{
    staticInit();
    FW_ASSERT(s_open);
    m_window = new QWindow(*this);
    m_window->setMouseTracking(true);
    m_window->show();
}

//------------------------------------------------------------------------

void Window::destroy(void)
{
    m_window->deleteLater();
    delete m_gl;

    m_gl = NULL;
}

//------------------------------------------------------------------------

void Window::recreate(void)
{
    // Backup properties.

    QByteArray geometry = m_window->saveGeometry();

    // Recreate.

    destroy();
    create();

    // Restore properties.

    m_window->setWindowTitle(m_title.getPtr());
    m_window->restoreGeometry(geometry);

    if (m_isRealized)
        m_window->setVisible(m_isVisible);
}

//------------------------------------------------------------------------

bool Window::event(bool activate)
{
    if (activate || getDiscardEvents())
    {
        if (m_mouseDragCount)
            m_window->releaseMouse();

        m_pendingKeyFlush   = true;
        m_mouseKnown        = false;
        m_mouseDragCount    = 0;
        m_mouseWheelAcc     = 0;
    }

    if (m_pendingKeyFlush && !getDiscardEvents())
    {
        m_pendingKeyFlush = false;
        for (int slot = m_keysDown.firstSlot(); slot != -1; slot = m_keysDown.nextSlot(slot))
            postEvent(createKeyEvent(false, m_keysDown.getSlot(slot)));
        m_keysDown.clear();
    }
    return getDiscardEvents();
}

//------------------------------------------------------------------------

bool Window::event(QEvent* ev)
{
    incNestingLevel(1);
    if (event(ev->type() == QEvent::FocusIn))
        return true;

    bool ok = true;

    QMouseEvent* me;

    switch (ev->type())
    {
    case QEvent::Close:
        postEvent(createSimpleEvent(EventType_Close));
        return 0;

    case QEvent::MouseMove:
        me = static_cast<QMouseEvent*>(ev);
        postEvent(createMouseEvent(true, Vec2i(me->x(), me->y())));
        return 0;

    case QEvent::Wheel:
        m_mouseWheelAcc += static_cast<QWheelEvent*>(ev)->delta();
        while (m_mouseWheelAcc >= 120)
        {
            postEvent(createKeyEvent(true, FW_KEY_WHEEL_UP));
            postEvent(createKeyEvent(false, FW_KEY_WHEEL_UP));
            m_mouseWheelAcc -= 120;
        }
        while (m_mouseWheelAcc <= -120)
        {
            postEvent(createKeyEvent(true, FW_KEY_WHEEL_DOWN));
            postEvent(createKeyEvent(false, FW_KEY_WHEEL_DOWN));
            m_mouseWheelAcc += 120;
        }
        break;

    case QEvent::Leave:
        if (!m_mouseDragCount)
            postEvent(createMouseEvent(false, 0));
        break;

    case QEvent::KeyPress:
    case QEvent::KeyRelease:
    {
        QKeyEvent* ke = static_cast<QKeyEvent*>(ev);
        String key = qtkeyToKey(ke->key(),
                                ke->text().size() == 1 ? ke->text()[0] : QChar());
        bool down = ev->type() == QEvent::KeyPress;
        if (key.getLength()) {
            postEvent(createKeyEvent(down, key));
            if (down && !m_keysDown.contains(key))
                m_keysDown.add(key);
            else if(!down && m_keysDown.contains(key))
                m_keysDown.remove(key);
        }

        // Post character events.

        for (int i = 0; down && i < ke->text().size(); ++i)
            postEvent(createCharEvent(ke->text()[i].unicode()));
        break;
    }

    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    {
        QMouseEvent* me = static_cast<QMouseEvent*>(ev);

        // Identify the key.

        String key;
        bool down = ev->type() == QEvent::MouseButtonPress;
        if (me->button() == Qt::LeftButton)
            key = FW_KEY_MOUSE_LEFT;
        else if(me->button() == Qt::MidButton)
            key = FW_KEY_MOUSE_MIDDLE;
        else if(me->button() == Qt::RightButton)
            key = FW_KEY_MOUSE_RIGHT;
        else {
            ok = false;
            break;
        }

        // Update drag status.

        if (down && !m_keysDown.contains(key))
        {
            m_keysDown.add(key);
            /// @todo grabbing mouse is really annoying in some situations
            // if (!m_mouseDragCount)
            //    m_window->grabMouse();
            m_mouseDragCount++;
        }
        else if (!down && m_keysDown.contains(key))
        {
            m_keysDown.remove(key);
            m_mouseDragCount--;
            if (!m_mouseDragCount)
                m_window->releaseMouse();
        }

        // Post event.

        postEvent(createKeyEvent(down, key));
        break;
    }

    default:
        ok = false;
    }

    incNestingLevel(-1);
    return ok;
}


} // namespace FW

//------------------------------------------------------------------------

QWindow::QWindow(FW::Window & window)
    : QGLWidget(QGLFormat(QGL::DoubleBuffer | QGL::DepthBuffer | QGL::Rgba |
                          QGL::AlphaChannel | QGL::DirectRendering | QGL::SampleBuffers)),
    m_window(window)
{
}

//------------------------------------------------------------------------

void QWindow::paintGL()
{
    FW::incNestingLevel(1);
    m_window.event();
    m_window.repaintNow();
    FW::incNestingLevel(-1);
}

//------------------------------------------------------------------------

bool QWindow::event(QEvent* ev)
{
    return m_window.event(ev) || QGLWidget::event(ev);
}
