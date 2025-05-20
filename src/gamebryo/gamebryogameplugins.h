#ifndef GAMEBRYOGAMEPLUGINS_H
#define GAMEBRYOGAMEPLUGINS_H

#include <QDateTime>
#include <QStringList>
#include <gameplugins.h>
#include <imoinfo.h>

class GamebryoGamePlugins : public MOBase::GamePlugins
{
public:
  GamebryoGamePlugins(MOBase::IOrganizer* organizer);

  virtual void writePluginLists(const MOBase::IPluginList* pluginList) override;
  virtual void readPluginLists(MOBase::IPluginList* pluginList) override;
  virtual QStringList getLoadOrder() override;

protected:
  MOBase::IOrganizer* organizer() const { return m_Organizer; }

  virtual void writePluginList(const MOBase::IPluginList* pluginList,
                               const QString& filePath);
  virtual void writeLoadOrderList(const MOBase::IPluginList* pluginList,
                                  const QString& filePath);
  virtual QStringList readLoadOrderList(MOBase::IPluginList* pluginList,
                                        const QString& filePath);
  virtual QStringList readPluginList(MOBase::IPluginList* pluginList);

  /**
   * @brief Returns the path of the plugins.txt file
   * @note The linux-specific implementation does a case-insensitive lookup for the file name.
   * This is required because Skyrim uses plugins.txt, while Oblivion and SkyrimSE use
   * Plugins.txt
   */
  QString getPluginsPath() const;
  /**
   * @brief Returns the path of the loadorder.txt file
   * @note The linux-specific implementation does a case-insensitive lookup for the file name.
   */
  QString getLoadOrderPath() const;

protected:
  MOBase::IOrganizer* m_Organizer;
  QDateTime m_LastRead;

private:
  void writeList(const MOBase::IPluginList* pluginList, const QString& filePath,
                 bool loadOrder);

private:
  std::map<QString, QByteArray> m_LastSaveHash;
};

#endif  // GAMEBRYOGAMEPLUGINS_H
