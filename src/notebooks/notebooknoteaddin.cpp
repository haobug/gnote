/*
 * gnote
 *
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



#include <glibmm/i18n.h>
#include <gtkmm/imagemenuitem.h>

#include "notebooks/notebooknoteaddin.hpp"
#include "notebooks/notebookmanager.hpp"
#include "debug.hpp"
#include "tag.hpp"
#include "tagmanager.hpp"
#include "notewindow.hpp"
#include "utils.hpp"
#include "sharp/foreach.hpp"

namespace gnote {
namespace notebooks {


  NotebookNoteAddin::NotebookNoteAddin()
    : m_toolButton(NULL)
    , m_menu(NULL)
  {
    m_notebookIcon = utils::get_icon("notebook", 22);
    m_newNotebookIcon =  utils::get_icon("notebook-new", 16);
  }


  NoteAddin * NotebookNoteAddin::create()
  {
    return new NotebookNoteAddin();
  }


  void NotebookNoteAddin::initialize ()
  {
  }

  void NotebookNoteAddin::initialize_tool_button()
  {
    m_toolButton = manage(new Gtk::MenuToolButton(*manage(new Gtk::Image(m_notebookIcon))));
    Gtk::Label * l = manage(new Gtk::Label());
    // Ellipsize names longer than 12 chars in length
    // TODO: Should we hardcode the ellipsized notebook name to 12 chars?
    l->set_max_width_chars(12);
    l->set_ellipsize(Pango::ELLIPSIZE_END);
    l->show_all ();
    m_toolButton->set_label_widget(*l);
    m_toolButton->set_homogeneous(false);
    m_toolButton->set_tooltip_text(_("Place this note into a notebook"));

    m_show_menu_cid = m_menu->signal_show()
      .connect(sigc::mem_fun(*this, &NotebookNoteAddin::on_menu_shown));
    m_toolButton->show_all();
    get_note()->get_window()->toolbar()->append(*m_toolButton);
    update_notebook_button_label();
    
    m_note_added_cid = NotebookManager::instance().signal_note_added_to_notebook()
      .connect(sigc::mem_fun(*this, &NotebookNoteAddin::on_note_added_to_notebook));
    m_note_removed_cid = NotebookManager::instance().signal_note_removed_from_notebook()
      .connect(sigc::mem_fun(*this, &NotebookNoteAddin::on_note_removed_from_notebook));
  }


  void NotebookNoteAddin::shutdown ()
  {
    if(m_toolButton) {
      m_show_menu_cid.disconnect();
      m_note_added_cid.disconnect();
      m_note_removed_cid.disconnect();
    }
  }

  void NotebookNoteAddin::on_note_opened ()
  {
    if(!m_menu) {
      m_menu = manage(new Gtk::Menu());
      m_menu->show_all();
    }
    if(!m_toolButton) {
      initialize_tool_button();
      m_toolButton->set_menu(*m_menu);
      // Disable the notebook button if this note is a template note
      Tag::Ptr templateTag = TagManager::instance().get_or_create_system_tag (TagManager::TEMPLATE_NOTE_SYSTEM_TAG);
      if (get_note()->contains_tag (templateTag)) {
        m_toolButton->set_sensitive(false);
				
        // Also prevent notebook templates from being deleted
        if (NotebookManager::instance().get_notebook_from_note (get_note())) {
          get_note()->get_window()->delete_button()->set_sensitive(false);
        }
      }
    }
  }

  void NotebookNoteAddin::on_menu_shown()
  {
    update_menu();
  }

  void NotebookNoteAddin::on_note_added_to_notebook(const Note & note, 
                                                    const Notebook::Ptr & notebook)
  {
    if(&note == get_note().get()) {
      update_notebook_button_label(notebook);
    }
  }


  void NotebookNoteAddin::on_note_removed_from_notebook(const Note & note, 
                                                      const Notebook::Ptr &)
  {
    if(&note == get_note().get()) {
      update_notebook_button_label();
    }
  }
 

  void NotebookNoteAddin::on_new_notebook_menu_item()
  {
    Note::List noteList;
    noteList.push_back(get_note());
    NotebookManager::instance().prompt_create_new_notebook(get_note()->get_window(),
                                                           noteList);
  }


  void NotebookNoteAddin::update_notebook_button_label()
  {
    Notebook::Ptr currentNotebook = NotebookManager::instance().get_notebook_from_note(get_note());
    update_notebook_button_label(currentNotebook);
  }


  void NotebookNoteAddin::update_notebook_button_label(const Notebook::Ptr & notebook)
  {
    std::string labelText = (notebook ? notebook->get_name() : _("Notebook"));
    
    Gtk::Label * l = dynamic_cast<Gtk::Label*>(m_toolButton->get_label_widget());
    if (l) {
      l->set_text(labelText);
    }
  }

  void NotebookNoteAddin::update_menu()
  {
    //
    // Clear out the old list
    //
    foreach (Gtk::MenuItem & oldItem, m_menu->items()) {
      m_menu->remove (oldItem);
    }

    //
    // Build a new menu
    //
			
    // Add the "New Notebook..."
    Gtk::ImageMenuItem *newNotebookMenuItem =
      manage(new Gtk::ImageMenuItem (_("_New notebook..."), true));
    newNotebookMenuItem->set_image(*manage(new Gtk::Image (m_newNotebookIcon)));
    newNotebookMenuItem->signal_activate()
      .connect(sigc::mem_fun(*this,&NotebookNoteAddin::on_new_notebook_menu_item));
    newNotebookMenuItem->show ();
    m_menu->append (*newNotebookMenuItem);
			
    // Add the "(no notebook)" item at the top of the list
    NotebookMenuItem *noNotebookMenuItem = manage(new NotebookMenuItem (m_radio_group,
                                                    get_note(), Notebook::Ptr()));
    noNotebookMenuItem->show_all ();
    m_menu->append (*noNotebookMenuItem);
			
    // Add in all the real notebooks
    std::list<NotebookMenuItem*> notebookMenuItems = get_notebook_menu_items ();
    if (!notebookMenuItems.empty()) {
      Gtk::SeparatorMenuItem *separator = manage(new Gtk::SeparatorMenuItem ());
      separator->show_all ();
      m_menu->append (*separator);
				
      foreach (NotebookMenuItem * item, notebookMenuItems) {
        item->show_all ();
        m_menu->append (*item);
      }
    }
  }
  

  std::list<NotebookMenuItem*> NotebookNoteAddin::get_notebook_menu_items()
  {
    std::list<NotebookMenuItem*>items;
			
    Glib::RefPtr<Gtk::TreeModel> model = NotebookManager::instance().get_notebooks();
    Gtk::TreeIter iter;
			
    iter = model->children().begin();
    foreach(const Gtk::TreeRow & row, model->children()) {
      Notebook::Ptr notebook;
      row.get_value(0, notebook);
      NotebookMenuItem *item = manage(new NotebookMenuItem (m_radio_group, 
                                                            get_note(), notebook));
      items.push_back (item);
    }
			
    items.sort ();
			
    return items;
  }


}
}