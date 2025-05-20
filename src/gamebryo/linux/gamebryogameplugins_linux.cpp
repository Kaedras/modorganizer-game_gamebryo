#include "../gamebryogameplugins.h"

/**
 * Finds a file in the specified path, matching its name case-insensitively, and
 * returns its absolute file path if found. If no matching file is found, the original
 * path is returned.
 * @param path The path to the file being searched, including the
 * file name.
 * @return The absolute file path of the matching file if found, or the
 * original  path if no matching file exists.
 */
QString findFileCaseInsensitive(const QString& path)
{
  QFileInfo info(path);
  QString fileName = info.fileName();
  QDir parentDir(info.dir());

  for (const QDirListing::DirEntry& dirEntry :
       QDirListing(parentDir.absolutePath(), QDirListing::IteratorFlag::FilesOnly)) {
    if (dirEntry.fileName().compare(fileName, Qt::CaseInsensitive) == 0) {
      return dirEntry.absoluteFilePath();
    }
  }

  return path;
}

QString GamebryoGamePlugins::getLoadOrderPath() const
{
  return findFileCaseInsensitive(organizer()->profile()->absolutePath() +
                                 "/loadorder.txt");
}

QString GamebryoGamePlugins::getPluginsPath() const
{
  return findFileCaseInsensitive(organizer()->profile()->absolutePath() +
                                 "/plugins.txt");
}