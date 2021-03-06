// License: GPL2
//
// FIFOを使ったプロセス間通信を行うクラス
//

#ifndef _IOMONITOR_H
#define _IOMONITOR_H

#include <gtkmm.h>
#ifdef _WIN32
#include "jdlib/jdthread.h"
#include <windows.h>
#endif

namespace CORE
{
    enum
    {
        FIFO_OK = 0,
        FIFO_OPEN_ERROR,
        FIFO_CREATE_ERROR
    };

    class IOMonitor
    {
#ifndef _WIN32
        // FIFOのファイルディスクリプタ
        int m_fifo_fd;
        // I/Oの架け橋
        Glib::RefPtr< Glib::IOChannel > m_iochannel;
#else
        HANDLE m_slot_hd;
        std::string m_slot_name;
        JDLIB::Thread m_thread;
#endif

        // FIFOファイル名
        std::string m_fifo_file;

        // FIFOの状態
        int m_fifo_stat;

        // メインプロセスか否か
        bool m_main_process;

      private:

        // 初期化
        void init();

        // FIFOを削除する
        void delete_fifo();

        // FIFOに書き込まれたら呼び出される
        bool slot_ioin( Glib::IOCondition io_condition );
#ifdef _WIN32
        static void* monitor_launcher( void* dat );
#endif

      public:

        IOMonitor();
        ~IOMonitor();

        // FIFOの状態を取得
        int get_fifo_stat(){ return m_fifo_stat; }

        // メインプロセスか否かを取得
        bool is_main_process(){ return m_main_process; }

        // FIFOに書き込み
        bool send_command( const char* command );
    };
}
#endif
