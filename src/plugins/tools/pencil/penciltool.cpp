/***************************************************************************
 *   Project TUPITUBE DESK                                                *
 *   Project Contact: info@maefloresta.com                                 *
 *   Project Website: http://www.maefloresta.com                           *
 *   Project Leader: Gustav Gonzalez <info@maefloresta.com>                *
 *                                                                         *
 *   Developers:                                                           *
 *   2010:                                                                 *
 *    Gustavo Gonzalez                                                     *
 *                                                                         *
 *   KTooN's versions:                                                     * 
 *                                                                         *
 *   2006:                                                                 *
 *    David Cuadrado                                                       *
 *    Jorge Cuadrado                                                       *
 *   2003:                                                                 *
 *    Fernado Roldan                                                       *
 *    Simena Dinas                                                         *
 *                                                                         *
 *   Copyright (C) 2010 Gustav Gonzalez - http://www.maefloresta.com       *
 *   License:                                                              *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#include "penciltool.h"
#include "tuppaintareaevent.h"

#include <QGraphicsView>
#include <cmath>

PencilTool::PencilTool(): TupToolPlugin()
{
    settings = nullptr;
    item = nullptr;

    setupActions();
}

PencilTool::~PencilTool()
{
}

void PencilTool::setupActions()
{
    penCursor = QCursor(kAppProp->themeDir() + "cursors/pencil.png", 4, 4);
    TAction *pencil = new TAction(QPixmap(kAppProp->themeDir() + "icons/pencil.png"), tr("Pencil"), this);
    pencil->setShortcut(QKeySequence(tr("P")));
    pencil->setToolTip(tr("Pencil") + " - " + "P");
    pencil->setCursor(penCursor);

    penActions.insert(tr("Pencil"), pencil);
}

void PencilTool::init(TupGraphicsScene *gScene)
{
    /*
    #ifdef TUP_DEBUG
        qDebug() << "PencilTool::init()";
    #endif
    */

    scene = gScene;
    brushManager = scene->getBrushManager();
    input = scene->inputDeviceInformation();
    resize = false;
    circleZValue = ZLAYER_BASE + (gScene->currentScene()->layersCount() * ZLAYER_LIMIT);

    TCONFIG->beginGroup("BrushParameters");
    penWidth = TCONFIG->value("Thickness", 3).toInt();

    foreach (QGraphicsView *view, gScene->views())
        view->setDragMode(QGraphicsView::NoDrag);
}

QStringList PencilTool::keys() const
{
    return QStringList() << tr("Pencil");
}

void PencilTool::press(const TupInputDeviceInformation *input, TupBrushManager *brushManager, TupGraphicsScene *gScene)
{
    if (!resize) {
        firstPoint = input->pos();

        path = QPainterPath();
        path.moveTo(firstPoint);

        previousPos = input->pos();

        item = new TupPathItem();
        if (brushManager->pen().color().alpha() == 0) {
            QPen pen;
            pen.setWidth(1);
            pen.setBrush(QBrush(Qt::black));
            item->setPen(pen);
        } else {
            item->setPen(brushManager->pen());
        }
        gScene->includeObject(item);
    }
}

void PencilTool::move(const TupInputDeviceInformation *input, TupBrushManager *brushManager, TupGraphicsScene *gScene)
{
    Q_UNUSED(brushManager)
    Q_UNUSED(gScene)

    if (resize) {
        QPointF point = input->pos();
        QPointF result = penCirclePos - point;
        penWidth = static_cast<int>(sqrt(pow(result.x(), 2) + pow(result.y(), 2)));

        QPointF topLeft(penCirclePos.x() - (penWidth/2), penCirclePos.y() - (penWidth/2));
        QSize size(penWidth, penWidth);
        QRectF rect(topLeft, size);
        penCircle->setRect(rect);
    } else {
        if (!item)
            return;

        QPointF lastPoint = input->pos();

        path.moveTo(previousPos);
        path.lineTo(lastPoint);

        item->setPath(path);
        previousPos = lastPoint;
    }
}

void PencilTool::release(const TupInputDeviceInformation *input, TupBrushManager *brushManager, TupGraphicsScene *gScene)
{
    Q_UNUSED(brushManager)

    if (!resize) {
        if (!item)
            return;

        // Drawing a point
        if (firstPoint == input->pos() && path.elementCount() == 1) {
            QPointF currentPoint = input->pos();
            gScene->removeItem(item);
 
            qreal radius = brushManager->pen().width();
            QPointF distance((radius + 2)/2, (radius + 2)/2);
            QPen inkPen(brushManager->penColor(), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
            TupEllipseItem *blackEllipse = new TupEllipseItem(QRectF(currentPoint - distance,
                                                              QSize(static_cast<int>(radius + 2), static_cast<int>(radius + 2))));
            blackEllipse->setPen(inkPen);
            blackEllipse->setBrush(inkPen.brush());
            gScene->includeObject(blackEllipse);

            QPointF point = currentPoint - distance;

            QDomDocument doc;
            doc.appendChild(blackEllipse->toXml(doc));
            TupProjectRequest request = TupRequestBuilder::createItemRequest(gScene->currentSceneIndex(), gScene->currentLayerIndex(), gScene->currentFrameIndex(),
                                                                             0, point, gScene->getSpaceContext(), TupLibraryObject::Item, TupProjectRequest::Add,
                                                                             doc.toString());
            emit requested(&request);
            return;
        } else {
            if (smoothness > 0)
                smoothPath(path, smoothness);
        }

        item->setPen(brushManager->pen());
        item->setBrush(brushManager->brush());
        item->setPath(path);

        QDomDocument doc;
        doc.appendChild(item->toXml(doc));

        TupProjectRequest request = TupRequestBuilder::createItemRequest(gScene->currentSceneIndex(), gScene->currentLayerIndex(), gScene->currentFrameIndex(),
                                                                         0, QPoint(), gScene->getSpaceContext(), TupLibraryObject::Item, TupProjectRequest::Add,
                                                                         doc.toString());
        emit requested(&request);
    }
}

void PencilTool::smoothPath(QPainterPath &path, double smoothness, int from, int to)
{
    QPolygonF polygon;
    QList<QPolygonF> polygons = path.toSubpathPolygons();
    QList<QPolygonF>::iterator it = polygons.begin();
    QPolygonF::iterator pointIt;

    while (it != polygons.end()) {
        pointIt = (*it).begin();

        while (pointIt <= (*it).end()-2) {
            polygon << (*pointIt);
            pointIt += 2;
        }
        ++it;
    }

    if (smoothness > 0) {
        path = TupGraphicalAlgorithm::bezierFit(polygon, static_cast<float>(smoothness), from, to);
    } else {
        path = QPainterPath();
        path.addPolygon(polygon);
    }
}

QMap<QString, TAction *> PencilTool::actions() const
{
    return penActions;
}

int PencilTool::toolType() const
{
    return TupToolInterface::Brush;
}

QWidget *PencilTool::configurator()
{
    if (!settings) {
        settings = new PenSettings;
        connect(settings, SIGNAL(smoothnessUpdated(double)), this, SLOT(updateSmoothness(double)));

        TCONFIG->beginGroup("PencilTool");
        smoothness = TCONFIG->value("Smoothness", 4.0).toDouble();
        if (smoothness == 0.0)
            smoothness = 4.0;
        settings->updateSmoothness(smoothness);
    }

    return settings;
}

void PencilTool::updateSmoothness(double value)
{
    smoothness = value;

    TCONFIG->beginGroup("PencilTool");
    TCONFIG->setValue("Smoothness", QString::number(smoothness, 'f', 2));
}

void PencilTool::aboutToChangeTool() 
{
}

void PencilTool::saveConfig()
{
    if (settings) {
        settings = new PenSettings;
        connect(settings, SIGNAL(smoothnessUpdated(double)), this, SLOT(updateSmoothness(double)));

        TCONFIG->beginGroup("PencilTool");
        TCONFIG->setValue("Smoothness", QString::number(smoothness, 'f', 2));
        settings->updateSmoothness(smoothness);
    }
}

void PencilTool::keyPressEvent(QKeyEvent *event)
{
    #ifdef TUP_DEBUG
        qDebug() << "PencilTool::keyPressEvent()";
    #endif

    if (event->modifiers() == Qt::ShiftModifier) {
        resize = true;
        input = scene->inputDeviceInformation();
        int diameter = brushManager->penWidth();
        int radius = diameter/2;
        penCirclePos = input->pos();

        penCircle = new QGraphicsEllipseItem(penCirclePos.x() - radius, penCirclePos.y() - radius, diameter, diameter);
        penCircle->setZValue(circleZValue);
        scene->addItem(penCircle);
        return;
    }

    if (event->key() == Qt::Key_F11 || event->key() == Qt::Key_Escape) {
        emit closeHugeCanvas();
        return;
    }

    QPair<int, int> flags = TupToolPlugin::setKeyAction(event->key(), event->modifiers());
    if (flags.first != -1 && flags.second != -1)
        emit callForPlugin(flags.first, flags.second);
}

void PencilTool::keyReleaseEvent(QKeyEvent *event) 
{
    #ifdef TUP_DEBUG
        qDebug() << "PencilTool::keyReleaseEvent()";
    #endif

    Q_UNUSED(event)

    if (resize) {
        resize = false;
        scene->removeItem(penCircle);

        TCONFIG->beginGroup("BrushParameters");
        TCONFIG->setValue("Thickness", penWidth);

        emit penWidthChanged(penWidth);
    }
}

QCursor PencilTool::polyCursor() const
{
    return penCursor;
}

void PencilTool::sceneResponse(const TupSceneResponse *event)
{
    Q_UNUSED(event)
}
