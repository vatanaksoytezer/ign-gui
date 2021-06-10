/*
 * Copyright (C) 2021 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/

#ifdef _MSC_VER
#pragma warning(push, 0)
#endif
#include <ignition/msgs/twist.pb.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <string>

#include <ignition/gui/Application.hh>
#include <ignition/gui/MainWindow.hh>
#include <ignition/plugin/Register.hh>

#include "Teleop.hh"

namespace ignition
{
namespace gui
{
  enum class keyLinear{
    forward,
    backward,
    stop,
  };

  enum class keyAngular{
    left,
    right,
    stop,
  };

  class TeleopPrivate
  {
    /// \brief Node for communication.
    public: ignition::transport::Node node;

    /// \brief Topic. Set '/cmd_vel' as default.
    public: std::string topic = "/cmd_vel";

    /// \brief Publisher.
    ignition::transport::Node::Publisher cmdVelPub;

    /// \brief Linear velocity.
    public: float linearVel = 0;
    /// \brief Angular velocity.
    public: float angularVel = 0;

    /// \brief Linear direction.
    public: int linearDir = 0;
    /// \brief Angular direction.
    public: int angularDir = 0;

    /// \brief Linear state setted by keyboard input.
    public: keyLinear linearState = keyLinear::stop;
    /// \brief Angular state setted by keyboard input.
    public: keyAngular angularState = keyAngular::stop;

    public: bool keyEnable = false;
    public: bool newTopic = true;

  };
}
}

using namespace ignition;
using namespace gui;

/////////////////////////////////////////////////
Teleop::Teleop(): Plugin(), dataPtr(new TeleopPrivate)
{
}

/////////////////////////////////////////////////
Teleop::~Teleop()
{
}

/////////////////////////////////////////////////
void Teleop::LoadConfig(const tinyxml2::XMLElement *)
{
  if (this->title.empty())
    this->title = "Teleop";

  ignition::gui::App()->findChild
    <ignition::gui::MainWindow *>()->QuickWindow()->installEventFilter(this);
}

/////////////////////////////////////////////////
void Teleop::OnTeleopTwist()
{
  ignition::msgs::Twist cmdVelMsg;

  cmdVelMsg.mutable_linear()->set_x(
      this->dataPtr->linearDir * this->dataPtr->linearVel);
  cmdVelMsg.mutable_angular()->set_z(
      this->dataPtr->angularDir * this->dataPtr->angularVel);

  if (this->dataPtr->newTopic){
    this->dataPtr->cmdVelPub = ignition::transport::Node::Publisher();
    this->dataPtr->cmdVelPub =
    this->dataPtr->node.Advertise<ignition::msgs::Twist>(this->dataPtr->topic);
    this->dataPtr->newTopic = false;
  }

  this->dataPtr->cmdVelPub.Publish(cmdVelMsg);
}

/////////////////////////////////////////////////
void Teleop::OnTopicSelection(const QString& _topic)
{
  this->dataPtr->newTopic = true;
  this->dataPtr->topic = _topic.toStdString();
  ignmsg << "[OnTopicSelection]: topic: " << this->dataPtr->topic << std::endl;

}

/////////////////////////////////////////////////
void Teleop::OnLinearVelSelection(const QString& _velocity)
{
  this->dataPtr->linearVel = _velocity.toDouble();
  ignmsg << "[OnlinearVelSelection]: linear velocity: "
      << this->dataPtr->linearVel << std::endl;
}

/////////////////////////////////////////////////
void Teleop::OnAngularVelSelection(const QString& _velocity)
{
  this->dataPtr->angularVel = _velocity.toDouble();
  ignmsg << "[OnlinearVelSelection]: angular velocity: "
      << this->dataPtr->angularVel << std::endl;
}

/////////////////////////////////////////////////
void Teleop::OnKeySwitch(bool _checked)
{
  this->dataPtr->linearDir = 0;
  this->dataPtr->angularDir = 0;
  this->dataPtr->keyEnable = _checked;
}

/////////////////////////////////////////////////
void Teleop::OnSlidersSwitch(bool _checked)
{
  if(_checked){
    this->dataPtr->linearDir = 1;
    this->dataPtr->angularDir = 1;
    OnTeleopTwist();
  }
}

/////////////////////////////////////////////////
bool Teleop::eventFilter(QObject *_obj, QEvent *_event)
{
  if(_event->type() == QEvent::KeyPress && this->dataPtr->keyEnable == true)
  {
    QKeyEvent *keyEvent = static_cast<QKeyEvent*>(_event);
    switch(keyEvent->key())
    {
      case Qt::Key_W:
        this->dataPtr->linearState = keyLinear::forward;
        break;
      case Qt::Key_A:
        this->dataPtr->angularState = keyAngular::left;
        break;
      case Qt::Key_D:
        this->dataPtr->angularState = keyAngular::right;
        break;
      case Qt::Key_X:
        this->dataPtr->linearState = keyLinear::backward;
        break;
      default:
        ignmsg << "A non valid key was pressed" << std::endl;
        break;
    }
    setKeyDirection();
    OnTeleopTwist();
  }
  if(_event->type() == QEvent::KeyRelease && this->dataPtr->keyEnable == true)
  {
    QKeyEvent *keyEvent1 = static_cast<QKeyEvent*>(_event);
    switch(keyEvent1->key())
    {
      case Qt::Key_W:
        this->dataPtr->linearState = keyLinear::stop;
        break;
      case Qt::Key_A:
        this->dataPtr->angularState = keyAngular::stop;
        break;
      case Qt::Key_D:
        this->dataPtr->angularState = keyAngular::stop;
        break;
      case Qt::Key_X:
        this->dataPtr->linearState = keyLinear::stop;
        break;
      default:
        ignmsg << "A non valid key was pressed" << std::endl;
        break;
    }
    setKeyDirection();
    OnTeleopTwist();
  }
  return QObject::eventFilter(_obj, _event);
}

/////////////////////////////////////////////////
void Teleop::setKeyDirection()
{
  if(this->dataPtr->linearState == keyLinear::forward)
    this->dataPtr->linearDir = 1;
  else if(this->dataPtr->linearState == keyLinear::backward)
    this->dataPtr->linearDir = -1;
  else
    this->dataPtr->linearDir = 0;

  if(this->dataPtr->angularState == keyAngular::left)
    this->dataPtr->angularDir = 1;
  else if(this->dataPtr->angularState == keyAngular::right)
    this->dataPtr->angularDir = -1;
  else
    this->dataPtr->angularDir = 0;
}

/////////////////////////////////////////////////
int Teleop::LinearDirection() const
{
  return this->dataPtr->linearDir;
}

/////////////////////////////////////////////////
void Teleop::setLinearDirection(int _linearDir)
{
  this->dataPtr->linearDir = _linearDir;
  this->LinearDirectionChanged();
}

/////////////////////////////////////////////////
int Teleop::AngularDirection() const
{
  return this->dataPtr->angularDir;
}

/////////////////////////////////////////////////
void Teleop::setAngularDirection(int _angularDir)
{
  this->dataPtr->angularDir = _angularDir;
  this->AngularDirectionChanged();
}

// Register this plugin
IGNITION_ADD_PLUGIN(ignition::gui::Teleop,
                    ignition::gui::Plugin)
