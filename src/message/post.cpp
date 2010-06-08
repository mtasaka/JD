// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "messageadmin.h"
#include "post.h"
#include "confirmdiag.h"

#include "skeleton/msgdiag.h"

#include "jdlib/loaderdata.h"
#include "jdlib/jdiconv.h"
#include "jdlib/miscmsg.h"
#include "jdlib/miscutil.h"
#include "jdlib/jdregex.h"

#include "dbtree/interface.h"

#include "config/globalconf.h"

#include "httpcode.h"

#include <cstring>


using namespace MESSAGE;

enum
{
    SIZE_OF_RAWDATA = 2 * 1024 * 1024
};

Post::Post( Gtk::Widget* parent, const std::string& url, const std::string& msg, bool new_article )
    : SKELETON::Loadable(),
      m_parent( parent ),
      m_url( url ),
      m_msg( msg ),
      m_rawdata( 0 ),
      m_lng_rawdata( 0 ),
      m_count( 0 ),
      m_subbbs( 0 ),
      m_new_article( new_article ),
      m_writingdiag( 0 )
{
#ifdef _DEBUG
    std::cout << "Post::Post " << m_url << std::endl;
#endif

    clear();
}



Post::~Post()
{
#ifdef _DEBUG
    std::cout << "Post::~Post " << m_url << std::endl;
#endif

    clear();

    if( m_writingdiag ) delete m_writingdiag;
    m_writingdiag = NULL;
}


void Post::clear()
{
#ifdef _DEBUG
    std::cout << "Post::clear\n";
#endif

    if( m_rawdata ) free( m_rawdata );
    m_rawdata = NULL;

    if( m_writingdiag ) m_writingdiag->hide();
}


void Post::emit_sigfin()
{
#ifdef _DEBUG
    std::cout << "Post::emit_sigfin\n";
#endif

    m_sig_fin.emit();

    clear();

    if( m_writingdiag ) delete m_writingdiag;
    m_writingdiag = NULL;
}


//
// 書き込み中ダイアログ表示
//
void Post::show_writingdiag( const bool show_buttons )
{
    Gtk::Window* toplevel = dynamic_cast< Gtk::Window* >( m_parent->get_toplevel() );
    if( ! toplevel ) return;

    Gtk::ButtonsType buttons = Gtk::BUTTONS_NONE;
    if( show_buttons ) buttons = Gtk::BUTTONS_OK;

    if( ! m_writingdiag ){
        m_writingdiag = new SKELETON::MsgDiag( *toplevel, "書き込み中・・・", false, Gtk::MESSAGE_INFO, buttons, false );
        m_writingdiag->signal_response().connect( sigc::mem_fun( *this, &Post::slot_response ) );
    }
    m_writingdiag->show();

    // gtkのバージョンによってはラベルが選択状態になっている場合があるので
    // 選択状態を解除する
    Gtk::Label *label = dynamic_cast< Gtk::Label* >( m_writingdiag->get_focus() );
    if( label ) label->set_selectable( false );
}


//
// 書き込み中ダイアログのボタンを押した
//
void Post::slot_response( int id )
{
#ifdef _DEBUG
    std::cout << "Post::slot_response id = " << id << std::endl;
#endif

    if( m_writingdiag ) m_writingdiag->hide();
}


//
// ポスト実行
//
void Post::post_msg()
{
    if( is_loading() ) return;

    clear();
    m_rawdata = ( char* )malloc( SIZE_OF_RAWDATA );
    m_lng_rawdata = 0;

    // 書き込み中ダイアログ表示
    if( ! CONFIG::get_hide_writing_dialog() ) show_writingdiag( false );

    JDLIB::LOADERDATA data;

    // 通常書き込み
    if( ! m_new_article ){
        if( ! m_subbbs ) data.url = DBTREE::url_bbscgi( m_url );  // 1回目の投稿先
        else data.url = DBTREE::url_subbbscgi( m_url ); // 2回目の投稿先
    }

    // 新スレ作成
    else{
        if( ! m_subbbs ) data.url = DBTREE::url_bbscgi_new( m_url );  // 1回目の投稿先
        else data.url = DBTREE::url_subbbscgi_new( m_url ); // 2回目の投稿先
    }

    // Content-Type (2009/02/18に報告された"したらば"に書き込めない問題で追加)
    // http://www.asahi-net.or.jp/~sd5a-ucd/rec-html401j/interact/forms.html#h-17.13.4.1
    // http://www.w3.org/TR/html401/interact/forms.html#h-17.13.4.1
    data.contenttype = "application/x-www-form-urlencoded";

    data.agent = DBTREE::get_agent( m_url );
    data.referer = DBTREE::get_write_referer( m_url );
    data.str_post = m_msg;
    data.host_proxy = DBTREE::get_proxy_host_w( m_url );
    data.port_proxy = DBTREE::get_proxy_port_w( m_url );
    data.basicauth_proxy = DBTREE::get_proxy_basicauth_w( m_url );
    data.size_buf = CONFIG::get_loader_bufsize();
    data.timeout = CONFIG::get_loader_timeout_post();
    data.cookie_for_write = DBTREE::board_cookie_for_write( m_url );
    data.basicauth = DBTREE::board_basicauth( m_url );

#ifdef _DEBUG
    std::cout << "Post::post_msg : " << std::endl
              << "url = " << data.url << std::endl
              << "contenttype = " << data.contenttype << std::endl
              << "agent = " << data.agent << std::endl
              << "referer = " << data.referer << std::endl
              << "cookie = " << data.cookie_for_write << std::endl
              << "proxy = " << data.host_proxy << ":" << data.port_proxy << std::endl
              << m_msg << std::endl;
#endif

    if( data.url.empty() ) return;

    start_load( data );
}



//
// ローダからデータを受け取る
//
void Post::receive_data( const char* data, size_t size )
{
    if( get_code() != HTTP_OK ) return;

    memcpy( m_rawdata + m_lng_rawdata , data, size );
    m_lng_rawdata += size;
    m_rawdata[ m_lng_rawdata ] = '\0';
}



//
// ローダがsendを終了したので戻り値解析
//
void Post::receive_finish()
{
#ifdef _DEBUG
    std::cout << "Post::receive_finish\n";
#endif

    std::string charset = DBTREE::board_charset( m_url );
    JDLIB::Iconv* libiconv = new JDLIB::Iconv( charset, "UTF-8" );
    int byte_out;
    const std::string str = libiconv->convert( m_rawdata, m_lng_rawdata, byte_out );
    delete libiconv;

#ifdef _DEBUG
    std::cout << "code = " << get_code() << std::endl;
    std::cout << str << std::endl;
#endif

    clear();

    ///////////////////
    
    // ポスト失敗
    if( get_code() != HTTP_OK
        && ! ( ( get_code() == HTTP_MOVED_PERM || get_code() == HTTP_REDIRECT ) && ! location().empty() ) // リダイレクトは成功(かもしれない)
        ){

        m_errmsg = get_str_code();
        emit_sigfin();
        return;
    }

    // 以下、code == 200、 又は 302 かつ locationがセットされている(リダイレクト) の場合

    JDLIB::Regex regex;

    std::string title;
    std::string tag_2ch;
    std::string msg;
    std::string conf;

    bool ret;

    // タイトル
    regex.exec( ".*<title>([^<]*)</title>.*", str, 0, true, false );
    title = MISC::remove_space( regex.str( 1 ) );

    // 2chタグ
    regex.exec( ".*2ch_X:([^\\-]*)\\-\\->.*", str, 0, false, false );
    tag_2ch = MISC::remove_space( regex.str( 1 ) );

    // エラー内容を取得

    // 一番内側の<b>〜</b>を探して取得
    m_errmsg = std::string();
    if( regex.exec( "([^>]|[^b]>)*<b>(([^>]|[^b]>)*)</b>.*", str, 0, true, false ) ){

        m_errmsg = regex.str( 2 );
    }

    // 2ch タグで error が返った場合
    else if( tag_2ch.find( "error" ) != std::string::npos ){

        if( regex.exec( "error +-->(.*)</body>", str, 0, true, false ) ) m_errmsg = regex.str( 1 );
    }

    // p2 型
    else if( title.find( "error" ) != std::string::npos ){

        if( regex.exec( "<h4>(.*)</h4>", str, 0, true, false ) ) m_errmsg = regex.str( 1 );
    }

    if( ! m_errmsg.empty() ){

        m_errmsg = MISC::replace_str( m_errmsg, "\n", "" ); 

        // <a 〜を取り除く
        while( regex.exec( "(.*)<a +href *= *\"([^\"]*)\" *>(.*)</a>(.*)", m_errmsg ) ){
            m_errmsg = regex.str( 1 ) + " " + regex.str( 2 ) + " " + regex.str( 3 ) + regex.str( 4 );
        }

        // 改行その他
        m_errmsg= MISC::replace_str( m_errmsg, "<br>", "\n" );
        m_errmsg= MISC::replace_str( m_errmsg, "<hr>", "\n-------------------\n" );

        // samba秒取得
        if( regex.exec( "ＥＲＲＯＲ +- +593 +([0-9]+) +sec", m_errmsg ) ){
            time_t sec = atoi( regex.str( 1 ).c_str() );
#ifdef _DEBUG
            std::cout << "samba = " << sec << std::endl;
#endif
            DBTREE::board_set_samba_sec( m_url, sec );
            DBTREE::board_update_writetime( m_url );
        }
    }

    // 書き込み確認
    regex.exec( ".*<font size=\\+1 color=#FF0000>([^<]*)</font>.*", str );
    conf = MISC::remove_space( regex.str( 1 ) );

    // メッセージ本文

    // 2ch 型
    ret = regex.exec( ".*</ul>.*<b>(.*)</b>.*<form.*", str, 0, false, false );

    // 0ch 型
    if( ! ret ) ret = regex.exec( ".*</ul>.*<b>(.*)</b>.*<input.*", str, 0, false, false );

    msg = MISC::remove_space( regex.str( 1 ) );
    std::list< std::string > list_cookies = SKELETON::Loadable::cookies();

#ifdef _DEBUG
    std::cout << "TITLE: [" << title << "]\n";
    std::cout << "2ch_X: [" << tag_2ch << "]\n";
    std::cout << "CONF: [" << conf << "]\n";
    std::cout << "MSG: [" << msg << "]\n";
    std::cout << "ERR: [" << m_errmsg << "]\n";

    std::list< std::string >::iterator it = list_cookies.begin();
    for( ; it != list_cookies.end(); ++it ) std::cout << "cookie : [" << (*it) << "]\n";

    std::cout << "location: [" << location() << "]\n";
#endif

    // 成功
    if( title.find( "書きこみました" ) != std::string::npos
        || tag_2ch.find( "true" ) != std::string::npos
        || ( ( get_code() == HTTP_MOVED_PERM || get_code() == HTTP_REDIRECT ) && ! location().empty() )  // リダイレクトされた場合
        ){

#ifdef _DEBUG
        std::cout << "write ok" << std::endl;
#endif        

        DBTREE::article_update_writetime( m_url );
        emit_sigfin();
        return;
    }

    // クッキー確認
    else if( m_count < 1 && // 永久ループ防止
        ( title.find( "書き込み確認" ) != std::string::npos
          || tag_2ch.find( "cookie" ) != std::string::npos
          || ( ! DBTREE::board_list_cookies_for_write( m_url ).size() && list_cookies.size() )
            ) ){

        clear();

        // 書き込み確認表示
        if( ! CONFIG::get_always_write_ok() ){

            std::string diagmsg = MISC::replace_str( msg, "<br>", "\n" );
            if( diagmsg.empty() ) diagmsg = "クッキーを有功にして書き込みますか？";

            ConfirmDiag mdiag( m_url, diagmsg );
            const int ret = mdiag.run();
            mdiag.hide();
            if( ret != Gtk::RESPONSE_OK ){

                set_code( HTTP_CANCEL );
                emit_sigfin();
                return;
            }
            if( mdiag.get_chkbutton().get_active() ) CONFIG::set_always_write_ok( true );
        }

        // 書き込み用キーワード( hana=mogera や suka=pontan など )をセット
        DBTREE::board_analyze_keyword_for_write( m_url, str );

        // 現在のメッセージにキーワードが付加されていない時は付け加える
        const std::string keyword = DBTREE::board_keyword_for_write( m_url );
        if( ! keyword.empty() && m_msg.find( keyword ) == std::string::npos ) m_msg += "&" + keyword;

        // クッキーのセット
        if( list_cookies.size() ){
            DBTREE::board_set_list_cookies_for_write( m_url, list_cookies );
        }

        ++m_count; // 永久ループ防止
        post_msg();

        return;
    }

    // スレ立て時の書き込み確認
    else if( m_count < 1 // 永久ループ防止
             && ! m_subbbs && conf.find( "書き込み確認" ) != std::string::npos ){

        // 書き込み用キーワード( hana=mogera や suka=pontan など )をセット
        DBTREE::board_analyze_keyword_for_write( m_url, str );

        // 現在のメッセージにキーワードが付加されていない時は付け加える
        const std::string keyword = DBTREE::board_keyword_for_write( m_url );
        if( ! keyword.empty() && m_msg.find( keyword ) == std::string::npos ) m_msg += "&" + keyword;

        // クッキーのセット
        if( ! SKELETON::Loadable::cookies().empty() )
            DBTREE::board_set_list_cookies_for_write( m_url, SKELETON::Loadable::cookies() );

        // subbbs.cgi にポスト先を変更してもう一回ポスト
        m_subbbs = true;
        ++m_count; // 永久ループ防止
        post_msg();
        return;
    }

    // その他エラー
    else{
        
#ifdef _DEBUG
        std::cout << "Error" << std::endl;
        std::cout << m_errmsg << std::endl;
#endif        

        MISC::ERRMSG( str );

        set_code( HTTP_ERR );
        emit_sigfin();
        return;
    }
}
