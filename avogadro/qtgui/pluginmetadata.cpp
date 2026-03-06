/******************************************************************************
  This source file is part of the Avogadro project.
  This source code is released under the 3-Clause BSD License, (see "LICENSE").
******************************************************************************/

#include "pluginmetadata.h"

#include <QtCore/QVariantMap>

namespace Avogadro {
namespace QtGui {

QString resolveLocalizableString(const QVariant& var, const QString& locale)
{
  // If not a VariantMap, var is presumably a plain string
  if (var.typeId() != QMetaType::QVariantMap)
    return var.toString();

  QVariantMap m = var.toMap();

  const QString localized = m.value(locale).toString();
  if (!localized.isEmpty()) {
    return localized;
  }
  return m.value(QStringLiteral("default")).toString();
}

} // namespace QtGui
} // namespace Avogadro
