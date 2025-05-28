#include "QSFMLCanvas.hpp"
#include <QResizeEvent>  // 添加这行

QSFMLCanvas::QSFMLCanvas(QWidget* parent, const QPoint& position, const QSize& size, unsigned int frameTime) :
    QWidget(parent),
    m_initialized(false),
    m_position(position),
    m_size(size)
{
    setAttribute(Qt::WA_PaintOnScreen);
    setAttribute(Qt::WA_OpaquePaintEvent);
    setAttribute(Qt::WA_NoSystemBackground);
    setFocusPolicy(Qt::StrongFocus);

    if (frameTime > 0) {
        m_timer.setInterval(frameTime);
        connect(&m_timer, SIGNAL(timeout()), this, SLOT(onTimer()));
        m_timer.start();
    }
}

QSFMLCanvas::~QSFMLCanvas() = default;

void QSFMLCanvas::showEvent(QShowEvent*)
{
    if (!m_initialized) {
        sf::RenderWindow::create((sf::WindowHandle)winId());
        onInit();
        m_initialized = true;
    }
}

void QSFMLCanvas::paintEvent(QPaintEvent*)
{
    onUpdate();
    display();
}

QPaintEngine* QSFMLCanvas::paintEngine() const
{
    return nullptr;
}

void QSFMLCanvas::resizeEvent(QResizeEvent* event)
{
    sf::RenderWindow::setSize(sf::Vector2u(event->size().width(), event->size().height()));
    onResize();
    QWidget::resizeEvent(event);
}

void QSFMLCanvas::onTimer()
{
    repaint();
}