/*
 * UBAbstractWidget.cpp
 *
 *  Created on: 11 févr. 2009
 *      Author: Luc
 */

#include "UBAbstractWidget.h"

#include <QtNetwork>
#include <QtXml>

#include "frameworks/UBFileSystemUtils.h"

#include "core/UBApplicationController.h"
#include "core/UBApplication.h"
#include "core/UBSettings.h"

#include "network/UBNetworkAccessManager.h"

#include "api/UBWidgetUniboardAPI.h"

#include "web/UBWebPage.h"
#include "web/UBWebKitUtils.h"
#include "web/UBWebController.h"

#include "core/memcheck.h"

QStringList UBAbstractWidget::sInlineJavaScripts;
bool UBAbstractWidget::sInlineJavaScriptLoaded = false;

UBAbstractWidget::UBAbstractWidget(const QUrl& pWidgetUrl, QWidget *parent)
    : UBRoutedMouseEventWebView(parent)
    , mWidgetUrl(pWidgetUrl)
    , mIsResizable(false)
    , mInitialLoadDone(false)
    , mLoadIsErronous(false)
    , mIsFreezable(true)
    , mCanBeContent(true)
    , mCanBeTool(true)
    , mIsFrozen(false)
    , mIsTakingSnapshot(false)
{
    setPage(new UBWebPage(this));
    QWebView::settings()->setAttribute(QWebSettings::PluginsEnabled, true);
    QWebView::settings()->setAttribute(QWebSettings::LocalStorageDatabaseEnabled, true);

    QWebView::page()->setNetworkAccessManager(UBNetworkAccessManager::defaultAccessManager());

    setAutoFillBackground(false);

    QPalette pagePalette = QWebView::page()->palette();
    pagePalette.setBrush(QPalette::Base, QBrush(Qt::transparent));
    pagePalette.setBrush(QPalette::Window, QBrush(Qt::transparent));
    QWebView::page()->setPalette(pagePalette);

    QPalette viewPalette = palette();
    pagePalette.setBrush(QPalette::Base, QBrush(Qt::transparent));
    viewPalette.setBrush(QPalette::Window, QBrush(Qt::transparent));
    setPalette(viewPalette);

    connect(QWebView::page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()), this, SLOT(javaScriptWindowObjectCleared()));
    connect(QWebView::page(), SIGNAL(geometryChangeRequested(const QRect&)), this, SIGNAL(geometryChangeRequested(const QRect&)));
    connect(QWebView::page(), SIGNAL(loadFinished(bool)), this, SLOT(mainFrameLoadFinished (bool)));

    setMouseTracking(true);
}


UBAbstractWidget::~UBAbstractWidget()
{
    // NOOP
}


bool UBAbstractWidget::event(QEvent *event)
{
    if (event->type() == QEvent::ContextMenu)
    {
        event->accept();
        return true;
    } else {
        return QWebView::event(event);
    }
}


void UBAbstractWidget::mainFrameLoadFinished (bool ok)
{
    mInitialLoadDone = true;
    mLoadIsErronous = !ok;

    update();
}


bool UBAbstractWidget::hasEmbededObjects()
{
    if (QWebView::page()->mainFrame())
    {
        QList<UBWebKitUtils::HtmlObject> htmlObjects = UBWebKitUtils::objectsInFrame(QWebView::page()->mainFrame());
        return htmlObjects.length() > 0;
    }

    return false;
}


bool UBAbstractWidget::hasEmbededFlash()
{
    if (hasEmbededObjects())
    {
        return QWebView::page()->mainFrame()->toHtml().contains("application/x-shockwave-flash");
    }
    else
    {
        return false;
    }
}


void UBAbstractWidget::resize(qreal width, qreal height)
{
    QWebView::page()->setViewportSize(QSize(width, height));
    QWebView::setFixedSize(QSize(width, height));
}


QString UBAbstractWidget::iconFilePath(const QUrl& pUrl)
{
    // TODO UB 4.x read config.xml widget.icon param first

    QStringList files;

    files << "icon.svg";  // W3C widget default 1
    files << "icon.ico";  // W3C widget default 2
    files << "icon.png";  // W3C widget default 3
    files << "icon.gif";  // W3C widget default 4

    files << "Icon.png";  // Apple widget default

    QString file = UBFileSystemUtils::getFirstExistingFileFromList(pUrl.toLocalFile(), files);

    // default
    if (file.length() == 0)
    {
        file = QString(":/images/defaultWidgetIcon.png");
    }

    return file;
}



QString UBAbstractWidget::widgetName(const QUrl& widgetPath)
{
    QString name;
    QString version;

    QFile w3CConfigFile(widgetPath.toLocalFile() + "/config.xml");
    QFile appleConfigFile(widgetPath.toLocalFile() + "/Info.plist");

    if (w3CConfigFile.exists() && w3CConfigFile.open(QFile::ReadOnly))
    {
        QDomDocument doc;
        doc.setContent(w3CConfigFile.readAll());
        QDomElement root = doc.firstChildElement("widget");
        if (!root.isNull())
        {
            QDomElement nameElement = root.firstChildElement("name");
            if (!nameElement.isNull())
                name = nameElement.text();

            version = root.attribute("version", "");
        }

        w3CConfigFile.close();
    }
    else if (appleConfigFile.exists() && appleConfigFile.open(QFile::ReadOnly))
    {
        QDomDocument doc;
        doc.setContent(appleConfigFile.readAll());
        QDomElement root = doc.firstChildElement("plist");
        if (!root.isNull())
        {
            QDomElement dictElement = root.firstChildElement("dict");
            if (!dictElement.isNull())
            {
                QDomNodeList childNodes  = dictElement.childNodes();

                // looking for something like
                //  ..
                //  <key>CFBundleDisplayName</key>
                //  <string>brain scans</string>
                //  ..

                for(int i = 0; i < childNodes.count() - 1; i++)
                {
                    if (childNodes.at(i).isElement())
                    {
                        QDomElement elKey = childNodes.at(i).toElement();
                        if (elKey.text() == "CFBundleDisplayName")
                        {
                            if (childNodes.at(i + 1).isElement())
                            {
                               QDomElement elValue = childNodes.at(i + 1).toElement();
                               name = elValue.text();
                            }
                        }
                        else if (elKey.text() == "CFBundleShortVersionString")
                        {
                            if (childNodes.at(i + 1).isElement())
                            {
                               QDomElement elValue = childNodes.at(i + 1).toElement();
                               version = elValue.text();
                            }
                        }
                    }
                }
            }
        }

        appleConfigFile.close();
    }

    QString result;

    if (name.length() > 0)
    {
        result = name;
        if (version.length() > 0)
        {
            result += " ";
            result += version;
        }
    }

    return result;
}


int UBAbstractWidget::widgetType(const QUrl& pUrl)
{
    QString mime = UBFileSystemUtils::mimeTypeFromFileName(pUrl.toString());

    if (mime == "application/vnd.apple-widget")
    {
        return UBWidgetType::Apple;
    }
    else if (mime == "application/widget")
    {
        return UBWidgetType::W3C;
    }
    else
    {
        return UBWidgetType::Other;
    }
}


void UBAbstractWidget::mousePressEvent(QMouseEvent *event)
{
    if(mIsFrozen)
    {
        event->accept();
        return;
    }

    UBRoutedMouseEventWebView::mousePressEvent(event);
    mMouseIsPressed = true;
}


void UBAbstractWidget::mouseMoveEvent(QMouseEvent *event)
{

    if(mIsFrozen)
    {
        event->accept();
        return;
    }

    // TODO UB 4.x fix web kit mouse move routing

    if (mFirstReleaseAfterMove)
    {
        mFirstReleaseAfterMove = false;
    }
    else
    {
        UBRoutedMouseEventWebView::mouseMoveEvent(event);
    }
}


void UBAbstractWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if(mIsFrozen)
    {
        event->accept();
        return;
    }

    UBRoutedMouseEventWebView::mouseReleaseEvent(event);
    mMouseIsPressed = false;
    mFirstReleaseAfterMove = true;
}


QWebView * UBAbstractWidget::createWindow(QWebPage::WebWindowType type)
{
    if (type == QWebPage::WebBrowserWindow)
    {
        UBApplication::applicationController->showInternet();
        return UBApplication::webController->createNewTab();
    }
    else
    {
        return this;
    }
}


void UBAbstractWidget::injectInlineJavaScript()
{
    if (!sInlineJavaScriptLoaded)
    {
        sInlineJavaScripts = UBApplication::applicationController->widgetInlineJavaScripts();
        sInlineJavaScriptLoaded = true;
    }

    foreach(QString script, sInlineJavaScripts)
    {
        QWebView::page()->mainFrame()->evaluateJavaScript(script);
    }
}


void UBAbstractWidget::javaScriptWindowObjectCleared()
{
    injectInlineJavaScript();
}


void UBAbstractWidget::paintEvent(QPaintEvent * event)
{
    if (mIsFrozen)
    {
        QPainter p(this);
        p.drawPixmap(0, 0, mSnapshot);
    }
    else if(mIsTakingSnapshot || (mInitialLoadDone && !mLoadIsErronous))
    {
        QWebView::paintEvent(event);
    }
    else
    {
         QPainter p(this);
         QString message;

         if (mLoadIsErronous)
             message = tr("Cannot load content");
         else
             message = tr("Loading ...");

         p.setFont(QFont("Arial", 12));

         QFontMetrics fm = p.fontMetrics();
         QRect txtBoundingRect = fm.boundingRect(message);

         txtBoundingRect.moveCenter(rect().center());
         txtBoundingRect.adjust(-10, -5, 10, 5);

         p.setPen(Qt::NoPen);
         p.setBrush(UBSettings::paletteColor);
         p.drawRoundedRect(txtBoundingRect, 3, 3);

         p.setPen(Qt::white);
         p.drawText(rect(), Qt::AlignCenter, message);
    }
}


QPixmap UBAbstractWidget::takeSnapshot()
{
    mIsTakingSnapshot = true;

    QPixmap pix(size());
    pix.fill(Qt::transparent);

    render(&pix);

    mIsTakingSnapshot = false;

    return pix;
}


void UBAbstractWidget::setSnapshot(const QPixmap& pix)
{
    mSnapshot = pix;
}


void UBAbstractWidget::freeze()
{
    QPixmap pix = takeSnapshot();
    mIsFrozen = true;
    setSnapshot(pix);
    update();
}


void UBAbstractWidget::unFreeze()
{
    mIsFrozen = false;
    update();
}

