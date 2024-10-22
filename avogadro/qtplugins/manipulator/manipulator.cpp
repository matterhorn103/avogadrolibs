/******************************************************************************
  This source file is part of the Avogadro project.
  This source code is released under the 3-Clause BSD License, (see "LICENSE").
******************************************************************************/

#include "manipulator.h"
#include "ui_manipulatewidget.h"

#include <avogadro/core/vector.h>

#include <avogadro/qtgui/molecule.h>
#include <avogadro/qtgui/rwmolecule.h>

#include <avogadro/qtopengl/glwidget.h>

#include <avogadro/rendering/camera.h>
#include <avogadro/rendering/glrenderer.h>

#include <QAction>
#include <QtGui/QIcon>
#include <QtGui/QKeyEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QWheelEvent>

using Avogadro::QtGui::Molecule;

namespace Avogadro::QtPlugins {

using QtGui::Molecule;
using QtGui::RWAtom;

#define ROTATION_SPEED 0.5

ManipulateWidget::ManipulateWidget(QWidget* parent)
  : QWidget(parent), m_ui(new Ui::ManipulateWidget)
{
  m_ui->setupUi(this);
}

ManipulateWidget::~ManipulateWidget()
{
  delete m_ui;
}

void ManipulateWidget::setAtomSelectionBox(size_t max)
{
  m_ui->atomSelectionBox->clear();
  for (size_t i = 0; i < max; ++i) {
    m_ui->atomSelectionBox->addItem(QString::number(i + 1));
  }
}

Manipulator::Manipulator(QObject* parent_)
  : QtGui::ToolPlugin(parent_), m_activateAction(new QAction(this)),
    m_molecule(nullptr), m_renderer(nullptr), m_pressedButtons(Qt::NoButton),
    m_toolWidget(new ManipulateWidget(dynamic_cast<QWidget*>(parent_))),
    m_currentAction(Nothing)
{
  m_activateAction->setText(tr("Manipulate"));
  m_activateAction->setToolTip(
    tr("Manipulation Tool\n\n"
       "Left Mouse: \tClick and drag to move atoms\n"
       "Right Mouse: \tClick and drag to rotate selected atoms.\n"));
  setIcon();
  connect(m_toolWidget->buttonBox, SIGNAL(clicked(QAbstractButton*)), this,
          SLOT(buttonClicked(QAbstractButton*)));
  m_toolWidget->setAtomSelectionBox(m_molecule->atomCount());
}

Manipulator::~Manipulator() {}

void Manipulator::setIcon(bool darkTheme)
{
  if (darkTheme)
    m_activateAction->setIcon(QIcon(":/icons/manipulator_dark.svg"));
  else
    m_activateAction->setIcon(QIcon(":/icons/manipulator_light.svg"));
}

QWidget* Manipulator::toolWidget() const
{
  return m_toolWidget;
}

void Manipulator::buttonClicked(QAbstractButton* button)
{
  if (m_toolWidget == nullptr)
    return;

  // clear focus from the boxes (they eat up keystrokes)
  m_toolWidget->xTranslateSpinBox->clearFocus();
  m_toolWidget->yTranslateSpinBox->clearFocus();
  m_toolWidget->zTranslateSpinBox->clearFocus();

  m_toolWidget->xRotateSpinBox->clearFocus();
  m_toolWidget->yRotateSpinBox->clearFocus();
  m_toolWidget->zRotateSpinBox->clearFocus();

  if (m_toolWidget->buttonBox->buttonRole(button) !=
      QDialogButtonBox::ApplyRole) {
    // reset values
    m_toolWidget->xTranslateSpinBox->setValue(0.0);
    m_toolWidget->yTranslateSpinBox->setValue(0.0);
    m_toolWidget->zTranslateSpinBox->setValue(0.0);

    m_toolWidget->xRotateSpinBox->setValue(0.0);
    m_toolWidget->yRotateSpinBox->setValue(0.0);
    m_toolWidget->zRotateSpinBox->setValue(0.0);

    return;
  }

  // apply values
  Vector3 delta(m_toolWidget->xTranslateSpinBox->value(),
                m_toolWidget->yTranslateSpinBox->value(),
                m_toolWidget->zTranslateSpinBox->value());

  translate(delta);

  Vector3 rotation(m_toolWidget->xRotateSpinBox->value(),
                   m_toolWidget->yRotateSpinBox->value(),
                   m_toolWidget->zRotateSpinBox->value());
  Vector3 center(0.0, 0.0, 0.0);

  // Check to see what we're rotating around (the origin, the center of the
  // selection, or a specific atom)
  switch m_toolWidget->centerSelectionBox->currentIndex() {
    case 0:
      // Described as origin in UI
      // Was previously not wrt actual origin but to center of all atoms for some reason?
      center = m_molecule->molecule().centerOfGeometry();
    
    case 1:
      // Center of selected atoms
      unsigned long selectedAtomCount = 0;
      for (Index i = 0; i < m_molecule->atomCount(); ++i) {
        if (!m_molecule->atomSelected(i))
          continue;

        center += m_molecule->atomPosition3d(i);
        selectedAtomCount++;
      }
      if (selectedAtomCount > 0)
        center /= selectedAtomCount;

    case 2:
      // Specific atom as center
      center_index = m_toolWidget->atomSelectionBox->currentIndex();
      center = m_molecule>atomPosition3d(center_index);
  }

  // Settings are in degrees
#ifndef DEG_TO_RAD
#define DEG_TO_RAD 0.0174532925
#endif
  rotate(rotation * DEG_TO_RAD, center);

  m_molecule->emitChanged(Molecule::Atoms | Molecule::Modified);
}

QUndoCommand* Manipulator::keyPressEvent(QKeyEvent* e)
{
  switch (e->key()) {
    case Qt::Key_Left:
    case Qt::Key_H:
    case Qt::Key_A:
      translate(Vector3(-0.1, 0.0, 0.0));
      e->accept();
      break;
    case Qt::Key_Right:
    case Qt::Key_L:
    case Qt::Key_D:
      translate(Vector3(+0.1, 0.0, 0.0));
      e->accept();
      break;
    case Qt::Key_Up:
    case Qt::Key_K:
    case Qt::Key_W:
      translate(Vector3(0.0, +0.1, 0.0));
      e->accept();
      break;
    case Qt::Key_Down:
    case Qt::Key_J:
    case Qt::Key_S:
      translate(Vector3(0.0, -0.1, 0.0));
      e->accept();
      break;
    default:
      e->ignore();
  }
  m_molecule->emitChanged(Molecule::Atoms | Molecule::Modified);
  return nullptr;
}

QUndoCommand* Manipulator::mousePressEvent(QMouseEvent* e)
{
  if (!m_renderer)
    return nullptr;

  updatePressedButtons(e, false);
  m_lastMousePosition = e->pos();
  Vector2f windowPos(e->localPos().x(), e->localPos().y());
  m_lastMouse3D = m_renderer->camera().unProject(windowPos);

  if (m_molecule) {
    m_molecule->setInteractive(true);
  }

  if (m_pressedButtons & Qt::LeftButton) {
    m_object = m_renderer->hit(e->pos().x(), e->pos().y());

    switch (m_object.type) {
      case Rendering::AtomType:
        e->accept();
        return nullptr;
      default:
        break;
    }
  }

  return nullptr;
}

QUndoCommand* Manipulator::mouseReleaseEvent(QMouseEvent* e)
{
  if (!m_renderer)
    return nullptr;

  updatePressedButtons(e, true);

  if (m_object.type == Rendering::InvalidType)
    return nullptr;

  if (m_molecule) {
    m_molecule->setInteractive(false);
  }

  switch (e->button()) {
    case Qt::LeftButton:
    case Qt::RightButton:
      resetObject();
      e->accept();
      break;
    default:
      break;
  }

  return nullptr;
}

QUndoCommand* Manipulator::mouseMoveEvent(QMouseEvent* e)
{
  updatePressedButtons(e, false);
  e->ignore();

  const Core::Molecule* mol = &m_molecule->molecule();
  Vector2f windowPos(e->localPos().x(), e->localPos().y());

  if (mol->isSelectionEmpty() && m_object.type == Rendering::AtomType &&
      m_object.molecule == &m_molecule->molecule()) {
    // translate single atom position
    RWAtom atom = m_molecule->atom(m_object.index);
    Vector3f oldPos(atom.position3d().cast<float>());
    Vector3f newPos = m_renderer->camera().unProject(windowPos, oldPos);
    atom.setPosition3d(newPos.cast<double>());
  } else if (!mol->isSelectionEmpty()) {
    // update all selected atoms
    Vector3f newPos = m_renderer->camera().unProject(windowPos);
    Vector3 delta = (newPos - m_lastMouse3D).cast<double>();

    if (m_currentAction == Translation) {
      translate(delta);
    } else {
      // get the center of the selected atoms
      Vector3 centroid(0.0, 0.0, 0.0);
      unsigned long selectedAtomCount = 0;
      for (Index i = 0; i < m_molecule->atomCount(); ++i) {
        if (!m_molecule->atomSelected(i))
          continue;

        centroid += m_molecule->atomPosition3d(i);
        selectedAtomCount++;
      }
      if (selectedAtomCount > 0)
        centroid /= selectedAtomCount;

      if (m_currentAction == Rotation) {
        rotate(delta, centroid);
      } else if (m_currentAction == ZoomTilt) {
        tilt(delta, centroid);
      }
    }

    // now that we've moved things, save the position
    m_lastMouse3D = newPos;
  }

  m_molecule->emitChanged(Molecule::Atoms | Molecule::Modified);
  e->accept();
  return nullptr;
}

void Manipulator::translate(Vector3 delta)
{
  for (Index i = 0; i < m_molecule->atomCount(); ++i) {
    if (!m_molecule->atomSelected(i))
      continue;

    Vector3 currentPos = m_molecule->atomPosition3d(i);
    m_molecule->setAtomPosition3d(i, currentPos + delta.cast<double>());
  }
}

void Manipulator::rotate(Vector3 delta, Vector3 centroid)
{
  // Rotate the selected atoms about the center
  // rotate only selected primitives
  Rendering::Camera* camera = &m_renderer->camera();
  Eigen::Vector3d backTransformX =
    camera->modelView().cast<double>().linear().row(0).transpose().normalized();
  Eigen::Vector3d backTransformY =
    camera->modelView().cast<double>().linear().row(1).transpose().normalized();

  Eigen::Projective3d fragmentRotation;
  fragmentRotation.matrix().setIdentity();
  fragmentRotation.translation() = centroid;
  fragmentRotation.rotate(
    Eigen::AngleAxisd(delta[1] * ROTATION_SPEED, backTransformX));
  fragmentRotation.rotate(
    Eigen::AngleAxisd(delta[0] * ROTATION_SPEED, backTransformY));
  fragmentRotation.translate(-centroid);

  for (Index i = 0; i < m_molecule->atomCount(); ++i) {
    if (!m_molecule->atomSelected(i))
      continue;

    Vector3 currentPos = m_molecule->atomPosition3d(i);
    m_molecule->setAtomPosition3d(
      i, (fragmentRotation * currentPos.homogeneous()).head<3>());
  }
}

void Manipulator::tilt(Vector3 delta, Vector3 centroid)
{
  // Rotate the selected atoms about the center
  // rotate only selected primitives
  Rendering::Camera* camera = &m_renderer->camera();
  Eigen::Vector3d backTransformZ =
    camera->modelView().cast<double>().linear().row(2).transpose().normalized();

  Eigen::Projective3d fragmentRotation;
  fragmentRotation.matrix().setIdentity();
  fragmentRotation.translation() = centroid;
  fragmentRotation.rotate(
    Eigen::AngleAxisd(delta[0] * ROTATION_SPEED, backTransformZ));
  fragmentRotation.translate(-centroid);

  for (Index i = 0; i < m_molecule->atomCount(); ++i) {
    if (!m_molecule->atomSelected(i))
      continue;

    Vector3 currentPos = m_molecule->atomPosition3d(i);
    m_molecule->setAtomPosition3d(
      i, (fragmentRotation * currentPos.homogeneous()).head<3>());
  }
}

void Manipulator::updatePressedButtons(QMouseEvent* e, bool release)
{
  if (release)
    m_pressedButtons &= e->buttons();
  else
    m_pressedButtons |= e->buttons();

  // check for modifier keys (e.g., Mac)
  if (e->buttons() & Qt::LeftButton && e->modifiers() == Qt::NoModifier) {
    m_currentAction = Translation;
  } else if (e->buttons() & Qt::MiddleButton ||
             (e->buttons() & Qt::LeftButton &&
              e->modifiers() == Qt::ShiftModifier)) {
    m_currentAction = ZoomTilt;
  } else if (e->buttons() & Qt::RightButton ||
             (e->buttons() & Qt::LeftButton &&
              (e->modifiers() == Qt::ControlModifier ||
               e->modifiers() == Qt::MetaModifier))) {
    m_currentAction = Rotation;
  }
}

} // namespace Avogadro::QtPlugins
