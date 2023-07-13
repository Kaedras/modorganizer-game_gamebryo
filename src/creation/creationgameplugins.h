#ifndef CREATIONGAMEPLUGINS_H
#define CREATIONGAMEPLUGINS_H

#include <gamebryogameplugins.h>
#include <imoinfo.h>
#include <iplugingame.h>
#include <map>

class CreationGamePlugins : public GamebryoGamePlugins
{
public:
  CreationGamePlugins(MOBase::IOrganizer* organizer);

protected:
  virtual void writePluginList(const MOBase::IPluginList* pluginList,
                               const QString& filePath) override;
  virtual QStringList readPluginList(MOBase::IPluginList* pluginList) override;
  virtual QStringList getLoadOrder() override;
  virtual bool lightPluginsAreSupported() override;

private:
  std::map<QString, QByteArray> m_LastSaveHash;
};

#endif  // CREATIONGAMEPLUGINS_H
