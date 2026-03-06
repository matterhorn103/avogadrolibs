/******************************************************************************
  This source file is part of the Avogadro project.
  This source code is released under the 3-Clause BSD License, (see "LICENSE").
******************************************************************************/

#ifndef AVOGADRO_QTGUI_PLUGINMETADATA_H
#define AVOGADRO_QTGUI_PLUGINMETADATA_H

#include "avogadroqtguiexport.h"

#include <QtCore/QString>
#include <QtCore/QVariant>

namespace Avogadro {
namespace QtGui {

/**
 * @brief Resolve an optionally-localised TOML value to a plain string.
 *
 * Plugin metadata may express certain human-visible strings either as a bare
 * string or as a localisation table, e.g.:
 *
 *   # bare string
 *   item = "Move All"
 *
 *   # localised table
 *   [item]
 *   default = "Move All"
 *   de = "Alle verschieben"
 *
 * This function takes a variant that may be either form, as well as the current
 * locale, and resolves the string to the correct localized form.
 * If @p var is a plain string it is always returned as-is.
 * When @p locale is "" or "default", the "default" key is used.
 *
 * @param var    The QVariant from the parsed metadata (string or map).
 * @param locale Locale to look up, e.g."fr" or "de_DE".
 * @return       The resolved string.
 */
AVOGADROQTGUI_EXPORT QString resolveLocalizableString(
  const QVariant& var,
  const QString& locale = QString());

} // namespace QtGui
} // namespace Avogadro

#endif // AVOGADRO_QTGUI_PLUGINMETADATA_H
