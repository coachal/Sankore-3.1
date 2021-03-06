#ifndef UB_ABSTRACTDRAWRULER_H_
#define UB_ABSTRACTDRAWRULER_H_

#include <QtGui>
class UBGraphicsScene;
class QGraphicsSvgItem;

class UBAbstractDrawRuler : public QObject
{
    Q_OBJECT

public:
    UBAbstractDrawRuler();
    ~UBAbstractDrawRuler();

    void create(QGraphicsItem& item);

    virtual void StartLine(const QPointF& position, qreal width);
    virtual void DrawLine(const QPointF& position, qreal width);
    virtual void EndLine();

    signals:
        void hidden();

protected:

    void paint();

    virtual UBGraphicsScene* scene() const = 0;

    virtual void rotateAroundCenter(qreal angle) = 0;

    virtual QPointF rotationCenter() const = 0;
    virtual QRectF  closeButtonRect() const = 0;

    bool mShowButtons;
    QGraphicsSvgItem* mCloseSvgItem;
    qreal mAntiScaleRatio;

    QPointF startDrawPosition;

    QCursor moveCursor() const;
    QCursor rotateCursor() const;
    QCursor closeCursor() const;
    QCursor drawRulerLineCursor() const;

    QColor  drawColor() const;
    QColor  middleFillColor() const;
    QColor  edgeFillColor() const;
    QFont   font() const;

    static const QColor sLightBackgroundEdgeFillColor;
    static const QColor sLightBackgroundMiddleFillColor;
    static const QColor sLightBackgroundDrawColor;
    static const QColor sDarkBackgroundEdgeFillColor;
    static const QColor sDarkBackgroundMiddleFillColor;
    static const QColor sDarkBackgroundDrawColor;

    static const int    sLeftEdgeMargin;
    static const int    sDegreeToQtAngleUnit;
    static const int    sRotationRadius;
    static const int    sPixelsPerMillimeter;
    static const int    sFillTransparency;
    static const int    sDrawTransparency;
    static const int    sRoundingRadius;

};

#endif
