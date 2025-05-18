#include "gamegamebryo.h"

#include "bsainvalidation.h"
#include "dataarchives.h"
#include "gamebryomoddatacontent.h"
#include "gamebryosavegame.h"
#include "gameplugins.h"
#include "iprofile.h"
#include "log.h"
#include "registry.h"
#include "savegameinfo.h"
#include "scopeguard.h"
#include "scriptextender.h"
#include "utility.h"
#include "vdf_parser.h"

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QIcon>
#include <QJsonDocument>
#include <QSettings>
#include <QStandardPaths>
#include <QtDebug>
#include <QtGlobal>

#include <optional>
#include <string>
#include <vector>

using namespace Qt::Literals::StringLiterals;

GameGamebryo::GameGamebryo() {}

void GameGamebryo::detectGame()
{
  m_GamePath    = identifyGamePath();
  m_MyGamesPath = determineMyGamesPath(gameName());
}

bool GameGamebryo::init(MOBase::IOrganizer* moInfo)
{
  m_Organizer = moInfo;
  m_Organizer->onAboutToRun([this](const auto& binary) {
    return prepareIni(binary);
  });
  return true;
}

bool GameGamebryo::isInstalled() const
{
  return !m_GamePath.isEmpty();
}

QIcon GameGamebryo::gameIcon() const
{
  return MOBase::iconForExecutable(gameDirectory().absoluteFilePath(binaryName()));
}

QDir GameGamebryo::gameDirectory() const
{
  return QDir(m_GamePath);
}

QDir GameGamebryo::dataDirectory() const
{
  return gameDirectory().absoluteFilePath("Data");
}

void GameGamebryo::setGamePath(const QString& path)
{
  m_GamePath = path;
}

QDir GameGamebryo::documentsDirectory() const
{
  return m_MyGamesPath;
}

QDir GameGamebryo::savesDirectory() const
{
  return QDir(myGamesPath() + "/Saves");
}

std::vector<std::shared_ptr<const MOBase::ISaveGame>>
GameGamebryo::listSaves(QDir folder) const
{
  QStringList filters;
  filters << QString("*.") + savegameExtension();

  std::vector<std::shared_ptr<const MOBase::ISaveGame>> saves;
  for (auto info : folder.entryInfoList(filters, QDir::Files)) {
    try {
      saves.push_back(makeSaveGame(info.filePath()));
    } catch (std::exception& e) {
      MOBase::log::error("{}", e.what());
      continue;
    }
  }

  return saves;
}

void GameGamebryo::setGameVariant(const QString& variant)
{
  m_GameVariant = variant;
}

QString GameGamebryo::binaryName() const
{
  return gameShortName() + ".exe";
}

MOBase::IPluginGame::LoadOrderMechanism GameGamebryo::loadOrderMechanism() const
{
  return LoadOrderMechanism::FileTime;
}

MOBase::IPluginGame::SortMechanism GameGamebryo::sortMechanism() const
{
  return SortMechanism::LOOT;
}

bool GameGamebryo::looksValid(QDir const& path) const
{
  // Check for <prog>.exe for now.
  return path.exists(binaryName());
}

QString GameGamebryo::gameVersion() const
{
  // We try the file version, but if it looks invalid (starts with the fallback
  // version), we look the product version instead. If the product version is
  // not empty, we use it.
  QString binaryAbsPath = gameDirectory().absoluteFilePath(binaryName());
  QString version       = MOBase::getFileVersion(binaryAbsPath);
  if (version.startsWith(FALLBACK_GAME_VERSION)) {
    QString pversion = MOBase::getProductVersion(binaryAbsPath);
    if (!pversion.isEmpty()) {
      version = pversion;
    }
  }
  return version;
}

QString GameGamebryo::getLauncherName() const
{
  return gameShortName() + "Launcher.exe";
}

uint16_t GameGamebryo::getArch(QString const& program) const
{
  // brief summary of this function:
  // seek to 0x3C
  // the next 4 bytes contain either "PE\0\0" or the offset of the aforementioned value,
  // in which case seek to the offset, read the 2 bytes after "PE\0\0" and return them
  // as uint16_t

  static constexpr uint16_t offset = 0x3C;
  static constexpr char pe[4]      = {'P', 'E', 0, 0};

  QString realPath;
#ifdef __unix__
  realPath = program;
#else
  // todo: check if this is required
  realPath = "\\\\?\\" +
             QDir::toNativeSeparators(this->gameDirectory().absoluteFilePath(program));
#endif

  QFile file(realPath);
  if (!file.open(QIODevice::ReadOnly)) {
    return 0;
  }

  // this location contains either "PE\0\0" or the offset the value can be found
  if (!file.seek(offset)) {
    return 0;
  }
  uint32_t peValue;
  if (file.read(reinterpret_cast<char*>(&peValue), sizeof(uint32_t)) !=
      sizeof(uint32_t)) {
    return 0;
  }

  if (memcmp(&peValue, pe, sizeof(uint32_t)) != 0) {
    // read value is offset, seek to real value
    // convert bytes to uint64
    if (!file.seek(peValue)) {
      return 0;
    }
    if (file.read(reinterpret_cast<char*>(&peValue), sizeof(uint32_t)) !=
        sizeof(uint32_t)) {
      return 0;
    }
    if (memcmp(&peValue, pe, sizeof(uint32_t)) != 0) {
      return 0;
    }
  }

  uint16_t value;
  if (file.read(reinterpret_cast<char*>(&value), sizeof(uint16_t)) !=
      sizeof(uint16_t)) {
    return 0;
  }

  return value;
}

QFileInfo GameGamebryo::findInGameFolder(const QString& relativePath) const
{
  return QFileInfo(m_GamePath + "/" + relativePath);
}

bool GameGamebryo::prepareIni(const QString& exec)
{
  MOBase::IProfile* profile = m_Organizer->profile();

  QString basePath = profile->localSettingsEnabled()
                         ? profile->absolutePath()
                         : documentsDirectory().absolutePath();

  if (!iniFiles().isEmpty()) {

    QString profileIni = basePath + "/" + iniFiles()[0];

    QSettings ini(profileIni, QSettings::IniFormat);

    if (ini.value(u"Launcher/bEnableFileSelection"_s, "0").toString() != "1") {
      ini.setValue(u"Launcher/bEnableFileSelection"_s, "1");
    }
  }

  return true;
}

QString GameGamebryo::selectedVariant() const
{
  return m_GameVariant;
}

QString GameGamebryo::myGamesPath() const
{
  return m_MyGamesPath;
}

QString GameGamebryo::localAppFolder()
{
  // return values:
  // linux: ~/.config
  // os x: ~/Library/Preferences
  // windows: C:/Users/<USER>/AppData/Local
  return QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).first();
}

void GameGamebryo::copyToProfile(QString const& sourcePath,
                                 QDir const& destinationDirectory,
                                 QString const& sourceFileName)
{
  copyToProfile(sourcePath, destinationDirectory, sourceFileName, sourceFileName);
}

void GameGamebryo::copyToProfile(QString const& sourcePath,
                                 QDir const& destinationDirectory,
                                 QString const& sourceFileName,
                                 QString const& destinationFileName)
{
  QString filePath = destinationDirectory.absoluteFilePath(destinationFileName);
  if (!QFileInfo(filePath).exists()) {
    if (!MOBase::shellCopy(sourcePath + "/" + sourceFileName, filePath)) {
      // if copy file fails, create the file empty
      QFile(filePath).open(QIODevice::WriteOnly);
    }
  }
}

MappingType GameGamebryo::mappings() const
{
  MappingType result;

  for (const QString& profileFile : {"plugins.txt", "loadorder.txt"}) {
    result.push_back({m_Organizer->profilePath() + "/" + profileFile,
                      localAppFolder() + "/" + gameShortName() + "/" + profileFile,
                      false});
  }

  return result;
}

void GameGamebryo::registerFeature(std::shared_ptr<MOBase::GameFeature> feature)
{
  // priority does not matter, this is a game plugin so will get lowest priority in MO2
  m_Organizer->gameFeatures()->registerFeature(this, feature, 0, true);
}
