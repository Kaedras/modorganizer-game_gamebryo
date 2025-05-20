#include "../gamebryogameplugins.h"

QString GamebryoGamePlugins::getLoadOrderPath() const
{
  return organizer()->profile()->absolutePath() + "/loadorder.txt";
}

QString GamebryoGamePlugins::getPluginsPath() const
{
  return organizer()->profile()->absolutePath() + "/plugins.txt";
}