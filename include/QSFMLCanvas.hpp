#ifndef QSFMLCANVAS_HPP
#define QSFMLCANVAS_HPP

#include <SFML/Graphics.hpp>
#include <QWidget>
#include <QPaintEngine>
#include <QTimer>
#include <QResizeEvent>  // 添加这行

class QSFMLCanvas : public QWidget, public sf::RenderWindow
{
    Q_OBJECT

public:
    explicit QSFMLCanvas(QWidget* parent = nullptr, 
                        const QPoint& position = QPoint(0, 0), 
                        const QSize& size = QSize(400, 300), 
                        unsigned int frameTime = 0);
    
    virtual ~QSFMLCanvas();

protected:
    virtual void showEvent(QShowEvent*);
    virtual void paintEvent(QPaintEvent*);
    virtual QPaintEngine* paintEngine() const;
    virtual void resizeEvent(QResizeEvent* event);

    virtual void onInit() {}
    virtual void onUpdate() {}
    virtual void onResize() {}

private:
    bool m_initialized;
    QPoint m_position;
    QSize m_size;
    QTimer m_timer;

private slots:
    void onTimer();
};

#endif // QSFMLCANVAS_HPP