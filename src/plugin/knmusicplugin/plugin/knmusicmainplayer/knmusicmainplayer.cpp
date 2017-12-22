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
#include <QBoxLayout>
#include <QFileInfo>
#include <QLabel>

#include "kneditablelabel.h"
#include "knthememanager.h"
#include "knopacityanimebutton.h"
#include "knprogressslider.h"
#include "knglassanimebutton.h"
#include "knvolumeslider.h"
#include "knopacitybutton.h"
#include "kndpimanager.h"

#include "knmusiclyricsmanager.h"
#include "knmusicbackend.h"
#include "knmusicproxymodel.h"
#include "knmusicscrolllyrics.h"
#include "knmusicmainplayerpanel.h"
#include "knmusicnowplayingbase.h"
#include "knmusicnowplayinglistview.h"
#include "knmusicmainplayercontent.h"

#include "knmusicmainplayer.h"

#include <QDebug>

KNMusicMainPlayer::KNMusicMainPlayer(QWidget *parent) :
    KNMusicMainPlayerBase(parent),
    m_playIcon(QPixmap(":/plugin/music/player/play_dark.png")),
    m_pauseIcon(QPixmap(":/plugin/music/player/pause_dark.png")),
    m_buttonLeftLayout(nullptr),
    m_buttonRightLayout(nullptr),
    m_backend(nullptr),
    m_hideMainPlayer(new KNOpacityAnimeButton(this)),
    m_detailInfoPanel(new KNMusicMainPlayerPanel(this)),
    m_lyricsPanel(new KNMusicScrollLyrics(this)),
    m_playlistPanel(new KNMusicNowPlayingListView(this)),
    m_contentContainer(new KNMusicMainPlayerContent(this)),
    m_controlPanel(new QWidget(this)),
    m_progressSlider(new KNProgressSlider(this)),
    m_duration(new QLabel(this)),
    m_position(new KNEditableLabel(this)),
    m_loopMode(new KNOpacityAnimeButton(this)),
    m_volumeIcon(new KNOpacityButton(this)),
    m_volumeSlider(new KNVolumeSlider(this)),
    m_firstStageVolume(-1),
    m_secondStageVolume(-1),
    m_progressPressed(false)
{
    setObjectName("MainPlayer");
    //Set properties.
    setAutoFillBackground(true);

    //Initial the resources.
    m_loopStateIcon[NoRepeat]    =
            QPixmap(":/plugin/music/player/NoRepeat.png");
    m_loopStateIcon[RepeatTrack] =
            QPixmap(":/plugin/music/player/RepeatSingle.png");
    m_loopStateIcon[RepeatAll]   =
            QPixmap(":/plugin/music/player/Repeat.png");
    m_loopStateIcon[Shuffle]     =
            QPixmap(":/plugin/music/player/Random.png");
    m_volumeSizeIcon[NoVolume]=QPixmap(":/plugin/music/player/volume_0.png");
    m_volumeSizeIcon[Volume1] =QPixmap(":/plugin/music/player/volume_1.png");
    m_volumeSizeIcon[Volume2] =QPixmap(":/plugin/music/player/volume_2.png");
    m_volumeSizeIcon[Volume3] =QPixmap(":/plugin/music/player/volume_3.png");

    //Configure the hide main player button.
    m_hideMainPlayer->setFixedSize(knDpi->size(32, 32));
    m_hideMainPlayer->setIcon(
                QPixmap(":/plugin/music/player/hide_mainplayer.png"));
    connect(m_hideMainPlayer, &KNOpacityAnimeButton::clicked,
            this, &KNMusicMainPlayer::requireHide);
    //Configure the detail info panel.
    // Link the require show signals.
    connect(m_detailInfoPanel, &KNMusicMainPlayerPanel::requireShowInSongs,
            this, &KNMusicMainPlayer::requireShowInSongs);
    connect(m_detailInfoPanel, &KNMusicMainPlayerPanel::requireShowInArtists,
            this, &KNMusicMainPlayer::requireShowInArtists);
    connect(m_detailInfoPanel, &KNMusicMainPlayerPanel::requireShowInAlbums,
            this, &KNMusicMainPlayer::requireShowInAlbums);
    connect(m_detailInfoPanel, &KNMusicMainPlayerPanel::requireShowInGenres,
            this, &KNMusicMainPlayer::requireShowInGenres);
    // When it require show, it will automatically hide the songs.
    connect(m_detailInfoPanel, &KNMusicMainPlayerPanel::requireShowInSongs,
            this, &KNMusicMainPlayer::requireHide);
    connect(m_detailInfoPanel, &KNMusicMainPlayerPanel::requireShowInArtists,
            this, &KNMusicMainPlayer::requireHide);
    connect(m_detailInfoPanel, &KNMusicMainPlayerPanel::requireShowInAlbums,
            this, &KNMusicMainPlayer::requireHide);
    connect(m_detailInfoPanel, &KNMusicMainPlayerPanel::requireShowInGenres,
            this, &KNMusicMainPlayer::requireHide);
    //Configure the lyrics.
    m_lyricsPanel->setObjectName("MainPlayerLyrics");
    m_lyricsPanel->setBackend(knMusicGlobal->lyricsManager()->backend());
    m_lyricsPanel->setAlignment(Qt::AlignHCenter);
    knTheme->registerWidget(m_lyricsPanel);
    //Configure the playlist panel.
    m_playlistPanel->setSizePolicy(QSizePolicy::Minimum,
                                   QSizePolicy::Minimum);
    m_playlistPanel->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_playlistPanel->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    //Configure control panel.
    // Configure progress slider.
    m_progressSlider->setWheelStep(1000);
    m_progressSlider->setMaximum(0);
    connect(m_progressSlider, &KNProgressSlider::sliderPressed,
            [=]{m_progressPressed=true;});
    connect(m_progressSlider, &KNProgressSlider::sliderReleased,
            [=]{m_progressPressed=false;});
    connect(m_progressSlider, &KNProgressSlider::sliderMoved,
            this, &KNMusicMainPlayer::setPosition);
    connect(m_progressSlider, &KNProgressSlider::valueChanged,
            this, &KNMusicMainPlayer::updatePositionText);
    // Configure loop mode button.
    m_loopMode->setIcon(m_loopStateIcon[NoRepeat]);
    // Configure the volume indicator and slider.
    m_volumeIcon->setFixedSize(knDpi->size(32, 32));
    m_volumeIcon->setIcon(m_volumeSizeIcon[NoVolume]);
    m_volumeSlider->setMinimumWidth(knDpi->width(100));
    m_volumeSlider->setMaximumWidth(knDpi->width(150));
    // Initial and configure the buttons.
    for(int i=0; i<ControlButtonsCount; i++)
    {
        //Generate the button.
        m_controlButtons[i]=new KNGlassAnimeButton(this);
        //Resize the button, don't worry of this size, give any number is okay.
        m_controlButtons[i]->setFixedSize(knDpi->size(46, 46));
    }
    //Set the buttons icon.
    m_controlButtons[ButtonPrev]->setIcon(
                QPixmap(":/plugin/music/player/previous_dark.png"));
    m_controlButtons[ButtonPlayNPause]->setIcon(
                m_playIcon);
    m_controlButtons[ButtonNext]->setIcon(
                QPixmap(":/plugin/music/player/next_dark.png"));
    //Configure the button.
    m_controlButtons[0]->setShowLeftLine(true);
    // Configure time labels.
    m_duration->setObjectName("MainPlayerLabel");
    m_position->setObjectName("MainPlayerLabel");
    knTheme->registerWidget(m_duration);
    knTheme->registerWidget(m_position);
    QFont timeFont=m_duration->font();
    timeFont.setFamily("096MKSD");
    m_duration->setFont(timeFont);
    m_position->setFont(timeFont);
    //Configure position label.
    connect(m_position, &KNEditableLabel::contentChanged,
            this, &KNMusicMainPlayer::onActionPositionEdited);

    //Register the widget to the theme manager.
    knTheme->registerWidget(this);

    //Set the main player layout.
    QBoxLayout *mainLayout=new QBoxLayout(QBoxLayout::TopToBottom,
                                          this);
    mainLayout->setContentsMargins(0,0,0,0);
    QBoxLayout *headerLayout=new QBoxLayout(QBoxLayout::LeftToRight,
                                            mainLayout->widget());
    headerLayout->setContentsMargins(knDpi->margins(16,16,16,16));
    //Add widget to header layout.
    headerLayout->addWidget(m_hideMainPlayer);
    headerLayout->addStretch();
    mainLayout->addLayout(headerLayout);
    setLayout(mainLayout);
    //Initial the strectch space for content layout.
    mainLayout->addWidget(m_contentContainer, 1);
    //Add widget to content layout.
    m_contentContainer->setColumnWidget(0, m_detailInfoPanel);
    m_contentContainer->setColumnWidget(1, m_lyricsPanel);
    m_contentContainer->setColumnWidget(2, m_playlistPanel);
    //Reset the widget visible.
    //Use different column count to update the widget visible.
    m_contentContainer->setColumnCount(3);
    m_contentContainer->setColumnCount(1);
    //Link the content container to main player.
    connect(m_contentContainer, &KNMusicMainPlayerContent::columnCountChanged,
            this, &KNMusicMainPlayer::onColumnCountChanged);
    //Configure the control panel.
    //Generate the layout.
    QBoxLayout *controlLayout=new QBoxLayout(QBoxLayout::TopToBottom,
                                             m_controlPanel);
    controlLayout->setContentsMargins(0,0,0,0);
    controlLayout->setSpacing(0);
    m_controlPanel->setLayout(controlLayout);
    //Add widgets to control panels.
    controlLayout->addWidget(m_progressSlider);
    //Add to main playout.
    mainLayout->addWidget(m_controlPanel);
    //Generate the button layout.
    QBoxLayout *buttonLayout=new QBoxLayout(QBoxLayout::LeftToRight,
                                            controlLayout->widget());
    buttonLayout->setContentsMargins(knDpi->margins(5,0,5,0));
    buttonLayout->setSpacing(0);
    controlLayout->addLayout(buttonLayout, 1);
    //Add widgets to button layout.
    buttonLayout->addWidget(m_position, 0, Qt::AlignTop);
    //Generate the left part layout.
    m_buttonLeftLayout=new QBoxLayout(QBoxLayout::LeftToRight,
                                      controlLayout->widget());
    m_buttonLeftLayout->setSpacing(0);
    m_buttonLeftLayout->addStretch();
    //Add widget to left layout.
    m_buttonLeftLayout->addWidget(m_loopMode);
    buttonLayout->addLayout(m_buttonLeftLayout, 1);
    //Add widgets to left layout.
    //Add control buttons.
    for(int i=0; i<ControlButtonsCount; i++)
    {
        buttonLayout->addWidget(m_controlButtons[i], 0, Qt::AlignBottom);
    }
    //Generate the right part layout.
    m_buttonRightLayout=new QBoxLayout(QBoxLayout::LeftToRight,
                                       controlLayout->widget());
    m_buttonRightLayout->setSpacing(0);
    //Add widgets to right layout.
    QBoxLayout *volumeLayout=new QBoxLayout(QBoxLayout::LeftToRight,
                                            buttonLayout->widget());
    volumeLayout->setContentsMargins(0,0,0,0);
    volumeLayout->setSpacing(knDpi->width(2));
    volumeLayout->addWidget(m_volumeIcon);
    volumeLayout->addWidget(m_volumeSlider, 1);
    //Add volume layout to right layout.
    m_buttonRightLayout->addLayout(volumeLayout);
    m_buttonRightLayout->addStretch();
    buttonLayout->addLayout(m_buttonRightLayout, 1);
    buttonLayout->addWidget(m_duration, 0, Qt::AlignTop);
}

void KNMusicMainPlayer::setBackend(KNMusicBackend *backend)
{
    //Save the backend pointer.
    m_backend=backend;
    //Check whether the backend is null or not.
    if(m_backend==nullptr)
    {
        return;
    }
    //Link to the backend.
    connect(m_volumeSlider, &KNVolumeSlider::valueChanged,
            m_backend, &KNMusicBackend::setVolume);
    connect(m_controlButtons[ButtonPlayNPause], &KNGlassAnimeButton::clicked,
            m_backend, &KNMusicBackend::playNPause);
    //Connect the response.
    connect(m_backend, &KNMusicBackend::volumeChanged,
            this, &KNMusicMainPlayer::onVolumeChanged);
    connect(m_backend, &KNMusicBackend::durationChanged,
            this, &KNMusicMainPlayer::updateDuration);
    connect(m_backend, &KNMusicBackend::positionChanged,
            [=](const qint64 &position)
            {
                //Update the value of progress slider when the progress slider
                //is not being edited.
                if(!m_progressPressed)
                {
                    //Update the slider value.
                    m_progressSlider->setValue(position);
                }
            });
    connect(m_backend, &KNMusicBackend::playingStateChanged,
            [=](const int &state)
            {
                //If it's playing, then should display pause icon.
                m_controlButtons[ButtonPlayNPause]->setIcon(
                            (state==Playing)?m_pauseIcon:m_playIcon);
            });
    //Calculate the volume stage data.
    int volumeStage=(m_backend->maximumVolume()-m_backend->minimalVolume())/3;
    m_firstStageVolume=m_backend->minimalVolume()+volumeStage;
    m_secondStageVolume=m_firstStageVolume+volumeStage;
    //Sync data from the backend.
    m_volumeSlider->setRange(m_backend->minimalVolume(),
                             m_backend->maximumVolume());
    //Change the mouse step based on the range.
    int preferStep=(m_volumeSlider->maximum()-m_volumeSlider->minimal())/100;
    m_volumeSlider->setWheelStep(preferStep<1?1:preferStep);
    //Sync the volume data via called the slot.
    onVolumeChanged(m_backend->volume());
}

void KNMusicMainPlayer::setNowPlaying(KNMusicNowPlayingBase *nowPlaying)
{
    //Link the now playing with the widget.
    connect(nowPlaying, &KNMusicNowPlayingBase::nowPlayingChanged,
            this, &KNMusicMainPlayer::onAnalysisItemChanged);
    connect(nowPlaying, &KNMusicNowPlayingBase::nowPlayingReset,
            [=]
            {
                //Generate a empty analysis item.
                KNMusicAnalysisItem item;
                //Set the analysis item to be the empty one.
                onAnalysisItemChanged(item);
            });
    connect(nowPlaying, &KNMusicNowPlayingBase::loopStateChanged,
            this, &KNMusicMainPlayer::onLoopStateChanged);
    connect(nowPlaying, &KNMusicNowPlayingBase::nowPlayingModelChanged,
            this, &KNMusicMainPlayer::onPlayingModelChanged);
    //Link the button controls to the now playing
    connect(m_controlButtons[ButtonPrev], &KNGlassAnimeButton::clicked,
            nowPlaying, &KNMusicNowPlayingBase::playPrevious);
    connect(m_controlButtons[ButtonNext], &KNGlassAnimeButton::clicked,
            nowPlaying, &KNMusicNowPlayingBase::playNext);
    connect(m_loopMode, &KNOpacityAnimeButton::clicked,
            nowPlaying, &KNMusicNowPlayingBase::changeLoopState);
    //Sync the states from the now playing.
    onLoopStateChanged(nowPlaying->loopState());
}

void KNMusicMainPlayer::resizeEvent(QResizeEvent *event)
{
    //Do the resize.
    KNMusicMainPlayerBase::resizeEvent(event);

    //Calculate the font size.
    int fontSize=knDpi->height(25);
    //Resize the font.
    QFont textFont=font();
    textFont.setPixelSize(fontSize);
    //Set the new font.
    m_lyricsPanel->setFont(textFont);
    m_lyricsPanel->setSpacing(fontSize>>1);
    m_detailInfoPanel->updatePanelFont(textFont);
    //Check out the font size.
    textFont.setPixelSize(fontSize>>1);
    m_playlistPanel->setFont(textFont);
    //Resize the time font.
    textFont=m_position->font();
    textFont.setPixelSize(fontSize-knDpi->height(5));
    m_position->setFont(textFont);
    m_duration->setFont(textFont);
    //Calculate the button size.
    int buttonSize=(fontSize<<1);
    //Resize the control buttons.
    for(int i=0; i<ControlButtonsCount; ++i)
    {
        m_controlButtons[i]->setFixedSize(buttonSize, buttonSize);
    }
    //Resize the loop mode button.
    //Small button size.
    int smallButtonSize=knDpi->width(20);
    m_loopMode->setFixedSize(smallButtonSize, smallButtonSize);
    m_volumeIcon->setFixedSize(smallButtonSize, smallButtonSize);
    //Change the spacing of the layouts.
    int controlLayoutSpacing=buttonSize>>3;
    m_buttonLeftLayout->setContentsMargins(0,0,controlLayoutSpacing<<1,0);
    m_buttonLeftLayout->setSpacing(controlLayoutSpacing);
    m_buttonRightLayout->setContentsMargins(controlLayoutSpacing<<1,0,0,0);
    m_buttonRightLayout->setSpacing(controlLayoutSpacing);
}

void KNMusicMainPlayer::onAnalysisItemChanged(
        const KNMusicAnalysisItem &item)
{
    //Update the panel data.
    m_detailInfoPanel->setAnalysisItem(item);
}

void KNMusicMainPlayer::onVolumeChanged(int volumeSize)
{
    //Check out the backend first.
    if(!m_backend)
    {
        return;
    }
    //Check is the size is the minimum.
    if(volumeSize==m_backend->minimalVolume())
    {
        //When the backend is the minimum, it should be treated as no volume.
        m_volumeIcon->setIcon(m_volumeSizeIcon[NoVolume]);
    }
    //Check is the size in the first stage.
    else if(volumeSize<m_firstStageVolume)
    {
        //Change the volume icon to stage 1.
        m_volumeIcon->setIcon(m_volumeSizeIcon[Volume1]);
    }
    //Check is the size in the second stage.
    else if(volumeSize<m_secondStageVolume)
    {
        //Change the volume icon to stage 2.
        m_volumeIcon->setIcon(m_volumeSizeIcon[Volume2]);
    }
    else
    {
        //Then all the other volume is in the third stage.
        m_volumeIcon->setIcon(m_volumeSizeIcon[Volume3]);
    }
    //Check the volume slider is the same as the current volume.
    if(volumeSize!=m_volumeSlider->value())
    {
        //Block the value changed signal.
        m_volumeSlider->blockSignals(true);
        m_volumeSlider->setValue(volumeSize);
        m_volumeSlider->blockSignals(false);
    }
}

void KNMusicMainPlayer::onLoopStateChanged(int state)
{
    //Change the icon.
    m_loopMode->setIcon(m_loopStateIcon[state]);
}

void KNMusicMainPlayer::updatePositionText(const qint64 &position)
{
    //Check if the position label is being editing.
    if(!m_position->isEditing())
    {
        //Update the position data to new time position
        m_position->setText(KNMusicUtil::msecondToString(position));
    }
}

void KNMusicMainPlayer::updateDuration(const qint64 &duration)
{
    //Change the progress slider range.
    m_progressSlider->setMaximum(duration);
    //Set duration display text.
    m_duration->setText(KNMusicUtil::msecondToString(duration));
}

void KNMusicMainPlayer::onPlayingModelChanged(KNMusicProxyModel *proxyModel)
{
    //Set the new proxy model to the list view.
    m_playlistPanel->setModel(proxyModel);
}

void KNMusicMainPlayer::onColumnCountChanged(int columnCount)
{
    //Update the scroll alignment of the column count.
    m_lyricsPanel->setAlignment(columnCount==1?Qt::AlignHCenter:Qt::AlignLeft);
}

void KNMusicMainPlayer::onActionPositionEdited()
{
    //Get the latest text from the position.
    QString positionText=m_position->text();
    //Find the colon.
    int colonPosition=positionText.indexOf(':');
    //If we cannot find the colon, means it's not format as 'xx:xx'.
    if(-1==colonPosition)
    {
        //This might be a number, we treat it as second time.
        //Translate it to a number.
        bool translateSuccess=false;
        qint64 triedPositon=positionText.toLongLong(&translateSuccess);
        //If we succeed, set the position to that second.
        if(translateSuccess)
        {
            setPosition(triedPositon*1000);
        }
        return;
    }
    //Calculate the ms.
    qint64 minutePart=positionText.left(colonPosition).toInt(),
           secondPart=positionText.mid(colonPosition+1).toInt(),
           preferPosition=(minutePart*60+secondPart)*1000;
    if(preferPosition>0 && preferPosition<m_progressSlider->maximum())
    {
        setPosition(preferPosition);
    }
}

void KNMusicMainPlayer::setPosition(const qint64 &position)
{
    //Check the backend first.
    if(m_backend)
    {
        //Change the position.
        m_backend->setPosition(position);
    }
}
