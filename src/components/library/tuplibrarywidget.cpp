/***************************************************************************
 *   Project TUPITUBE DESK                                                *
 *   Project Contact: info@maefloresta.com                                 *
 *   Project Website: http://www.maefloresta.com                           *
 *   Project Leader: Gustav Gonzalez <info@maefloresta.com>                *
 *                                                                         *
 *   Developers:                                                           *
 *   2010:                                                                 *
 *    Gustavo Gonzalez / xtingray                                          *
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

#include "tuplibrarywidget.h"
#include "tuplayer.h"

#define RETURN_IF_NOT_LIBRARY if (!library) return;

TupLibraryWidget::TupLibraryWidget(QWidget *parent) : TupModuleWidgetBase(parent)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupLibraryWidget()]";
    #endif

    childCount = 0;
    renaming = false;
    mkdir = false;
    isEffectSound = false;

    setWindowIcon(QPixmap(THEME_DIR + "icons/library.png"));
    setWindowTitle(tr("Library"));

    screen = QGuiApplication::screens().at(0);

    libraryDir = QDir(CONFIG_DIR + "libraries");

    display = new TupLibraryDisplay();
    connect(display, SIGNAL(frameUpdated(int)), this, SLOT(updateSoundTiming(int)));

    libraryTree = new TupItemManager(this);

    connect(libraryTree, SIGNAL(itemSelected(QTreeWidgetItem *)), this,
                                   SLOT(previewItem(QTreeWidgetItem *)));

    connect(libraryTree, SIGNAL(itemRemoved()), this,
                                   SLOT(removeCurrentItem()));

    connect(libraryTree, SIGNAL(itemCloned(QTreeWidgetItem*)), this,
                                   SLOT(cloneObject(QTreeWidgetItem*)));

    connect(libraryTree, SIGNAL(itemExported(QTreeWidgetItem*)), this,
                                   SLOT(exportObject(QTreeWidgetItem*)));

    connect(libraryTree, SIGNAL(itemRenamed(QTreeWidgetItem*)), this,
                                   SLOT(renameObject(QTreeWidgetItem*)));

    connect(libraryTree, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this,
                                   SLOT(refreshItem(QTreeWidgetItem*)));

    connect(libraryTree, SIGNAL(editorClosed()), this,
                                   SLOT(updateItemEditionState()));

    connect(libraryTree, SIGNAL(itemMoved(QString, QString)), this,
                                   SLOT(updateLibrary(QString, QString)));

    connect(libraryTree, SIGNAL(itemRequired()), this,
                                   SLOT(insertObjectInWorkspace()));

    connect(libraryTree, SIGNAL(itemCreated(QTreeWidgetItem*)), this,
                                   SLOT(activeRefresh(QTreeWidgetItem*)));

    connect(libraryTree, SIGNAL(inkscapeEditCall(QTreeWidgetItem*)), this,
                                   SLOT(openInkscapeToEdit(QTreeWidgetItem*)));

    connect(libraryTree, SIGNAL(gimpEditCall(QTreeWidgetItem*)), this,
                                   SLOT(openGimpToEdit(QTreeWidgetItem*)));

    connect(libraryTree, SIGNAL(kritaEditCall(QTreeWidgetItem*)), this,
                                   SLOT(openKritaToEdit(QTreeWidgetItem*)));

    connect(libraryTree, SIGNAL(myPaintEditCall(QTreeWidgetItem*)), this,
                                   SLOT(openMyPaintToEdit(QTreeWidgetItem*)));

    connect(libraryTree, SIGNAL(newRasterCall()), this,
                                   SLOT(createRasterObject()));

    connect(libraryTree, SIGNAL(newVectorCall()), this,
                                   SLOT(createVectorObject()));

    QGroupBox *buttons = new QGroupBox(this);
    QHBoxLayout *buttonLayout = new QHBoxLayout(buttons);
    buttonLayout->setMargin(0);
    buttonLayout->setSpacing(0);

    QHBoxLayout *comboLayout = new QHBoxLayout;
    comboLayout->setMargin(0);
    comboLayout->setSpacing(0);

    itemType = new QComboBox();
    itemType->setIconSize(QSize(15, 15));
    itemType->setMaximumWidth(120);
    
    itemType->addItem(QIcon(THEME_DIR + "icons/bitmap.png"), tr("Image"));
    itemType->addItem(QIcon(THEME_DIR + "icons/svg.png"), tr("Svg File"));
    itemType->addItem(QIcon(THEME_DIR + "icons/drawing_object.png"), tr("Native Object"));
    itemType->addItem(QIcon(THEME_DIR + "icons/bitmap_array.png"), tr("Image Sequence"));
    itemType->addItem(QIcon(THEME_DIR + "icons/svg_array.png"), tr("Svg Sequence"));
    itemType->addItem(QIcon(THEME_DIR + "icons/sound_object.png"), tr("Sound File"));

    comboLayout->addWidget(itemType);

    connect(itemType, SIGNAL(currentIndexChanged(int)), this, SLOT(importLibraryObject()));

    TImageButton *addGC = new TImageButton(QPixmap(THEME_DIR + "icons/plus_sign.png"), 22, buttons);
    addGC->setToolTip(tr("Add an object to library"));
    connect(addGC, SIGNAL(clicked()), this, SLOT(importLibraryObject()));
    comboLayout->addWidget(addGC);

    buttonLayout->addLayout(comboLayout);

    TImageButton *addFolderGC = new TImageButton(QPixmap(THEME_DIR + "icons/create_folder.png"), 22, buttons);
    connect(addFolderGC, SIGNAL(clicked()), this, SLOT(addFolder()));
    addFolderGC->setToolTip(tr("Create new folder"));
    buttonLayout->addWidget(addFolderGC);
    // SQA: Temporary code
    // addFolderGC->setEnabled(false);

    TImageButton *gctoDrawingArea = new TImageButton(QPixmap(THEME_DIR + "icons/library_to_ws.png"), 22, buttons);
    connect(gctoDrawingArea, SIGNAL(clicked()), this, SLOT(insertObjectInWorkspace()));
    gctoDrawingArea->setToolTip(tr("Insert library item into frame"));
    buttonLayout->addWidget(gctoDrawingArea);

    buttons->setLayout(buttonLayout);

    addChild(display);
    addChild(buttons);
    addChild(libraryTree);

    watcher = new QFileSystemWatcher(this);
    connect(watcher, SIGNAL(fileChanged(QString)), this, SLOT(updateItemFromSaveAction()));
}

TupLibraryWidget::~TupLibraryWidget()
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[~TupLibraryWidget()]";
        #else
            TEND;
        #endif
    #endif
}

void TupLibraryWidget::resetGUI()
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupLibraryWidget::resetGUI()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    if (library)
        library->reset();

    if (display)
        display->reset();

    if (libraryTree)
        libraryTree->cleanUI();
}

void TupLibraryWidget::setLibrary(TupLibrary *assets)
{
#ifdef TUP_DEBUG
    #ifdef Q_OS_WIN
        qDebug() << "[TupLibraryWidget::setLibrary()]";
    #else
        T_FUNCINFO;
    #endif
#endif

    library = assets;
    project = library->getProject();
}

void TupLibraryWidget::setNetworking(bool netOn)
{
    isNetworked = netOn;
}

void TupLibraryWidget::addFolder(const QString &folderName)
{
    libraryTree->createFolder(folderName);
    mkdir = true;
}

void TupLibraryWidget::updateItemEditionState()
{
    if (editorItems.count() == 2) {
        TupProjectRequest request = TupRequestBuilder::createLibraryRequest(TupProjectRequest::Remove, 
                                                                            editorItems.at(0), TupLibraryObject::Folder);
        emit requestTriggered(&request);
    }

    editorItems.clear();
}

void TupLibraryWidget::activeRefresh(QTreeWidgetItem *item)
{
    mkdir = true;
    refreshItem(item);
}

void TupLibraryWidget::previewItem(QTreeWidgetItem *item)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupLibraryWidget::previewItem()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    RETURN_IF_NOT_LIBRARY;

    if (item) {
        currentItemDisplayed = item;

        if (item->text(2).length() == 0) {
            display->showDisplay();
            QGraphicsTextItem *msg = new QGraphicsTextItem(tr("Directory"));
            display->render(static_cast<QGraphicsItem *>(msg));
            return;
        }

        TupLibraryObject *object = library->getObject(item->text(1) + "." + item->text(2).toLower());

        if (!object) {
            #ifdef TUP_DEBUG
                QString msg = "TupLibraryWidget::previewItem() - Fatal Error: Cannot find the object: " 
                              + item->text(1) + "." + item->text(2).toLower();
                #ifdef Q_OS_WIN
                    qDebug() << msg;
                #else
                    tError() << msg;
                #endif
            #endif

            display->showDisplay();
            QGraphicsTextItem *text = new QGraphicsTextItem(tr("No preview available"));
            display->render(static_cast<QGraphicsItem *>(text));

            return;
        }

        switch (object->getType()) {
                case TupLibraryObject::Svg:
                   {
                     display->showDisplay();
                     QGraphicsSvgItem *svg = new QGraphicsSvgItem(object->getDataPath()); 
                     display->render(static_cast<QGraphicsItem *>(svg));
                   }
                   break;
                case TupLibraryObject::Image:
                case TupLibraryObject::Item:
                   {
                     if (object->getData().canConvert<QGraphicsItem *>()) {
                         display->showDisplay();
                         display->render(qvariant_cast<QGraphicsItem *>(object->getData()));

                         /* SQA: Just a test
                         TupSymbolEditor *editor = new TupSymbolEditor;
                         editor->setSymbol(object);
                         emit postPage(editor);
                         */    
                     } 
                   }
                   break;
                case TupLibraryObject::Sound:
                   {
                     currentSound = object;

                     display->setSoundObject(object->getDataPath());
                     display->showSoundPlayer();
                   }
                   break;
                default:
                   {
                     #ifdef TUP_DEBUG
                         QString msg = "TupLibraryWidget::previewItem() - Unknown symbol id: " + QString::number(object->getType());
                         #ifdef Q_OS_WIN
                             qDebug() << msg;
                         #else
                             tError("library") << msg;
                         #endif
                     #endif
                   }
                   break;
        }
    } else {
        QGraphicsTextItem *msg = new QGraphicsTextItem(tr("No preview available"));
        display->render(static_cast<QGraphicsItem *>(msg));
    }
}

void TupLibraryWidget::insertObjectInWorkspace()
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupLibraryWidget::insertObjectInWorkspace()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    if (libraryTree->topLevelItemCount() == 0) {
        TOsd::self()->display(tr("Error"), tr("Library is empty!"), TOsd::Error);
        #ifdef TUP_DEBUG
            QString msg = "TupLibraryWidget::insertObjectInWorkspace() - Library is empty!";
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tError() << msg;
            #endif
        #endif
        return;
    }

    if (!libraryTree->currentItem()) {
        TOsd::self()->display(tr("Error"), tr("There's no current selection!"), TOsd::Error);
        #ifdef TUP_DEBUG
            QString msg = "TupLibraryWidget::insertObjectInWorkspace() - There's no current selection!";
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tError() << msg;
            #endif
        #endif
        return;
    } 

    QString extension = libraryTree->currentItem()->text(2);
    if (extension.length() == 0) {
        TOsd::self()->display(tr("Error"), tr("It's a directory! Please, pick a graphic object"), TOsd::Error);
        #ifdef TUP_DEBUG
            QString msg = "TupLibraryWidget::insertObjectInWorkspace() - It's a directory!";
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tFatal() << msg;
            #endif
        #endif
        return;
    }

    if ((extension.compare("OGG") == 0) || (extension.compare("WAV") == 0) || (extension.compare("MP3") == 0)) {
        TOsd::self()->display(tr("Error"), tr("It's a sound file! Please, pick a graphic object"), TOsd::Error);
        #ifdef TUP_DEBUG
            QString msg = "TupLibraryWidget::insertObjectInWorkspace() - It's a sound file!";
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tFatal() << msg;
            #endif
        #endif
        return;
    } 

    QString key = libraryTree->currentItem()->text(1) + "." + extension.toLower();
    int objectType = libraryTree->itemType();
    TupProjectRequest request = TupRequestBuilder::createLibraryRequest(TupProjectRequest::InsertSymbolIntoFrame, key,
                                TupLibraryObject::Type(objectType), project->spaceContext(),
                                nullptr, QString(), currentFrame.scene, currentFrame.layer, currentFrame.frame);

    emit requestTriggered(&request);
}

void TupLibraryWidget::removeCurrentItem()
{
    if (!libraryTree->currentItem())
        return;

    TCONFIG->beginGroup("General");
    bool ask = TCONFIG->value("ConfirmRemoveObject", true).toBool();

    if (ask) {
        TOptionalDialog dialog(tr("Do you want to remove this object from Library?"), tr("Confirmation"), this);
        dialog.setModal(true);
        // QDesktopWidget desktop;

        /*
        dialog.move(static_cast<int> ((desktop.screenGeometry().width() - dialog.sizeHint().width())/2),
                    static_cast<int> ((desktop.screenGeometry().height() - dialog.sizeHint().height())/2));
        */

        dialog.move(static_cast<int> ((screen->geometry().width() - dialog.sizeHint().width()) / 2),
                    static_cast<int> ((screen->geometry().height() - dialog.sizeHint().height()) / 2));

        if (dialog.exec() == QDialog::Rejected)
            return;

        TCONFIG->beginGroup("General");
        TCONFIG->setValue("ConfirmRemoveObject", dialog.shownAgain());
        TCONFIG->sync();
    }

    QString objectKey = libraryTree->currentItem()->text(1);
    QString extension = libraryTree->currentItem()->text(2);
    TupLibraryObject::Type type = TupLibraryObject::Folder;

    // If it's NOT a directory
    if (extension.length() > 0) {
        objectKey = libraryTree->currentItem()->text(3);
        if (extension.compare("JPEG") == 0 || extension.compare("JPG") == 0 || extension.compare("PNG") == 0 || extension.compare("GIF") == 0)
            type = TupLibraryObject::Image;
        if (extension.compare("SVG")==0)
            type = TupLibraryObject::Svg;
        if (extension.compare("TOBJ")==0)
            type = TupLibraryObject::Item;
        if ((extension.compare("OGG") == 0) || (extension.compare("WAV") == 0) || (extension.compare("MP3") == 0))
            type = TupLibraryObject::Sound;
    } 

    TupProjectRequest request = TupRequestBuilder::createLibraryRequest(TupProjectRequest::Remove, objectKey, type);
    emit requestTriggered(&request);
}

void TupLibraryWidget::cloneObject(QTreeWidgetItem* item)
{
    if (item) {
        QString id = item->text(3);
        TupLibraryObject *object = library->getObject(id);

        if (object) {
            QString smallId = object->getSmallId();
            QString extension = object->getExtension();
            TupLibraryObject::Type type = object->getType();
            QString path = object->getDataPath();
            int limit = path.lastIndexOf("/");
            QString newPath = path.left(limit + 1); 

            QString symbolName = "";

            if (itemNameEndsWithDigit(smallId)) {
                int index = getItemNameIndex(smallId);
                symbolName = nameForClonedItem(smallId, extension, index, newPath);
                newPath += symbolName;
            } else {
                symbolName = nameForClonedItem(smallId, extension, newPath);
                newPath += symbolName;
            }

            QString baseName = symbolName.section('.', 0, 0);
            baseName = verifyNameAvailability(baseName, extension, true);
            symbolName = baseName + "." + extension.toLower();

            bool isOk = QFile::copy(path, newPath);

            if (!isOk) {
                #ifdef TUP_DEBUG
                    QString msg = "TupLibraryWidget::cloneObject() - Fatal Error: Object file couldn't be cloned!";
                    #ifdef Q_OS_WIN
                        qDebug() << msg;
                    #else
                        tError() << msg;
                    #endif
                #endif
                return;
            }

            TupLibraryObject *newObject = new TupLibraryObject();
            newObject->setSymbolName(symbolName);
            newObject->setType(type);
            newObject->setDataPath(newPath);
            isOk = newObject->loadData(newPath);

            if (!isOk) {
                #ifdef TUP_DEBUG
                    QString msg = "TupLibraryWidget::cloneObject() - Fatal Error: Object file couldn't be loaded!";
                    #ifdef Q_OS_WIN
                        qDebug() << msg;
                    #else
                        tError() << msg;
                    #endif
                #endif
                return;
            } 

            library->addObject(newObject);

            QTreeWidgetItem *item = new QTreeWidgetItem(libraryTree);
            item->setText(1, baseName);
            item->setText(2, extension);
            item->setText(3, symbolName);
            item->setFlags(item->flags() | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled);

            switch (object->getType()) {
                case TupLibraryObject::Item:
                    {
                        item->setIcon(0, QIcon(THEME_DIR + "icons/drawing_object.png"));
                        libraryTree->setCurrentItem(item);
                        previewItem(item);
                    }
                    break;
                case TupLibraryObject::Image:
                    {
                        item->setIcon(0, QIcon(THEME_DIR + "icons/bitmap.png"));
                        libraryTree->setCurrentItem(item);
                        previewItem(item);
                    }
                    break;
                case TupLibraryObject::Svg:
                    {
                        item->setIcon(0, QIcon(THEME_DIR + "icons/svg.png"));
                        libraryTree->setCurrentItem(item);
                        previewItem(item);
                    }
                    break;
                case TupLibraryObject::Sound:
                    {
                        item->setIcon(0, QIcon(THEME_DIR + "icons/sound_object.png"));
                        previewItem(item);
                    }
                    break;
                default:
                    {
                    }
            }
        } else {
            #ifdef TUP_DEBUG
                QString msg = "TupLibraryWidget::cloneObject() - Fatal Error: Object doesn't exist! [ " + id + " ]";
                #ifdef Q_OS_WIN
                    qDebug() << msg;
                #else
                    tError() << msg;
                #endif
            #endif

            return;
        }
    }
}

void TupLibraryWidget::exportObject(QTreeWidgetItem *item)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupLibraryWidget::exportObject()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    if (item) {
        QString id = item->text(3);
        TupLibraryObject *object = library->getObject(id);
        if (object) {
            QString path = object->getDataPath();
            if (path.length() > 0) {
                TupLibraryObject::Type type = object->getType();
                QString fileExtension = object->getExtension();
                QString filter;

                if (type == TupLibraryObject::Image) {
                    filter = tr("Images") + " ";
                    if (fileExtension.compare("PNG") == 0)
                        filter += "(*.png)";
                    if ((fileExtension.compare("JPG") == 0) || (fileExtension.compare("JPEG") == 0))
                        filter += "(*.jpg *.jpeg)";
                    if (fileExtension.compare("GIF") == 0)
                        filter += "(*.gif)";
                    if (fileExtension.compare("XPM") == 0)
                        filter += "(*.xpm)";
                    if (fileExtension.compare("SVG") == 0)
                        filter += "(*.svg)";
                } else if (type == TupLibraryObject::Sound) {
                           filter = tr("Sounds") + " ";
                           if (fileExtension.compare("OGG") == 0)
                               filter += "(*.ogg)";
                           if (fileExtension.compare("MP3") == 0)
                               filter += "(*.mp3)";
                           if (fileExtension.compare("WAV") == 0)
                               filter += "(*.wav)";
                } else if (type == TupLibraryObject::Item) {
                           filter = tr("Native Objects") + " " + "(*.tobj)";
                }

                TCONFIG->beginGroup("General");
                QString defaultPath = TCONFIG->value("DefaultPath", QDir::homePath()).toString();
                QString target = QFileDialog::getSaveFileName(this, tr("Export object..."), defaultPath + "/" + id , filter);

                if (target.isEmpty())
                    return;

                QString filename = target.toUpper();
                if (type == TupLibraryObject::Image) {
                    if (fileExtension.compare("PNG") == 0 && !filename.endsWith(".PNG"))
                        target += ".png";
                    if ((fileExtension.compare("JPG") == 0) && (!filename.endsWith(".JPG") || !filename.endsWith(".JPEG")))
                        target += ".jpg";
                    if (fileExtension.compare("GIF") == 0 && !filename.endsWith(".GIF"))
                        target += ".gif";
                    if (fileExtension.compare("XPM") == 0 && !filename.endsWith(".XPM"))
                        target += ".xpm";
                    if (fileExtension.compare("SVG") == 0 && !filename.endsWith(".SVG"))
                        target += ".svg";
                } else if (type == TupLibraryObject::Sound) {
                           if (fileExtension.compare("OGG") == 0 && !filename.endsWith(".OGG"))
                               target += ".ogg";
                           if (fileExtension.compare("MP3") == 0 && !filename.endsWith(".MP3"))
                               target += ".mp3";
                           if (fileExtension.compare("WAV") == 0 && !filename.endsWith(".WAV"))
                               target += ".wav";
                } else if (type == TupLibraryObject::Item && !filename.endsWith(".TOBJ")) {
                           target += ".tobj";
                }

                if (QFile::exists(target)) {
                    if (!QFile::remove(target)) {
                        #ifdef TUP_DEBUG
                            QString msg = "TupLibraryWidget::exportObject() - Fatal Error: Destination path already exists! [ " + id + " ]";
                            #ifdef Q_OS_WIN
                                qDebug() << msg;
                            #else
                                tError() << msg;
                            #endif
                        #endif
                        return;
                    }
                }

                if (QFile::copy(path, target)) {
                    setDefaultPath(target);
                    TOsd::self()->display(tr("Info"), tr("Item exported successfully!"), TOsd::Info);
                } else {
                    #ifdef TUP_DEBUG
                        QString msg = "TupLibraryWidget::exportObject() - Error: Object file couldn't be exported! [ " + id + " ]";
                        #ifdef Q_OS_WIN
                            qDebug() << msg;
                        #else
                            tError() << msg;
                        #endif
                    #endif
                    return;
                }
            } else {
                #ifdef TUP_DEBUG
                    QString msg = "TupLibraryWidget::exportObject() - Error: Object path is null! [ " + id + " ]";
                    #ifdef Q_OS_WIN
                        qDebug() << msg;
                    #else
                        tError() << msg;
                    #endif
                #endif
                return;
            }
        } else {
            #ifdef TUP_DEBUG
                QString msg = "TupLibraryWidget::exportObject() - Error: Object doesn't exist! [ " + id + " ]";
                #ifdef Q_OS_WIN
                    qDebug() << msg;
                #else
                    tError() << msg;
                #endif
            #endif
            return;
        }
    }
}

void TupLibraryWidget::renameObject(QTreeWidgetItem *item)
{
    if (item) {
        renaming = true;
        oldId = item->text(1);
        libraryTree->editItem(item, 1);
    }
}

void TupLibraryWidget::createRasterObject()
{
    QString name = "object00";
    QString extension = "PNG";
    name = verifyNameAvailability(name, extension, true);

    QSize size = project->getDimension();
    int w = QString::number(size.width()).length();
    int h = QString::number(size.height()).length();

    int width = 1;
    int height = 1; 
    for(int i=0; i<w; i++)
        width *= 10;
    for(int i=0; i<h; i++)
        height *= 10;

    size = QSize(width, height);
    TupNewItemDialog dialog(name, TupNewItemDialog::Raster, size);

    if (dialog.exec() == QDialog::Accepted) {
        QString name = dialog.getItemName();
        QSize size = dialog.itemSize();
        QColor background = dialog.getBackground();
        QString extension = dialog.itemExtension();
        QString editor = dialog.getSoftware();

        QString imagesDir = project->getDataDir() + "/images/";
        if (!QFile::exists(imagesDir)) {
            QDir dir;
            if (!dir.mkpath(imagesDir)) {
                #ifdef TUP_DEBUG
                    QString msg = "TupLibraryWidget::createRasterObject() - Fatal Error: Couldn't create directory " + imagesDir;
                    #ifdef Q_OS_WIN
                        qDebug() << msg;
                    #else
                        tError() << msg;
                    #endif
                #endif
                TOsd::self()->display(tr("Error"), tr("Couldn't create images directory!"), TOsd::Error);
                return;
            }
        }
             
        QString path = imagesDir + name + "." + extension.toLower();
        QString symbolName = name; 
        if (QFile::exists(path)) {
            symbolName = nameForClonedItem(name, extension, imagesDir);
            path = imagesDir + symbolName + "." + extension.toLower();
        }

        symbolName += "." + extension.toLower();

        QImage::Format format = QImage::Format_RGB32;
        if (extension.compare("PNG")==0)
            format = QImage::Format_ARGB32;

        QImage *image = new QImage(size, format); 
        image->fill(background);

        bool isOk = image->save(path);

        if (isOk) {
            TupLibraryObject *newObject = new TupLibraryObject();
            newObject->setSymbolName(symbolName);
            newObject->setType(TupLibraryObject::Image);
            newObject->setDataPath(path);
            isOk = newObject->loadData(path);

            if (isOk) {
                library->addObject(newObject);

                QTreeWidgetItem *item = new QTreeWidgetItem(libraryTree);
                item->setText(1, name);
                item->setText(2, extension);
                item->setText(3, symbolName);
                item->setFlags(item->flags() | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled);
                item->setIcon(0, QIcon(THEME_DIR + "icons/bitmap.png"));
                libraryTree->setCurrentItem(item);

                previewItem(item);

                lastItemEdited = item;
                executeSoftware(editor, path);
            } else {
                #ifdef TUP_DEBUG
                    QString msg = "TupLibraryWidget::createRasterObject() - Fatal Error: Object file couldn't be loaded from -> " + path;
                    #ifdef Q_OS_WIN
                        qDebug() << msg;
                    #else
                        tError() << msg;
                    #endif
                #endif
                return;
            }
        } else {
            #ifdef TUP_DEBUG
                QString msg = "TupLibraryWidget::createRasterObject() - Fatal Error: Object file couldn't be saved at -> " + path;
                #ifdef Q_OS_WIN
                    qDebug() << msg;
                #else
                    tError() << msg;
                #endif
            #endif
            return;
        }
    }
}

void TupLibraryWidget::createVectorObject()
{
    QString name = "object00";
    QString extension = "SVG";
    name = verifyNameAvailability(name, extension, true);

    QSize size = project->getDimension();
    int w = QString::number(size.width()).length();
    int h = QString::number(size.height()).length();

    int width = 1;
    int height = 1;
    for(int i=0; i<w; i++) 
        width *= 10;
    for(int i=0; i<h; i++) 
        height *= 10;

    size = QSize(width, height);

    TupNewItemDialog dialog(name, TupNewItemDialog::Vector, size);
    if (dialog.exec() == QDialog::Accepted) {
        QString name = dialog.getItemName();
        QSize size = dialog.itemSize();
        QString extension = dialog.itemExtension();
        QString editor = dialog.getSoftware();

        QString vectorDir = project->getDataDir() + "/svg/";
        if (!QFile::exists(vectorDir)) {
            QDir dir;
            if (!dir.mkpath(vectorDir)) {
                #ifdef TUP_DEBUG
                    QString msg = "TupLibraryWidget::createVectorObject() - Fatal Error: Couldn't create directory " + vectorDir;
                    #ifdef Q_OS_WIN
                        qDebug() << msg;
                    #else
                        tError() << msg;
                    #endif
                #endif

                TOsd::self()->display(tr("Error"), tr("Couldn't create vector directory!"), TOsd::Error);
                return;
            }
        }

        QString path = vectorDir + name + "." + extension.toLower();
        QString symbolName = name;
        if (QFile::exists(path)) {
            symbolName = nameForClonedItem(name, extension, vectorDir);
            path = vectorDir + symbolName + "." + extension.toLower();
        }

        symbolName += "." + extension.toLower();

        QSvgGenerator generator;
        generator.setFileName(path);
        generator.setSize(size);
        generator.setViewBox(QRect(0, 0, size.width(), size.height()));
        generator.setTitle(name);
        generator.setDescription(tr("TupiTube library item"));
        QPainter painter;
        painter.begin(&generator);
        bool isOk = painter.end();

        if (isOk) {
            QDomDocument doc;
            QFile file(path);
            if (!file.open(QIODevice::ReadOnly)) {
                #ifdef TUP_DEBUG
                    QString msg = "TupLibraryWidget::createVectorObject() - Fatal Error: SVG file couldn't be opened -> " + path;
                    #ifdef Q_OS_WIN
                        qDebug() << msg;
                    #else
                        tError() << msg;
                    #endif
                #endif
                return;
            }
            if (!doc.setContent(&file)) {
                #ifdef TUP_DEBUG
                    QString msg = "TupLibraryWidget::createVectorObject() - Fatal Error: SVG file couldn't be parsed as XML -> " + path;
                    #ifdef Q_OS_WIN
                        qDebug() << msg;
                    #else
                        tError() << msg;
                    #endif
                #endif
                return;
            }
            file.close();

            QDomNodeList roots = doc.elementsByTagName("svg");
            QDomElement root = roots.at(0).toElement(); 
            root.setAttribute("width", size.width());
            root.setAttribute("height", size.height());
            if (!file.open(QIODevice::Truncate | QIODevice::WriteOnly)) {
                #ifdef TUP_DEBUG
                    QString msg = "TupLibraryWidget::createVectorObject() - Fatal Error: SVG file couldn't be updated -> " + path;
                    #ifdef Q_OS_WIN
                        qDebug() << msg;
                    #else
                        tError() << msg;
                    #endif
                #endif
                return;
            } 
            QByteArray xml = doc.toByteArray();
            file.write(xml);
            file.close();

            TupLibraryObject *newObject = new TupLibraryObject();
            newObject->setSymbolName(symbolName);
            newObject->setType(TupLibraryObject::Svg);
            newObject->setDataPath(path);
            isOk = newObject->loadData(path);

            if (isOk) {
                library->addObject(newObject);
                QTreeWidgetItem *item = new QTreeWidgetItem(libraryTree);
                item->setText(1, name);
                item->setText(2, extension);
                item->setText(3, symbolName);
                item->setFlags(item->flags() | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled);
                item->setIcon(0, QIcon(THEME_DIR + "icons/svg.png"));

                libraryTree->setCurrentItem(item);
                previewItem(item);

                lastItemEdited = item;
                executeSoftware(editor, path);
            } else {
                #ifdef TUP_DEBUG
                    QString msg = "TupLibraryWidget::createVectorObject() - Fatal Error: Object file couldn't be loaded from -> " + path;
                    #ifdef Q_OS_WIN
                        qDebug() << msg;
                    #else
                        tError() << msg;
                    #endif
                #endif
                return;
            }
        } else {
                #ifdef TUP_DEBUG
                    QString msg = "TupLibraryWidget::createVectorObject() - Fatal Error: Object file couldn't be saved at -> " + path;
                    #ifdef Q_OS_WIN
                        qDebug() << msg;
                    #else
                        tError() << msg;
                    #endif
                #endif
                return;
        }

    }
}

void TupLibraryWidget::importImageGroup()
{
    TCONFIG->beginGroup("General");
    QString path = TCONFIG->value("DefaultPath", QDir::homePath()).toString();

    QFileDialog dialog(this, tr("Import images..."), path);
    dialog.setNameFilter(tr("Images") + " (*.png *.xpm *.jpg *.jpeg *.gif)");
    dialog.setFileMode(QFileDialog::ExistingFiles);

    if (dialog.exec() == QDialog::Accepted) {
        QStringList files = dialog.selectedFiles();
        int size = files.size();
        for (int i = 0; i < size; ++i)
             importImage(files.at(i));

        setDefaultPath(files.at(0));
    }
}

void TupLibraryWidget::importImage(const QString &imagePath)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupLibraryWidget::importImage()]";
    #endif

    if (imagePath.isEmpty())
        return;

    QFile imageFile(imagePath);

    if (imageFile.open(QIODevice::ReadOnly)) {
        QFileInfo fileInfo(imageFile);
        QString key = fileInfo.fileName().toLower();
        key = key.replace("(","_");
        key = key.replace(")","_");
        int index = key.lastIndexOf(".");

        QString name = key.mid(0, index);
        if (name.length() > 30)
            name = key.mid(0, 30);

        QString extension = key.mid(index, key.length() - index);
        QByteArray data = imageFile.readAll();
        imageFile.close();

        QPixmap *pixmap = new QPixmap(imagePath);
        int picWidth = pixmap->width();
        int picHeight = pixmap->height();
        int projectWidth = project->getDimension().width();
        int projectHeight = project->getDimension().height();

        #ifdef TUP_DEBUG
            #ifdef Q_OS_WIN
               qDebug() << "TupLibraryWidget::importImage() - Image filename: " << key << " | Raw Size: " << data.size();
               qDebug() << "TupLibraryWidget::importImage() - Image Size: " << "[" << picWidth << ", " << picHeight << "]"
                        << " | Project Size: " << "[" << projectWidth << ", " << projectHeight << "]";
            #else
               tFatal() << "TupLibraryWidget::importImage() - Image filename: " << key << " | Raw Size: " << data.size();
               tFatal() << "TupLibraryWidget::importImage() - Image Size: " << "[" << picWidth << ", " << picHeight << "]"
                        << " | Project Size: " << "[" << projectWidth << ", " << projectHeight << "]";
            #endif
        #endif

        if (picWidth > projectWidth || picHeight > projectHeight) {
            // QDesktopWidget desktop;
            QMessageBox msgBox;
            msgBox.setWindowTitle(tr("Information"));
            msgBox.setIcon(QMessageBox::Question);
            msgBox.setText(tr("Image is bigger than workspace."));
            msgBox.setInformativeText(tr("Do you want to resize it?"));
            msgBox.setStandardButtons(QMessageBox::No | QMessageBox::Yes);
            msgBox.setDefaultButton(QMessageBox::Ok);
            msgBox.show();

            /*
            msgBox.move(static_cast<int> ((desktop.screenGeometry().width() - msgBox.width())/2),
                        static_cast<int> ((desktop.screenGeometry().height() - msgBox.height())/2));
            */

            msgBox.move(static_cast<int> ((screen->geometry().width() - msgBox.width()) / 2),
                        static_cast<int> ((screen->geometry().height() - msgBox.height()) / 2));

            int answer = msgBox.exec();

            if (answer == QMessageBox::Yes) {
                msgBox.close();
                pixmap = new QPixmap();
                QString extension = fileInfo.suffix().toUpper();
                QByteArray ba = extension.toLatin1();
                const char* ext = ba.data();
                if (pixmap->loadFromData(data, ext)) {
                    QPixmap newpix;
                    if (picWidth > picHeight) {
                        newpix = QPixmap(pixmap->scaledToHeight(projectHeight, Qt::SmoothTransformation));
                    } else {
                        newpix = QPixmap(pixmap->scaledToWidth(projectWidth, Qt::SmoothTransformation));
                    }
                    QBuffer buffer(&data);
                    buffer.open(QIODevice::WriteOnly);
                    newpix.save(&buffer, ext);
                }
            } 
        }

        int i = 0;
        while (library->exists(key)) {
            i++;
            key = name + "-" + QString::number(i) + extension;
        }

        TupProjectRequest request = TupRequestBuilder::createLibraryRequest(TupProjectRequest::Add, key,
                                                                            TupLibraryObject::Image, project->spaceContext(), data, QString(),
                                                                            currentFrame.scene, currentFrame.layer, currentFrame.frame);
        emit requestTriggered(&request);

        data.clear();
    } else {
        TOsd::self()->display(tr("Error"), tr("Cannot open file: %1").arg(imagePath), TOsd::Error);
    }
}

void TupLibraryWidget::importSvgGroup()
{
    TCONFIG->beginGroup("General");
    QString path = TCONFIG->value("DefaultPath", QDir::homePath()).toString();

    QFileDialog dialog(this, tr("Import SVG files..."), path);
    dialog.setNameFilter(tr("Vector") + " (*.svg)");
    dialog.setFileMode(QFileDialog::ExistingFiles);

    if (dialog.exec() == QDialog::Accepted) {
        QStringList files = dialog.selectedFiles();
        int size = files.size();
        for (int i = 0; i < size; ++i)
            importSvg(files.at(i));

        setDefaultPath(files.at(0));
    }
}

void TupLibraryWidget::importSvg(const QString &svgPath)
{
    if (svgPath.isEmpty())
        return;

    QFile file(svgPath);

    if (file.open(QIODevice::ReadOnly)) {
        QFileInfo fileInfo(file);
        QString key = fileInfo.fileName().toLower();
        key = key.replace("(","_");
        key = key.replace(")","_");

        int index = key.lastIndexOf(".");
        QString name = key.mid(0, index);
        if (name.length() > 30)
            name = key.mid(0, 30);

        QString extension = key.mid(index, key.length() - index);

        QByteArray data = file.readAll();
        file.close();

        #ifdef TUP_DEBUG
            QString msg1 = "TupLibraryWidget::importSvg() - Inserting SVG into project: " + project->getName();
            int projectWidth = project->getDimension().width();
            int projectHeight = project->getDimension().height();
            QString msg2 = "TupLibraryWidget::importSvg() - Project Size: [" + QString::number(projectWidth) + QString(", ")
                    + QString::number(projectHeight) + QString("]");

            #ifdef Q_OS_WIN
                qDebug() << msg1;
                qDebug() << msg2;
            #else
                tFatal() << msg1;
                tFatal() << msg2;
            #endif
        #endif

        int i = 0;
        while (library->exists(key)) {
            i++;
            key = name + "-" + QString::number(i) + extension;
        }

        TupProjectRequest request = TupRequestBuilder::createLibraryRequest(TupProjectRequest::Add, key,
                                                       TupLibraryObject::Svg, project->spaceContext(), data, QString(),
                                                       currentFrame.scene, currentFrame.layer, currentFrame.frame);
        emit requestTriggered(&request);
    } else {
        TOsd::self()->display(tr("Error"), tr("Cannot open file: %1").arg(svgPath), TOsd::Error);
    }
}

void TupLibraryWidget::importNativeObjects()
{
    TCONFIG->beginGroup("General");
    QString path = TCONFIG->value("DefaultPath", QDir::homePath()).toString();

    QFileDialog dialog(this, tr("Import objects..."), path);
    dialog.setNameFilter(tr("Native Objects") + " (*.tobj)");
    dialog.setFileMode(QFileDialog::ExistingFiles);

    if (dialog.exec() == QDialog::Accepted) {
        QStringList files = dialog.selectedFiles();
        int size = files.size();
        for (int i = 0; i < size; ++i)
            importNativeObject(files.at(i));

        setDefaultPath(files.at(0));
    }
}

void TupLibraryWidget::importNativeObject(const QString &object)
{
    if (object.isEmpty())
        return;

    QFile file(object);
    if (file.open(QIODevice::ReadOnly)) {
        QFileInfo fileInfo(file);
        QString key = fileInfo.fileName().toLower();
        key = key.replace("(","_");
        key = key.replace(")","_");
        QByteArray data = file.readAll();
        file.close();

        #ifdef TUP_DEBUG
            QString msg1 = "TupLibraryWidget::importNativeObject() - Inserting native object into project: "
                           + project->getName();
            #ifdef Q_OS_WIN
                qDebug() << msg1;
            #else
                tFatal() << msg1;
            #endif
        #endif

        int i = 0;
        int index = key.lastIndexOf(".");
        QString name = key.mid(0, index);
        QString extension = key.mid(index, key.length() - index);
        while (library->exists(key)) {
            i++;
            key = name + "-" + QString::number(i) + extension;
        }

        TupProjectRequest request = TupRequestBuilder::createLibraryRequest(TupProjectRequest::Add, key,
                                                       TupLibraryObject::Item, project->spaceContext(), data, QString(),
                                                       currentFrame.scene, currentFrame.layer, currentFrame.frame);
        emit requestTriggered(&request);
    } else {
        TOsd::self()->display(tr("Error"), tr("Cannot open file: %1").arg(object), TOsd::Error);
    }
}

void TupLibraryWidget::verifyFramesAvailability(int filesTotal)
{
    TupScene *scene = project->sceneAt(currentFrame.scene);
    TupLayer *layer = scene->layerAt(currentFrame.layer);
    int framesTotal = layer->framesCount();
    int initFrame = currentFrame.frame;
    int scope = initFrame + filesTotal;
    if (scope > framesTotal) {
        for (int i=framesTotal; i<scope; i++) {
             TupProjectRequest request = TupRequestBuilder::createFrameRequest(currentFrame.scene, currentFrame.layer,
                                                                               i, TupProjectRequest::Add, tr("Frame"));
             emit requestTriggered(&request);
        }
        TupProjectRequest request = TupRequestBuilder::createFrameRequest(currentFrame.scene, currentFrame.layer, initFrame,
                                                                          TupProjectRequest::Select);
        emit requestTriggered(&request);
    }
}

QStringList TupLibraryWidget::naturalSort(QStringList photograms)
{ 
    QCollator coll;
    coll.setNumericMode(true);
    for (int i = photograms.size()-1; i >= 0; i--) {
         for (int j = 1; j <= i; j++) {
              if (coll.compare(photograms.at(j-1), photograms.at(j)) > 0)
                  photograms.swapItemsAt(j-1, j);
         }
    }

    return photograms;
}

void TupLibraryWidget::importImageSequence()
{
    TCONFIG->beginGroup("General");
    QString path = TCONFIG->value("DefaultPath", QDir::homePath()).toString();

    QFileDialog dialog(this, tr("Choose the images directory..."), path);
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setOption(QFileDialog::ShowDirsOnly);
    dialog.setOption(QFileDialog::DontResolveSymlinks);

    if (dialog.exec() == QDialog::Accepted) {
        QStringList files = dialog.selectedFiles();
        path = files.at(0);

        QDir source(path); 
        QFileInfoList records = source.entryInfoList(QDir::Files, QDir::Name);
        int filesTotal = records.size();

        // Ensuring to get only graphic files here. Check extensions! (PNG, JPG, GIF, XPM) 
        int imagesCounter = 0; 
        QStringList photograms;
        for (int i = 0; i < filesTotal; ++i) {
             if (records.at(i).isFile()) {
                 QString extension = records.at(i).suffix().toUpper();
                 if (extension.compare("JPEG")==0 || extension.compare("JPG")==0 || extension.compare("PNG")==0 || extension.compare("GIF")==0 || 
                     extension.compare("XPM")==0) {
                     imagesCounter++;
                     photograms << records.at(i).absoluteFilePath();
                 }
             }
        }

        if (imagesCounter > 0) {
            QString text = tr("Image files found: %1.").arg(imagesCounter);
            bool resize = false;

            QPixmap *pixmap = new QPixmap(photograms.at(0));
            int picWidth = pixmap->width();
            int picHeight = pixmap->height(); 
            int projectWidth = project->getDimension().width();
            int projectHeight = project->getDimension().height();

            if (picWidth > projectWidth || picHeight > projectHeight) {
                text = text + "\n" + tr("Files are too big, so they will be resized.") + "\n"
                       + tr("Note: This task can take a while.");
                resize = true;
            }

            // QDesktopWidget desktop;
            QMessageBox msgBox;
            msgBox.setWindowTitle(tr("Information"));  
            msgBox.setIcon(QMessageBox::Question);
            msgBox.setText(text);
            msgBox.setInformativeText(tr("Do you want to continue?"));
            msgBox.setStandardButtons(QMessageBox::Cancel | QMessageBox::Ok);
            msgBox.setDefaultButton(QMessageBox::Ok);
            msgBox.show();

            /*
            msgBox.move(static_cast<int> ((desktop.screenGeometry().width() - msgBox.width())/2),
                        static_cast<int> ((desktop.screenGeometry().height() - msgBox.height())/2));
            */

            msgBox.move(static_cast<int> ((screen->geometry().width() - msgBox.width()) / 2),
                        static_cast<int> ((screen->geometry().height() - msgBox.height()) / 2));

            int answer = msgBox.exec();
            if (answer == QMessageBox::Ok) {
                msgBox.close();
                verifyFramesAvailability(filesTotal);

                QString directory = source.dirName();
                libraryTree->createFolder(directory);

                QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

                TupLibraryFolder *folder = new TupLibraryFolder(directory, project);
                library->addFolder(folder);

                photograms = naturalSort(photograms);

                int initFrame = currentFrame.frame;
                int filesTotal = photograms.size();
                QPixmap newpix;
                for (int i = 0; i < filesTotal; i++) {
                     QFile file(photograms.at(i));
                     QFileInfo fileInfo(file);
                     QString extension = fileInfo.suffix().toUpper();
                     if (extension.compare("JPEG")==0 || extension.compare("JPG")==0 || extension.compare("PNG")==0 || extension.compare("GIF")==0 ||
                         extension.compare("XPM")==0) {
                         QString symName = fileInfo.fileName().toLower();
                         symName = symName.replace("(","_");
                         symName = symName.replace(")","_");

                         if (file.open(QIODevice::ReadOnly)) {
                             QByteArray data = file.readAll();
                             file.close();

                             if (resize) {
                                 pixmap = new QPixmap();
                                 QString extension = fileInfo.suffix().toUpper();
                                 QByteArray ba = extension.toLatin1();
                                 const char* ext = ba.data();
                                 if (pixmap->loadFromData(data, ext)) {
                                     if (picWidth > picHeight)
                                         newpix = QPixmap(pixmap->scaledToWidth(projectWidth, Qt::SmoothTransformation));
                                     else
                                         newpix = QPixmap(pixmap->scaledToHeight(projectHeight, Qt::SmoothTransformation));
                                     QBuffer buffer(&data);
                                     buffer.open(QIODevice::WriteOnly);
                                     newpix.save(&buffer, ext);
                                 }
                             }
                           
                             TupProjectRequest request = TupRequestBuilder::createLibraryRequest(TupProjectRequest::Add, symName,
                                                         TupLibraryObject::Image, project->spaceContext(), data, directory);
                             emit requestTriggered(&request);
                             if (i < filesTotal-1) {
                                 request = TupRequestBuilder::createFrameRequest(currentFrame.scene, currentFrame.layer, currentFrame.frame + 1,
                                                                                 TupProjectRequest::Select);
                                 emit requestTriggered(&request);
                             }

                             /*
                             progressDialog.setLabelText(tr("Loading image #%1").arg(index));
                             progressDialog.setValue(index);
                             index++;
                             */
                         } else {
                             QMessageBox::critical(this, tr("ERROR!"), tr("ERROR: Can't open file %1. Please, check file permissions and try again.").arg(symName), QMessageBox::Ok);
                             QApplication::restoreOverrideCursor();
                             return;
                         }
                     }
                }
                saveDefaultPath(path);
                TupProjectRequest request = TupRequestBuilder::createFrameRequest(currentFrame.scene, currentFrame.layer, initFrame,
                                                                                  TupProjectRequest::Select);
                emit requestTriggered(&request);

                QApplication::restoreOverrideCursor();
            }
        } else {
            TOsd::self()->display(tr("Error"), tr("No image files were found.<br/>Please, try another directory"), TOsd::Error);
        }
    }
}

void TupLibraryWidget::importSvgSequence() 
{
    TCONFIG->beginGroup("General");
    QString path = TCONFIG->value("DefaultPath", QDir::homePath()).toString();

    QFileDialog dialog(this, tr("Choose the SVG files directory..."), path);
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setOption(QFileDialog::ShowDirsOnly);
    dialog.setOption(QFileDialog::DontResolveSymlinks);

    if (dialog.exec() == QDialog::Accepted) {
        QStringList files = dialog.selectedFiles();
        path = files.at(0);

        QDir source(path); 
        QFileInfoList records = source.entryInfoList(QDir::Files, QDir::Name);
        int filesTotal = records.size();
        QStringList photograms;

        // Ensuring to get only SVG files here. Check extension! (SVG)
        int svgCounter = 0;
        for (int i = 0; i < filesTotal; ++i) {
             if (records.at(i).isFile()) {
                 QString extension = records.at(i).suffix().toUpper();
                 if (extension.compare("SVG")==0) {
                     svgCounter++;
                     photograms << records.at(i).absoluteFilePath(); 
                 }
             }
        }

        if (svgCounter > 0) {
            QString text = tr("%1 SVG files will be loaded.").arg(svgCounter);

            // QDesktopWidget desktop;
            QMessageBox msgBox;
            msgBox.setWindowTitle(tr("Information"));  
            msgBox.setIcon(QMessageBox::Question);
            msgBox.setText(text);
            msgBox.setInformativeText(tr("Do you want to continue?"));
            msgBox.setStandardButtons(QMessageBox::Cancel | QMessageBox::Ok);
            msgBox.setDefaultButton(QMessageBox::Ok);
            msgBox.show();

            /*
            msgBox.move(static_cast<int> ((desktop.screenGeometry().width() - msgBox.width())/2),
                        static_cast<int> ((desktop.screenGeometry().height() - msgBox.height())/2));
            */

            msgBox.move(static_cast<int> ((screen->geometry().width() - msgBox.width()) / 2),
                        static_cast<int> ((screen->geometry().height() - msgBox.height()) / 2));


            int answer = msgBox.exec();

            if (answer == QMessageBox::Ok) {
                msgBox.close();
                verifyFramesAvailability(filesTotal);
                QString directory = source.dirName();
                libraryTree->createFolder(directory);

                QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

                TupLibraryFolder *folder = new TupLibraryFolder(directory, project);
                library->addFolder(folder);

                photograms = naturalSort(photograms);

                int initFrame = currentFrame.frame;
                filesTotal = photograms.size();
                QByteArray data;
                for (int i = 0; i < filesTotal; ++i) {
                     QFile file(photograms.at(i));
                     QFileInfo fileInfo(file);
                     QString extension = fileInfo.suffix().toUpper();
                     if (extension.compare("SVG")==0) {
                         QString symName = fileInfo.fileName().toLower();
                         symName = symName.replace("(","_");
                         symName = symName.replace(")","_");

                         if (file.open(QIODevice::ReadOnly)) {
                             data = file.readAll();
                             file.close();

                             TupProjectRequest request = TupRequestBuilder::createLibraryRequest(TupProjectRequest::Add, symName,
                                                                            TupLibraryObject::Svg, project->spaceContext(), data, directory);
                             emit requestTriggered(&request);
                             if (i < filesTotal-1) {
                                 request = TupRequestBuilder::createFrameRequest(currentFrame.scene, currentFrame.layer, currentFrame.frame + 1,
                                                                                 TupProjectRequest::Select);
                                 emit requestTriggered(&request);
                             }

                             /*
                             progressDialog.setLabelText(tr("Loading SVG file #%1").arg(index));
                             progressDialog.setValue(index);
                             index++;
                             */
                         } else {
                             QMessageBox::critical(this, tr("ERROR!"), tr("ERROR: Can't open file %1. Please, check file permissions and try again.").arg(symName), QMessageBox::Ok);
                             QApplication::restoreOverrideCursor();
                             return;
                         }
                     }
                }
                saveDefaultPath(path);
                
                TupProjectRequest request = TupRequestBuilder::createFrameRequest(currentFrame.scene, currentFrame.layer, initFrame,
                                                                                  TupProjectRequest::Select);
                emit requestTriggered(&request);
                QApplication::restoreOverrideCursor();
            }
        } else {
            TOsd::self()->display(tr("Error"), tr("No SVG files were found.<br/>Please, try another directory"), TOsd::Error);
        }
    }
}

void TupLibraryWidget::importSoundFile()
{
    TCONFIG->beginGroup("General");
    QString path = TCONFIG->value("DefaultPath", QDir::homePath()).toString();

    QFileDialog dialog(this, tr("Import audio file..."), path);
    dialog.setNameFilter(tr("Sound file") + " (*.ogg *.wav *.mp3)");
    dialog.setFileMode(QFileDialog::ExistingFile);

    if (dialog.exec() == QDialog::Accepted) {
        QStringList files = dialog.selectedFiles();
        path = files.at(0);

        QFile file(path);
        QFileInfo fileInfo(file);
        QString key = fileInfo.fileName().toLower();
        key = key.replace("(","_");
        key = key.replace(")","_");

        if (file.open(QIODevice::ReadOnly)) {
            QByteArray data = file.readAll();
            file.close();

            isEffectSound = true;
            TupProjectRequest request = TupRequestBuilder::createLibraryRequest(TupProjectRequest::Add, key,
                                                           TupLibraryObject::Sound, project->spaceContext(), data);
            emit requestTriggered(&request);
            setDefaultPath(path);
        } else {
            TOsd::self()->display(tr("Error"), tr("Error while opening file: %1").arg(path), TOsd::Error);
        }
    }
}

void TupLibraryWidget::sceneResponse(TupSceneResponse *response)
{
    switch (response->getAction()) {
        case TupProjectRequest::Select:
        {
            currentFrame.frame = 0;
            currentFrame.layer = 0;
            currentFrame.scene = response->getSceneIndex();
        }
        break;
    }
}

void TupLibraryWidget::libraryResponse(TupLibraryResponse *response)
{
    RETURN_IF_NOT_LIBRARY;

    switch (response->getAction()) {
            case TupProjectRequest::Add:
              {
                 if (response->symbolType() == TupLibraryObject::Folder) {
                     libraryTree->createFolder(response->getArg().toString());
                     return;
                 }

                 QString folderName = response->getParent(); 
                 QString id = response->getArg().toString();

                 int index = id.lastIndexOf(".");
                 QString name = id.mid(0, index);
                 QString extension = id.mid(index + 1, id.length() - index).toUpper();
                 TupLibraryObject *obj = library->getObject(id);

                 if (index < 0)
                     extension = "TOBJ"; 

                 QTreeWidgetItem *item;
                 if (folderName.length() > 0 && folderName.compare("library")!=0)
                     item = new QTreeWidgetItem(libraryTree->getFolder(folderName));
                 else
                     item = new QTreeWidgetItem(libraryTree);

                 item->setText(1, name);
                 item->setText(2, extension);
                 item->setText(3, id);
                 item->setFlags(item->flags() | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled);

                 if (obj) {
                     switch (obj->getType()) {
                            case TupLibraryObject::Item:
                               {
                                 item->setIcon(0, QIcon(THEME_DIR + "icons/drawing_object.png"));
                                 libraryTree->setCurrentItem(item);
                                 previewItem(item);
                                 if (!isNetworked && !library->isLoadingProject())
                                     insertObjectInWorkspace();
                               }
                            break;
                            case TupLibraryObject::Image:
                               {
                                 item->setIcon(0, QIcon(THEME_DIR + "icons/bitmap.png"));
                                 libraryTree->setCurrentItem(item);
                                 previewItem(item);
                                 if (!isNetworked && !folderName.endsWith(".pgo") && !library->isLoadingProject())
                                     insertObjectInWorkspace();
                               }
                            break;
                            case TupLibraryObject::Svg:
                               {
                                 item->setIcon(0, QIcon(THEME_DIR + "icons/svg.png"));
                                 libraryTree->setCurrentItem(item);
                                 previewItem(item);
                                 if (!isNetworked && !library->isLoadingProject())
                                     insertObjectInWorkspace();
                               }
                            break;
                            case TupLibraryObject::Sound:
                               {
                                 TupLibraryObject *object = library->getObject(id);
                                 if (object) {
                                     if (isEffectSound) {
                                         object->setSoundEffectFlag(true);
                                         isEffectSound = false;
                                     }
                                 }

                                 item->setIcon(0, QIcon(THEME_DIR + "icons/sound_object.png"));
                                 libraryTree->setCurrentItem(item);
                                 previewItem(item);
                               }
                            break;
                            default:
                               {
                               }
                            break;
                     }

                 } else {
                     #ifdef TUP_DEBUG
                         QString msg = "TupLibraryWidget::libraryResponse() - No object found: " + id;
                         #ifdef Q_OS_WIN
                             qDebug() << msg;
                         #else
                             tFatal() << msg;
                         #endif
                     #endif
                 }
              }
            break;
            case TupProjectRequest::InsertSymbolIntoFrame:
              {
                 #ifdef TUP_DEBUG
                     QString msg = "TupLibraryWidget::libraryResponse() -> InsertSymbolIntoFrame : No action taken";
                     #ifdef Q_OS_WIN
                         qDebug() << msg;
                     #else
                         tFatal() << msg;
                     #endif
                 #endif
              }
            break;
            case TupProjectRequest::RemoveSymbolFromFrame:
              {
                 #ifdef TUP_DEBUG
                     QString msg = "TupLibraryWidget::libraryResponse() -> RemoveSymbolFromFrame : No action taken";
                     #ifdef Q_OS_WIN
                         qDebug() << msg;
                     #else
                         tFatal() << msg;
                     #endif
                 #endif
              }
            break;
            case TupProjectRequest::Remove:
              {
                 QString id = response->getArg().toString();
                 QTreeWidgetItemIterator it(libraryTree);
                 while ((*it)) {
                        // If target is NOT a folder
                        if ((*it)->text(2).length() > 0) {
                            if (id == (*it)->text(3)) {
                                delete (*it);
                                break;
                            }
                        } else {
                            // If target is a folder
                            if (id == (*it)->text(1)) {
                                delete (*it);
                                break;
                            }
                        }
                        ++it;
                 }

                 if (libraryTree->topLevelItemCount() > 0) {
                     previewItem(libraryTree->currentItem());
                 } else  {
                     display->showDisplay();
                     display->reset();
                 }
              }
            break;
            default:
              {
                 #ifdef TUP_DEBUG
                     QString msg = "TupLibraryWidget::libraryResponse() - Unknown/Unhandled project action: "
                             + QString::number(response->getAction());
                     #ifdef Q_OS_WIN
                         qDebug() << msg;
                     #else
                         tFatal() << msg;
                     #endif
                 #endif
              }
            break;
    }
}

void TupLibraryWidget::frameResponse(TupFrameResponse *response)
{
#ifdef TUP_DEBUG
    #ifdef Q_OS_WIN
        qDebug() << "[TupLibraryWidget::frameResponse()]";
    #else
        T_FUNCINFO << "frameIndex: " << response->getFrameIndex();
        T_FUNCINFO << "action: " << response->getAction();
    #endif
#endif

    if (response->getAction() == TupProjectRequest::Add
        || response->getAction() == TupProjectRequest::Select) {
        currentFrame.frame = response->getFrameIndex();
        currentFrame.layer = response->getLayerIndex();
        currentFrame.scene = response->getSceneIndex();
    }
}

void TupLibraryWidget::importLibraryObject()
{
    QString option = itemType->currentText();

    if (option.compare(tr("Image")) == 0) {
        importImageGroup();
        return;
    }

    if (option.compare(tr("Image Sequence")) == 0) {
        importImageSequence();
        return;
    }

    if (option.compare(tr("Svg File")) == 0) {
        importSvgGroup();
        return;
    }

    if (option.compare(tr("Svg Sequence")) == 0) {
        importSvgSequence();
        return;
    }

    if (option.compare(tr("Native Object")) == 0) {
        importNativeObjects();
        return;
    }

    if (option.compare(tr("Sound File")) == 0) {
        importSoundFile();
        return;
    }
}

void TupLibraryWidget::refreshItem(QTreeWidgetItem *item)
{
    if (!item)
        return;

    if (mkdir) {
        mkdir = false;

        QString base = item->text(1);
        if (base.length() == 0)
            return;

        QString tag = base;
        int i = 0;
        while (library->folderExists(tag)) {
               int index = tag.lastIndexOf("-");
               if (index < 0) {
                   tag = base + "-1";
               } else {
                   QString name = base.mid(0, index);
                   i++;
                   tag = name + "-" + QString::number(i);
               }
        }

        item->setText(1, tag);

        TupLibraryFolder *folder = new TupLibraryFolder(tag, project);
        library->addFolder(folder);

        QGraphicsTextItem *msg = new QGraphicsTextItem(tr("Directory"));
        display->render(static_cast<QGraphicsItem *>(msg));

        editorItems << tag;

        return;
    }

    if (renaming) {
        // Renaming directory
        if (libraryTree->isFolder(item)) {
            QString base = item->text(1);
            if (oldId.length() == 0 || base.length() == 0)
                return;

            if (oldId.compare(base) == 0)
                return;

            int i = 0;
            QString tag = base;
            while (library->folderExists(tag)) {
                   int index = tag.lastIndexOf("-");
                   if (index < 0) {
                       tag = base + "-1";
                   } else {
                       QString name = base.mid(0, index);
                       i++;
                       tag = name + "-" + QString::number(i);
                   }
            }

            if (!library->folderExists(tag)) {
                // rename directory here!
                if (library->folderExists(oldId)) {
                    bool renamed = library->renameFolder(oldId, tag);
                    if (renamed)
                        item->setText(1, tag);
                }
            } 
        } else {
            // Renaming item
            if (oldId.length() == 0)
                return;

            QString newId = item->text(1);
            QString extension = item->text(2);

            if (oldId.compare(newId) != 0) {
                newId = verifyNameAvailability(newId, extension, false);
                QString oldRef = oldId + "." + extension.toLower();
                item->setText(1, newId);

                newId = newId + "." + extension.toLower();
                item->setText(3, newId);

                QTreeWidgetItem *parent = item->parent();
                if (parent) 
                    library->renameObject(parent->text(1), oldRef, newId);
                else
                    library->renameObject("", oldRef, newId);

                TupLibraryObject::Type type = TupLibraryObject::Image;
                if (extension.compare("SVG")==0)
                    type = TupLibraryObject::Svg;
                if (extension.compare("TOBJ")==0)
                    type = TupLibraryObject::Item;

                project->updateSymbolId(type, oldRef, newId);
            }
        }

        renaming = false;
    }
}

void TupLibraryWidget::updateLibrary(QString file, QString folder) 
{
    #ifdef TUP_DEBUG
        QString msg1 = "TupLibraryWidget::updateLibrary() - folder: " + folder;
        QString msg2 = "TupLibraryWidget::updateLibrary() - file: " + file;

        #ifdef Q_OS_WIN
            qDebug() << msg1;
            qDebug() << msg2;
        #else
            tError() << msg1;
            tError() << msg2;
        #endif
    #endif

    if (folder.length() > 0)
        library->moveObject(file, folder);
    else
        library->moveObjectToRoot(file);
}

void TupLibraryWidget::openInkscapeToEdit(QTreeWidgetItem *item)
{
    callExternalEditor(item, "Inkscape");
}

void TupLibraryWidget::openGimpToEdit(QTreeWidgetItem *item)
{
    callExternalEditor(item, "Gimp");
}

void TupLibraryWidget::openKritaToEdit(QTreeWidgetItem *item)
{
    callExternalEditor(item, "Krita");
}

void TupLibraryWidget::openMyPaintToEdit(QTreeWidgetItem *item)
{
    callExternalEditor(item, "MyPaint");
}

void TupLibraryWidget::callExternalEditor(QTreeWidgetItem *item, const QString &software)
{
    if (item) {
        lastItemEdited = item;
        QString id = item->text(1) + "." + item->text(2).toLower();
        TupLibraryObject *object = library->getObject(id);

        if (object) {
            QString path = object->getDataPath();
            executeSoftware(software, path);
        } else {
            #ifdef TUP_DEBUG
                QString msg = "TupLibraryWidget::callExternalEditor() - Fatal Error: No object related to the current library item -" + id + "- was found!";
                #ifdef Q_OS_WIN
                    qDebug() << msg;
                #else
                    tError() << msg;
                #endif
            #endif
        }
    } else {
        #ifdef TUP_DEBUG
            QString msg = "TupLibraryWidget::callExternalEditor() - Error: Current library item is invalid!";
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tError() << msg;
            #endif
        #endif
    }
}

void TupLibraryWidget::executeSoftware(const QString &software, QString &path)
{
    if (path.length() > 0 && QFile::exists(path)) {
        QString program = "/usr/bin/" + software.toLower(); 

        QStringList arguments;
        arguments << path;

        QProcess *editor = new QProcess(this);
        editor->start(program, arguments);

        // SQA: Check the path list and if it doesn't exist yet, then add it to 
        watcher->addPath(path);
    } else {
        #ifdef TUP_DEBUG
            QString msg = "TupLibraryWidget::executeSoftware() - Fatal Error: Item path either doesn't exist or is empty";
            #ifdef Q_OS_WIN
                qDebug() << msg;
            #else
                tError() << msg;
            #endif
        #endif
    }
}

void TupLibraryWidget::updateItemFromSaveAction()
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupLibraryWidget::updateItemFromSaveAction()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    LibraryObjects collection = library->getObjects();
    QMapIterator<QString, TupLibraryObject *> i(collection);
    while (i.hasNext()) {
           i.next();
           TupLibraryObject *object = i.value();
           if (object) {
               updateItem(object->getSmallId(), object->getExtension().toLower(), object);
           } else {
               #ifdef TUP_DEBUG
                   QString msg = "TupLibraryWidget::updateItemFromSaveAction() - Fatal Error: The library item modified was not found!";
                   #ifdef Q_OS_WIN
                       qDebug() << msg;
                   #else
                       tError() << msg;
                   #endif
               #endif
           }
    }

    TupProjectRequest request = TupRequestBuilder::createFrameRequest(currentFrame.scene, currentFrame.layer, currentFrame.frame,
                                                                      TupProjectRequest::Select);
    emit requestTriggered(&request);
}

void TupLibraryWidget::updateItem(const QString &name, const QString &extension, TupLibraryObject *object)
{
    #ifdef TUP_DEBUG
        #ifdef Q_OS_WIN
            qDebug() << "[TupLibraryWidget::updateItem()]";
        #else
            T_FUNCINFO;
        #endif
    #endif

    QString onEdition = name + "." + extension;
    QString onDisplay = currentItemDisplayed->text(1) + "." + currentItemDisplayed->text(2).toLower();

    TupLibraryObject::Type type = TupLibraryObject::Image;
    if (extension.compare("svg") == 0)
        type = TupLibraryObject::Svg;

    bool isOk = library->reloadObject(onEdition);
    if (isOk) 
        project->reloadLibraryItem(type, onEdition, object);

    if (onDisplay.compare(onEdition) == 0)
        previewItem(lastItemEdited);
}

bool TupLibraryWidget::itemNameEndsWithDigit(QString &name)
{
    QByteArray array = name.toLocal8Bit();

    QChar letter(array.at(array.size() - 1));
    if (letter.isDigit())
        return true;

    return false;
}

int TupLibraryWidget::getItemNameIndex(QString &name) const
{
    QByteArray array = name.toLocal8Bit();
    int index = 0;
    for (int i = array.size()-1; i >= 0; i--) {
         QChar letter(array.at(i));
         if (!letter.isDigit()) {
             index = i + 1;
             break;
         }
    }

    return index;
}

QString TupLibraryWidget::nameForClonedItem(QString &name, QString &extension, int index, QString &path) const
{
    QString symbolName = "";

    QString base = name.left(index); 
    int counter = name.right(index).toInt();

    while (true) {
           counter++;
           QString number = QString::number(counter);
           if (counter < 10)
               number = "0" + number; 
           symbolName = base + number + "." + extension.toLower();
           QString tester = path + symbolName;
           if (!QFile::exists(tester))
               break;
    }

    return symbolName;
}

QString TupLibraryWidget::nameForClonedItem(QString &smallId, QString &extension, QString &path) const
{
    QString symbolName = "";
    int index = 0;

    while(true) {
          QString number = QString::number(index);
          if (index < 10)
              number = "0" + number;

          QString base = smallId + number;
          symbolName = base + "." + extension.toLower();
          QString tester = path + symbolName;

          if (!QFile::exists(tester))
              break;

          index++;
    }

    return symbolName;
}

QString TupLibraryWidget::verifyNameAvailability(QString &name, QString &extension, bool isCloningAction) {

    int limit = 1;
    if (isCloningAction)
        limit = 0; 

    QList<QTreeWidgetItem *> list = libraryTree->findItems(name, Qt::MatchExactly, 1);
    if (list.size() > limit) {
        int total = 0;
        QTreeWidgetItem *node;
        for (int i=0; i<list.size(); i++) {
             node = list.at(i);
             if (node->text(2).compare(extension) == 0)
                 total++;
        }

        if (total > limit) {
            bool ok = false;
            if (itemNameEndsWithDigit(name)) {
                int index = getItemNameIndex(name);
                QString base = name.left(index);
                int counter = name.right(name.length() - index).toInt(&ok);
                if (ok) {
                    while (true) {
                           counter++;
                           QString number = QString::number(counter);
                           if (counter < 10)
                               number = "0" + number;
                           name = base + number;
                           QList<QTreeWidgetItem *> others = libraryTree->findItems(name, Qt::MatchExactly, 1);
                           if (others.size() == 0)
                               break;
                    }
                } else {
                    name = TAlgorithm::randomString(8);
                    #ifdef TUP_DEBUG
                        QString msg = "TupLibraryWidget::verifyNameAvailability() - Warning: error while processing item name!";
                        #ifdef Q_OS_WIN
                            qWarning() << msg;
                        #else
                            tWarning() << msg;
                        #endif
                    #endif
                }
            } else {
                int index = name.lastIndexOf("-");
                if (index < 0) {
                    name += "-1";
                } else {
                    QString first = name.mid(0, index);
                    QString last = name.mid(index+1, name.length() - index);
                    int newIndex = last.toInt(&ok);
                    if (ok) {
                        newIndex++;
                        name = first + "-" + QString::number(newIndex);
                    } else {
                        name = TAlgorithm::randomString(8);
                        #ifdef TUP_DEBUG
                            QString msg = "TupLibraryWidget::verifyNameAvailability() - Warning: error while processing item name!";
                            #ifdef Q_OS_WIN
                                qWarning() << msg;
                            #else
                                tWarning() << msg;
                            #endif
                        #endif
                    }
                }
            }
        }
    }

    return name;
}

void TupLibraryWidget::setDefaultPath(const QString &path)
{
    int last = path.lastIndexOf("/");
    QString dir = path.left(last);
    saveDefaultPath(dir);
}

void TupLibraryWidget::saveDefaultPath(const QString &dir)
{
    TCONFIG->beginGroup("General");
    TCONFIG->setValue("DefaultPath", dir);
    TCONFIG->sync();
}

void TupLibraryWidget::updateSoundTiming(int frame)
{
    if (currentSound) {
        currentSound->updateFrameToPlay(frame);
        library->updateEffectSoundList(currentSound->getDataPath(), frame);
        emit soundUpdated();
    }
}

void TupLibraryWidget::stopSoundPlayer()
{
    if (display)
        display->stopSoundPlayer();
}
