/*
 * Copyright (C) Kreogist Dev Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include <QFileDialog>

//Global Dependences.
#include "knglobal.h"
#include "knconfigure.h"
#include "knlocalemanager.h"
#include "knopacityanimebutton.h"

//Dependences.
#include "knmusicnowplayingbase.h"
#include "knmusicsolomenubase.h"

//Library SDK Ports.
#include "sdk/knmusiclibrarytab.h"
#include "sdk/knmusiccategorymodelbase.h"

//Library SDK Plugins.
#include "sdk/knmusiclibrarymodel.h"
#include "sdk/knmusiccategorymodel.h"
#include "sdk/knmusicalbummodel.h"
#include "sdk/knmusicgenremodel.h"
#include "sdk/knmusiclibrarysongtab.h"
#include "sdk/knmusiclibraryartisttab.h"
#include "sdk/knmusiclibraryalbumtab.h"
#include "sdk/knmusiclibrarygenretab.h"

#include "knmusicutil.h"

#include "knmusiclibrary.h"

using namespace MusicUtil;

KNMusicLibrary::KNMusicLibrary(QObject *parent) :
    KNMusicLibraryBase(parent),
    m_libraryPath(knMusicGlobal->musicLibraryPath()+"/Library"),
    m_addToLibraryButton(new KNOpacityAnimeButton()),
    m_libraryModel(new KNMusicLibraryModel(this)),
    m_songTab(new KNMusicLibrarySongTab),
    m_nowPlaying(nullptr),
    m_libraryConfigure(knMusicGlobal->configure()->getConfigure("MusicLibrary"))
{
    //Configure the library tabs.
    m_libraryModel->setDatabase(m_libraryPath+"/Music.db");
    m_libraryModel->setLibraryConfigure(m_libraryConfigure);
    m_libraryModel->setLibrarySystemConfigure(
                knMusicGlobal->systemConfigure()->getConfigure("MusicLibrary"));
    m_libraryModel->setLibraryPath(m_libraryPath);

    //Configure the tabs.
    // Song tab.
    m_songTab->setLibraryModel(m_libraryModel);
    linkLoadRequest(m_songTab);
    // Artist tab.
    m_categoryModel[TabArtists]=new KNMusicCategoryModel(this);
    m_categoryModel[TabArtists]->setCategoryColumn(Artist);
    m_libraryTabs[TabArtists]=new KNMusicLibraryArtistTab;
    // Album tab.
    m_categoryModel[TabAlbums]=new KNMusicAlbumModel(this);
    m_libraryTabs[TabAlbums]=new KNMusicLibraryAlbumTab;
    // Genre tab.
    m_categoryModel[TabGenres]=new KNMusicGenreModel(this);
    m_categoryModel[TabGenres]->setCategoryColumn(Genre);
    m_libraryTabs[TabGenres]=new KNMusicLibraryGenreTab;
    //Configure the album art tabs.
    KNMusicLibraryAlbumTab *albumTab=
            static_cast<KNMusicLibraryAlbumTab *>(m_libraryTabs[TabAlbums]);
    //Set the album art hash.
    albumTab->setImageManager(m_libraryModel->imageManager());

    //Generate the show in action list.
    QList<QAction *> showInActionList;
    //Add the actions to the list.
    showInActionList.append(m_songTab->showInAction());
    //Load the category tabs.
    for(int i=0; i<CategoryTabsCount; ++i)
    {
        //Install the category model to library model.
        m_libraryModel->installCategoryModel(m_categoryModel[i]);
        //Set the category model.
        m_libraryTabs[i]->setCategoryModel(m_categoryModel[i]);
        //Set the library model.
        m_libraryTabs[i]->setLibraryModel(m_libraryModel);
        //Link the load request.
        linkLoadRequest(m_libraryTabs[i]);
        //Add show in action to show in list.
        showInActionList.append(m_libraryTabs[i]->showInAction());
    }
    //Add the actions to solo menu.
    knMusicGlobal->soloMenu()->appendMusicActions(showInActionList);

    //Add to library status button.
    m_addToLibraryButton->setIcon(QIcon("://public/status_add_music.png"));
    connect(m_addToLibraryButton, &KNOpacityAnimeButton::clicked,
            this, &KNMusicLibrary::onAddToLibrary);
    knGlobal->addStatusWidget(m_addToLibraryButton);

    //Link the retranslate slot.
    knI18n->link(this, &KNMusicLibrary::retranslate);
    retranslate();
}

KNMusicTab *KNMusicLibrary::songTab()
{
    return m_songTab;
}

KNMusicTab *KNMusicLibrary::artistTab()
{
    return m_libraryTabs[TabArtists];
}

KNMusicTab *KNMusicLibrary::albumTab()
{
    return m_libraryTabs[TabAlbums];
}

KNMusicTab *KNMusicLibrary::genreTab()
{
    return m_libraryTabs[TabGenres];
}

bool KNMusicLibrary::isWorking()
{
    //The working state is the same as the music library working state.
    return m_libraryModel->isWorking();
}

KNMusicModel *KNMusicLibrary::musicModel(const QString &identifier)
{
    //Load the music library first.
    loadLibrary();
    //Check the identifier.
    return ("MusicModel/Library"==identifier)?m_libraryModel:nullptr;
}

KNMusicProxyModel *KNMusicLibrary::proxyMusicModel(const QString &identifier)
{
    //Check the identifier.
    if(!identifier.startsWith("ProxyModel/Library"))
    {
        //Failed to get the proxy model.
        return nullptr;
    }
    //Get the proxy model according to its identifier.
    QString tabName=identifier.mid(19);
    //Check the tab name.
    if("Song"==tabName)
    {
        //Song tab.
        return m_songTab->proxyMusicModel();
    }
    else if("Artist"==tabName)
    {
        //Artist tab.
        return m_libraryTabs[TabArtists]->proxyMusicModel();
    }
    else if("Album"==tabName)
    {
        //Album tab.
        return m_libraryTabs[TabAlbums]->proxyMusicModel();
    }
    else if("Genre"==tabName)
    {
        //Genre tab.
        return m_libraryTabs[TabGenres]->proxyMusicModel();
    }
    //For other case, return nullptr.
    return nullptr;
}

void KNMusicLibrary::showInSongTab()
{
    //Check out now playing pointer.
    if(m_nowPlaying)
    {
        //Show the playing song in the song tab.
        m_songTab->showInTab(m_nowPlaying->playingItem().detailInfo);
    }
}

void KNMusicLibrary::showInArtistTab()
{
    //Check out now playing pointer.
    if(m_nowPlaying)
    {
        //Show the playing song in the artist tab.
        m_libraryTabs[TabArtists]->showInTab(
                    m_nowPlaying->playingItem().detailInfo);
    }
}

void KNMusicLibrary::showInAlbumTab()
{
    //Check out now playing pointer.
    if(m_nowPlaying)
    {
        //Show the playing song in the artist tab.
        m_libraryTabs[TabAlbums]->showInTab(
                    m_nowPlaying->playingItem().detailInfo);
    }
}

void KNMusicLibrary::showInGenreTab()
{
    //Check out now playing pointer.
    if(m_nowPlaying)
    {
        //Show the playing song in the artist tab.
        m_libraryTabs[TabGenres]->showInTab(
                    m_nowPlaying->playingItem().detailInfo);
    }
}

void KNMusicLibrary::setNowPlaying(KNMusicNowPlayingBase *nowPlaying)
{
    //Save the now playing pointer.
    m_nowPlaying=nowPlaying;
}

void KNMusicLibrary::retranslate()
{
    //Translate the add to library button.
    m_addToLibraryButton->setToolTip(tr("Add music to Library"));
}

void KNMusicLibrary::loadLibrary()
{
    //Check the connection handler size.
    if(m_loadHandler.isEmpty())
    {
        //No need to load it again.
        return;
    }
    //Disconnect all links.
    m_loadHandler.disconnectAll();
    //Recover the library model.
    m_libraryModel->recoverModel();
}

void KNMusicLibrary::onAddToLibrary()
{
    //Generate the file dialog.
    QFileDialog fileDialog(knGlobal->mainWindow(),
                           tr("Add To Library"),
                           QString("."));
    //Configure the file dialog.
    fileDialog.setFileMode(QFileDialog::ExistingFiles);
    //Launch the file dialog.
    if(fileDialog.exec()==QDialog::Accepted &&
            !fileDialog.selectedUrls().isEmpty())
    {
        //Add all the selected urls to the library model.
        m_libraryModel->appendUrls(fileDialog.selectedUrls());
    }
}

void KNMusicLibrary::linkLoadRequest(KNMusicLibraryTab *libraryTab)
{
    //Link the library tab, add to load request handler.
    m_loadHandler.append(
                connect(libraryTab, &KNMusicLibraryTab::requireLoadLibrary,
                        this, &KNMusicLibrary::loadLibrary));
    //Simply link the show playlist list to require signal.
    connect(libraryTab, &KNMusicLibraryTab::requireShowPlaylistList,
            this, &KNMusicLibrary::requireShowPlaylistList);
    connect(libraryTab, &KNMusicLibraryTab::requireHidePlaylistList,
            this, &KNMusicLibrary::requireHidePlaylistList);
    connect(libraryTab, &KNMusicLibraryTab::requireAddToLibrary,
            this, &KNMusicLibrary::onAddToLibrary);
}
