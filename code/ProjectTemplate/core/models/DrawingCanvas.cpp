#include "DrawingCanvas.h"
#include <QPainter>
#include <QMouseEvent>
#include <QBuffer>

/**
 * @brief Constructs a DrawingCanvas widget
 * @param parent The parent widget
 */
DrawingCanvas::DrawingCanvas(QWidget* parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_StaticContents);
    setMinimumSize(400, 300);
    
    // Initialize with white background
    image_ = QImage(size(), QImage::Format_RGB32);
    image_.fill(Qt::white);
    
    // Set default pen
    currentPen_.setWidth(2);
    currentPen_.setColor(Qt::black);
    currentPen_.setCapStyle(Qt::RoundCap);
    currentPen_.setJoinStyle(Qt::RoundJoin);
}

/**
 * @brief Clears the canvas to white
 */
void DrawingCanvas::clearCanvas()
{
    image_.fill(Qt::white);
    update();
    emit drawingChanged();
}

/**
 * @brief Sets the pen color for drawing
 * @param color The new pen color
 */
void DrawingCanvas::setPenColor(const QColor& color)
{
    currentPen_.setColor(color);
}

/**
 * @brief Sets the pen width for drawing
 * @param width The new pen width in pixels
 */
void DrawingCanvas::setPenWidth(int width)
{
    currentPen_.setWidth(width);
}

/**
 * @brief Gets the current drawing as a QImage
 * @return The canvas image
 */
QImage DrawingCanvas::getImage() const
{
    return image_;
}

/**
 * @brief Sets the canvas image
 * @param image The image to display
 */
void DrawingCanvas::setImage(const QImage& image)
{
    image_ = image;
    update();
}

/**
 * @brief Exports the drawing as a base64-encoded PNG string
 * @return Base64 string representation of the drawing
 */
QString DrawingCanvas::toBase64() const
{
    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::WriteOnly);
    image_.save(&buffer, "PNG");
    return QString::fromLatin1(byteArray.toBase64());
}

/**
 * @brief Imports a drawing from a base64-encoded PNG string
 * @param data Base64 string representation of the drawing
 */
void DrawingCanvas::fromBase64(const QString& data)
{
    if (data.isEmpty()) {
        clearCanvas();
        return;
    }
    
    QByteArray byteArray = QByteArray::fromBase64(data.toLatin1());
    QImage loaded;
    if (loaded.loadFromData(byteArray, "PNG")) {
        image_ = loaded;
        update();
    }
}

/**
 * @brief Paints the canvas
 * @param event The paint event
 */
void DrawingCanvas::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    QRect dirtyRect = event->rect();
    painter.drawImage(dirtyRect, image_, dirtyRect);
}

/**
 * @brief Handles mouse press events to start drawing
 * @param event The mouse event
 */
void DrawingCanvas::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        lastPoint_ = event->pos();
        drawing_ = true;
    }
}

/**
 * @brief Handles mouse move events to draw lines
 * @param event The mouse event
 */
void DrawingCanvas::mouseMoveEvent(QMouseEvent* event)
{
    if ((event->buttons() & Qt::LeftButton) && drawing_) {
        QPainter painter(&image_);
        painter.setPen(currentPen_);
        painter.drawLine(lastPoint_, event->pos());
        
        int rad = (currentPen_.width() / 2) + 2;
        update(QRect(lastPoint_, event->pos())
                   .normalized()
                   .adjusted(-rad, -rad, +rad, +rad));
        
        lastPoint_ = event->pos();
    }
}

/**
 * @brief Handles mouse release events to stop drawing
 * @param event The mouse event
 */
void DrawingCanvas::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && drawing_) {
        drawing_ = false;
        emit drawingChanged();
    }
}

/**
 * @brief Handles resize events to maintain the canvas
 * @param event The resize event
 */
void DrawingCanvas::resizeEvent(QResizeEvent* event)
{
    if (width() > image_.width() || height() > image_.height()) {
        int newWidth = qMax(width() + 128, image_.width());
        int newHeight = qMax(height() + 128, image_.height());
        resizeImage(&image_, QSize(newWidth, newHeight));
        update();
    }
    QWidget::resizeEvent(event);
}

/**
 * @brief Resizes the image canvas
 * @param image Pointer to the image to resize
 * @param newSize The new size
 */
void DrawingCanvas::resizeImage(QImage* image, const QSize& newSize)
{
    if (image->size() == newSize) return;
    
    QImage newImage(newSize, QImage::Format_RGB32);
    newImage.fill(Qt::white);
    QPainter painter(&newImage);
    painter.drawImage(QPoint(0, 0), *image);
    *image = newImage;
}

