/*
Copyright (C) 2015 Sebastian Herbord. All rights reserved.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 3 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "gamebryolocalsavegames.h"
#include "registry.h"
#include <QSettings>
#include <QtDebug>
#include <iprofile.h>
#include <stddef.h>
#include <string>

#include "gamegamebryo.h"

using namespace Qt::Literals::StringLiterals;

GamebryoLocalSavegames::GamebryoLocalSavegames(const GameGamebryo* game,
                                               const QString& iniFileName)
    : m_Game{game}, m_IniFileName(iniFileName)
{}

MappingType GamebryoLocalSavegames::mappings(const QDir& profileSaveDir) const
{
  return {{profileSaveDir.absolutePath(), localSavesDirectory().absolutePath(), true,
           true}};
}

QString GamebryoLocalSavegames::localSavesDummy() const
{
  return "__MO_Saves/";
}

QDir GamebryoLocalSavegames::localSavesDirectory() const
{
  return QDir(m_Game->myGamesPath()).absoluteFilePath(localSavesDummy());
}

QDir GamebryoLocalSavegames::localGameDirectory() const
{
  return QDir(m_Game->myGamesPath()).absolutePath();
}

bool GamebryoLocalSavegames::prepareProfile(MOBase::IProfile* profile)
{
  bool enable = profile->localSavesEnabled();

  QString basePath        = profile->localSettingsEnabled()
                                ? profile->absolutePath()
                                : localGameDirectory().absolutePath();
  QString iniFilePath     = basePath + "/" + m_IniFileName;
  QString saveIniFilePath = profile->absolutePath() + "/" + "savepath.ini";

  QSettings ini(iniFilePath, QSettings::IniFormat);

  static const QString skipMe   = u"SKIP_ME"_s;
  static const QString deleteMe = u"DELETE_ME"_s;

  static const QString sLocalSavePath       = u"General/sLocalSavePath"_s;
  static const QString bUseMyGamesDirectory = u"General/bUseMyGamesDirectory"_s;

  // Get the current sLocalSavePath
  QString currentPath = ini.value(sLocalSavePath, skipMe).toString();

  bool alreadyEnabled = currentPath == localSavesDummy();

  // Get the current bUseMyGamesDirectory
  QString currentMyGames = ini.value(bUseMyGamesDirectory, skipMe).toString();
  // Create the __MO_Saves directory if local saves are enabled and it doesn't exist
  if (enable) {
    QDir saves = localSavesDirectory();
    if (!saves.exists()) {
      saves.mkdir(u"."_s);
    }
  }

  // Set the path to __MO_Saves if it's not already
  if (enable && !alreadyEnabled) {
    QSettings saveIni(saveIniFilePath, QSettings::IniFormat);

    // If the path is not blank, save it to savepath.ini
    if (currentPath != skipMe) {
      saveIni.setValue(sLocalSavePath, currentPath);
    }
    if (currentMyGames != skipMe) {
      saveIni.setValue(bUseMyGamesDirectory, currentMyGames);
    }

    ini.setValue(sLocalSavePath, localSavesDummy());
    ini.setValue(bUseMyGamesDirectory, u"1"_s);
  }

  // Get rid of the local saves setting if it's still there
  if (!enable && alreadyEnabled) {
    // If savepath.ini exists, use it and delete it
    if (QFile::exists(saveIniFilePath)) {
      QSettings saveIni(saveIniFilePath, QSettings::IniFormat);
      QString savedPath    = saveIni.value(sLocalSavePath, deleteMe).toString();
      QString savedMyGames = saveIni.value(bUseMyGamesDirectory, deleteMe).toString();

      if (savedPath != deleteMe) {
        ini.setValue(sLocalSavePath, savedPath);
      } else {
        ini.remove(sLocalSavePath);
      }
      if (savedMyGames != deleteMe) {
        ini.setValue(bUseMyGamesDirectory, savedMyGames);
      } else {
        ini.remove(bUseMyGamesDirectory);
      }
      QFile::remove(saveIniFilePath);
    }
    // Otherwise just delete the setting
    else {
      ini.remove(sLocalSavePath);
      ini.remove(bUseMyGamesDirectory);
    }
  }

  return enable != alreadyEnabled;
}
