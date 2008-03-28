// ライセンス: GPL2

//
// 履歴サブメニュー
//

#ifndef _HISTORYSUBMENU_H
#define _HISTORYSUBMENU_H

#include <gtkmm.h>
#include <string>

namespace HISTORY
{
    struct HIST_ITEM
    {
        std::string url;
        std::string name;
        int type;
    };

    class HistorySubMenu : public Gtk::Menu
    {
        std::string m_path_load_xml;
        std::string m_path_save_xml;
        std::list< Gtk::MenuItem* > m_itemlist;
        std::list< HIST_ITEM* > m_histlist;

        // ポップアップメニュー
        Gtk::Menu m_popupmenu;
        int m_number_menuitem;

      public:

        HistorySubMenu( const std::string& path_load_xml, const std::string& path_save_xml );
        ~HistorySubMenu();

        void clear();
        void append_item( const std::string& url, const std::string& name, int type );
        void update();

        // ラベルをセット
        void set_menulabel();

      private:

        // 上から num 版目の HIST_ITEM 取得
        HIST_ITEM* get_item( int num );

        void xml2list( const std::string& xml );
        std::string list2xml();

        void open_history( int i );

        // メニューアイテムがactiveになった
        void slot_active( const int i );
        bool slot_button_press( GdkEventButton* event, int i );

        // ポップアップメニューのslot
        void slot_open_history();
        void slot_remove_history();
        void slot_show_property();
    };
}

#endif