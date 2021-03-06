#ifndef UBDOCKPALETTE_H
#define UBDOCKPALETTE_H

#include <QWidget>
#include <QMouseEvent>
#include <QBrush>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QEvent>
#include <QTime>
#include <QPoint>
#include <QPixmap>

#define TABSIZE	    50
#define CLICKTIME   1000000

/**
 * \brief The dock positions
 */
typedef enum
{
    eUBDockOrientation_Left,  /** Left dock */
    eUBDockOrientation_Right, /** Right dock */
    eUBDockOrientation_Top,   /** [to be implemented]Top dock */
    eUBDockOrientation_Bottom /** [to be implemented]Bottom dock */
}eUBDockOrientation;

class UBDockPalette : public QWidget
{
public:
    UBDockPalette(QWidget* parent=0, const char* name="UBDockPalette");
    ~UBDockPalette();

    eUBDockOrientation orientation();
    void setOrientation(eUBDockOrientation orientation);

    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void paintEvent(QPaintEvent *event);
    virtual void enterEvent(QEvent *);
    virtual void leaveEvent(QEvent *);

    void setBackgroundBrush(const QBrush& brush);

protected:
    virtual int border();
    virtual int radius();
    virtual void updateMaxWidth();
    virtual void resizeEvent(QResizeEvent *event);
    virtual int collapseWidth();

    /** The current dock orientation */
    eUBDockOrientation mOrientation;
    /** The current baclground brush */
    QBrush mBackgroundBrush;
    /** The preferred width */
    int mPreferredWidth;
    /** The preferred height */
    int mPreferredHeight;
    /** A flag used to allow the resize */
    bool mCanResize;
    /** A flag indicating if the palette has been resized between a click and a release */
    bool mResized;
    /** The width that trig the collapse */
    int mCollapseWidth;
    /** The last width of the palette */
    int mLastWidth;
    /** The click time*/
    QTime mClickTime;
    /** The mouse pressed position */
    QPoint mMousePressPos;
    /** The palette icon */
    QPixmap mIcon;

private:
    void tabClicked();
};

#endif // UBDOCKPALETTE_H
