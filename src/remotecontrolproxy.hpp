/*
 * gnote
 *
 * Copyright (C) 2011 Aurimas Cernius
 * Copyright (C) 2009 Hubert Figuiere
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef _REMOTECONTROL_PROXY_HPP_
#define _REMOTECONTROL_PROXY_HPP_

#include <giomm/dbusconnection.h>
#include <giomm/dbusintrospection.h>

namespace gnote {

class RemoteControl;
class RemoteControlClient;
class NoteManager;

class RemoteControlProxy 
{
public:
  static const char *GNOTE_SERVER_PATH;
  static const char *GNOTE_INTERFACE_NAME;
  static const char *GNOTE_SERVER_NAME;

  typedef sigc::slot<void, bool, bool> slot_name_acquire_finish;
  typedef sigc::slot<void> slot_connected;

  /** Get a dbus client */
  static Glib::RefPtr<RemoteControlClient> get_instance();
  static RemoteControl *get_remote_control();
  static void register_remote(NoteManager & manager, const slot_name_acquire_finish & on_finish);
  static void register_object(const Glib::RefPtr<Gio::DBus::Connection> & conn, NoteManager & manager,
                              const slot_name_acquire_finish & on_finish);
private:
  static void on_bus_acquired(const Glib::RefPtr<Gio::DBus::Connection> & conn, const Glib::ustring & name);
  static void on_name_acquired(const Glib::RefPtr<Gio::DBus::Connection> & conn, const Glib::ustring & name);
  static void on_name_lost(const Glib::RefPtr<Gio::DBus::Connection> & conn, const Glib::ustring & name);
  static void load_introspection_xml();

  static NoteManager * s_manager;
  static RemoteControl * s_remote_control;
  static bool s_bus_acquired;
  static Glib::RefPtr<Gio::DBus::Connection> s_connection;
  static Glib::RefPtr<Gio::DBus::InterfaceInfo> s_gnote_interface;
  static Glib::RefPtr<RemoteControlClient> s_remote_control_proxy;
  static slot_name_acquire_finish s_on_name_acquire_finish;
};

}

#endif
