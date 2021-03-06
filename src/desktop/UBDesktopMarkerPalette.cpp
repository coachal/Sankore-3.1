#include "UBDesktopMarkerPalette.h"

#include "core/UBApplication.h"
#include "board/UBBoardController.h"
#include "board/UBDrawingController.h"
#include "gui/UBMainWindow.h"
#include "gui/UBToolbarButtonGroup.h"

#include "core/memcheck.h"

UBDesktopMarkerPalette::UBDesktopMarkerPalette(QWidget *parent)
    : UBPropertyPalette(Qt::Horizontal, parent)
{
    // Setup color choice widget
    QList<QAction *> colorActions;
    colorActions.append(UBApplication::mainWindow->actionColor0);
    colorActions.append(UBApplication::mainWindow->actionColor1);
    colorActions.append(UBApplication::mainWindow->actionColor2);
    colorActions.append(UBApplication::mainWindow->actionColor3);

    UBToolbarButtonGroup *colorChoice = new UBToolbarButtonGroup(UBApplication::mainWindow->boardToolBar, colorActions);
    colorChoice->displayText(false);

    //connect(colorChoice, SIGNAL(activated(int)), this, SLOT(UBApplication::boardController->setColorIndex(int)));
    connect(UBDrawingController::drawingController(), SIGNAL(colorIndexChanged(int)), colorChoice, SLOT(setCurrentIndex(int)));
    connect(UBDrawingController::drawingController(), SIGNAL(colorIndexChanged(int)), this, SLOT(close()));
    connect(UBDrawingController::drawingController(), SIGNAL(colorPaletteChanged()), colorChoice, SLOT(colorPaletteChanged()));
    connect(UBDrawingController::drawingController(), SIGNAL(colorPaletteChanged()), this, SLOT(close()));

    layout()->addWidget(colorChoice);

    // Setup line width choice widget
    QList<QAction *> lineWidthActions;
    lineWidthActions.append(UBApplication::mainWindow->actionLineSmall);
    lineWidthActions.append(UBApplication::mainWindow->actionLineMedium);
    lineWidthActions.append(UBApplication::mainWindow->actionLineLarge);

    UBToolbarButtonGroup *lineWidthChoice = new UBToolbarButtonGroup(UBApplication::mainWindow->boardToolBar, lineWidthActions);
    lineWidthChoice->displayText(false);

    connect(lineWidthChoice, SIGNAL(activated(int)), UBDrawingController::drawingController(), SLOT(setLineWidthIndex(int)));
    connect(lineWidthChoice, SIGNAL(activated(int)), this, SLOT(close()));
    connect(UBDrawingController::drawingController(), SIGNAL(lineWidthIndexChanged(int)), lineWidthChoice, SLOT(setCurrentIndex(int)));
    connect(UBDrawingController::drawingController(), SIGNAL(lineWidthIndexChanged(int)), this, SLOT(close()));

    layout()->addWidget(lineWidthChoice);
}
