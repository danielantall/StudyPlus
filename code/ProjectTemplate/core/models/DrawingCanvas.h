#pragma once
#include <QWidget>
#include <QImage>
#include <QPoint>
#include <QPen>
#include <QColor>

/**
 * @brief A simple drawing canvas widget for task annotations
 * 
 * Allows users to draw freehand sketches, diagrams, or notes
 * within a task. Supports basic drawing operations and export.
 */
class DrawingCanvas : public QWidget {
    Q_OBJECT

public:
    explicit DrawingCanvas(QWidget* parent = nullptr);
    
    // Canvas operations
    void clearCanvas();
    void setPenColor(const QColor& color);
    void setPenWidth(int width);
    
    // Data operations
    QImage getImage() const;
    void setImage(const QImage& image);
    QString toBase64() const;
    void fromBase64(const QString& data);

signals:
    void drawingChanged();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    QImage image_;
    bool drawing_ = false;
    QPoint lastPoint_;
    QPen currentPen_;
    
    void resizeImage(QImage* image, const QSize& newSize);
};

