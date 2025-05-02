#include "../gamegamebryo.h"
#include "stub.h"
#include "vdf_parser.h"

#include <QStandardPaths>
#include <QString>

using namespace Qt::StringLiterals;

QString GameGamebryo::identifyGamePath() const
{
  return parseSteamLocation(steamAPPId(), gameShortName());
}

QString GameGamebryo::getLootPath()
{
  STUB();
  return {};
}

QString GameGamebryo::localAppFolder()
{
  STUB();
  return {};
}

std::unique_ptr<BYTE[]> GameGamebryo::getRegValue(HKEY, LPCWSTR, LPCWSTR,
                                                  DWORD, LPDWORD)
{
  // this function is probably not needed
  return {};
}

QString GameGamebryo::findInRegistry(HKEY, LPCWSTR, LPCWSTR)
{
  // this function is probably not needed
  return {};
}

QString GameGamebryo::getKnownFolderPath(REFKNOWNFOLDERID folderId,
                                         bool)
{
  // use folderId as QStandardPaths::StandardLocation
  return QStandardPaths::standardLocations(
             static_cast<QStandardPaths::StandardLocation>(folderId))
      .first();
}

QString GameGamebryo::getSpecialPath(const QString&)
{
  // this function is probably not needed
  return {};
}

QString GameGamebryo::determineMyGamesPath(const QString& gameName)
{
  const QString pattern = "%1/My Games/"_L1 % gameName;

  auto tryDir = [&](const QString& dir) -> std::optional<QString> {
    if (dir.isEmpty()) {
      return {};
    }

    auto path = pattern.arg(dir);
    if (!QFileInfo::exists(path)) {
      return {};
    }

    return path;
  };

  // check inside user directory
  if (auto d =
          tryDir(QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation)
                     .first())) {
    return *d;
  }

  return {};
}
QString GameGamebryo::determineMyGamesPath(const QString& gameName,
                                           const QString& appID)
{
  const QString pattern = "%1/My Games/"_L1 % gameName;

  auto tryDir = [&](const QString& dir) -> std::optional<QString> {
    if (dir.isEmpty()) {
      return {};
    }

    auto path = pattern.arg(dir);
    if (!QFileInfo::exists(path)) {
      return {};
    }

    return path;
  };

  // check inside wine prefix
  QString steamLocation = parseSteamLocation(appID, gameName);

  QDir steamuserDocumentsDir = QDir(steamLocation);
  if (!steamuserDocumentsDir.cd(
          QStringLiteral("../../compatdata/%1/pfx/drive_c/users/steamuser/Documents")
              .arg(appID))) {
    return {};
              }

  if (auto d = tryDir(steamuserDocumentsDir.absolutePath())) {
    return *d;
  }

  // check inside user directory
  if (auto d =
          tryDir(QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation)
                     .first())) {
    return *d;
                     }
  return {};
}

QString GameGamebryo::parseEpicGamesLocation(const QStringList& manifests)
{
  STUB();
  return {};
}

QString GameGamebryo::parseSteamLocation(const QString& appid,
                                         const QString& directoryName)
{
  QString steamLibraryLocation;
  QString steamLibraries =
      QDir::homePath() % QStringLiteral("/.steam/steam/config/libraryfolders.vdf");
  if (QFile(steamLibraries).exists()) {
    std::ifstream file(steamLibraries.toStdString());
    auto root = tyti::vdf::read(file);
    for (const auto& child : root.childs) {
      tyti::vdf::object* library = child.second.get();
      auto apps                  = library->childs["apps"];
      if (apps->attribs.contains(appid.toStdString())) {
        steamLibraryLocation = QString::fromStdString(library->attribs["path"]);
        break;
      }
    }
  }
  if (!steamLibraryLocation.isEmpty()) {
    QString gameLocation =
        steamLibraryLocation % u"/steamapps/common/"_s % directoryName;
    if (QDir(gameLocation).exists())
      return gameLocation;
  }

  return {};
}