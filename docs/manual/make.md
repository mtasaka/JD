---
title: make、実行方法について
layout: default
---

&gt; [Top](../) &gt; {{ page.title }}

## {{ page.title }}

- [動作環境](#environment)
- [makeに必要なツール、ライブラリ](#requirement)
- [make 方法( rpmbuild の場合 )](#make-rpmbuild)
- [make 方法( configure + make の場合 )](#make-configure)
- [configureオプション](#configure-option)
- [メモ](#memo)


<a name="environment"></a>
### 動作環境

#### 必須環境
- gtkmm-2.24.0 以上 ( GTK2版 )
- gtkmm-3.0.0 以上 ( GTK3版 )
- zlib-1.2 以上
- gnutls-3.3.8 以上

#### 推奨環境
- Linux Kernel 3.10 以上
- gtkmm-2.24 以上
- gtkmm-3.18 以上 ( GTK3版 )
- UTF-8環境 ( EUC環境では `LANG="ja_JP.UTF-8"` を指定する必要がある )


<a name="requirement"></a>
### makeに必要なツール、ライブラリ

#### 必須
- autoconf
- autoconf-archive
- automake
- g++ 5 以上、または clang++ 3.3 以上
- gnutls
- gtkmm
- libtool
- make
- zlib

#### オプション
- alsa-lib (`--with-alsa`)
- openssl (`--with-tls=openssl`)
- oniguruma (`--with-regex=oniguruma`)
- libpcre (`--with-regex=pcre`)
- migemo (`--with-migemo`)

OSやディストリビューション別の解説は[OS/ディストリビューション別インストール方法][wiki-install] (JD wiki) を参照。


<a name="make-rpmbuild"></a>
### make 方法( rpmbuild の場合 )
1. `rpmbuild -tb 〜.tgz` でrpmファイルが出来るのであとは `rpm -Uvh 〜.rpm`
2. ライブラリが足りないといわれたら `yum install 〜-devel`
3. 起動はメニューから起動するか、端末で `jdim` と打ち込んでエンターを押す。


<a name="make-configure"></a>
### make 方法( configure + make の場合 )
デフォルトの設定では**GTK3版をビルド**する。(バージョン0.3.0以降)

1. `autoreconf -i` ( 又は `./autogen.sh` )
2. `./configure`
3. `make`
4. (お好みで) `strip src/jdim`


<a name="configure-option"></a>
### configureオプション
<dl>
  <dt>--with-sessionlib=xsmp|no</dt>
  <dd>
    XSMP を使ってセッション管理をするには <code>xsmp</code> を、
    セッション管理を無効にするには <code>no</code> を選択する。デフォルトでは XSMP を使用する。
  </dd>
  <dt>--with-pangolayout</dt>
  <dd>描画に PangoLayout を使う。デフォルトでは PangoGlyphString を使用する。</dd>
  <dt>--with-migemo</dt>
  <dd>migemo による検索が有効になる。migemoがUTF-8の辞書でインストールされている必要がある。</dd>
  <dt>--with-native</dt>
  <dd>CPUに合わせた最適化。CPUを指定する場合は <code>./configure CXXFLAGS="-march=ARCH"</code> を利用する。</dd>

  <dt>--with-tls=[gnutls|openssl]</dt>
  <dd>使用するSSL/TLSライブラリを設定する。デフォルトでは GnuTLS を使用する。</dd>
  <dt>--with-tls=openssl</dt>
  <dd>GnuTLS のかわりに OpenSSL を使用する。ライセンス上バイナリ配布が出来なくなることに注意すること。</dd>
  <dt>--with-openssl</dt>
  <dd><strong>非推奨</strong>: かわりに <code>--with-tls=openssl</code> を使用してください。</dd>

  <dt>--with-alsa</dt>
  <dd>ALSA による効果音再生機能を有効にする。
  詳しくは<a href="{{ site.baseurl }}/sound/">効果音の再生</a>の項を参照すること。</dd>
  <dt>--with-xdgopen</dt>
  <dd>デフォルトブラウザとしてxdg-openを使用する。</dd>
  <dt>--enable-gprof</dt>
  <dd>
    gprof によるプロファイリングを行う。
    コンパイルオプションに <code>-pg</code> が付き、JDimを実行すると <code>gmon.out</code> が出来るので
    <code>gprof  ./jdim  gmon.out</code> で解析できる。CPUの最適化は効かなくなるので注意する。
  </dd>

  <dt>--with-regex=[posix|oniguruma|pcre]</dt>
  <dd>使用する正規表現ライブラリを設定する。デフォルトでは POSIX regex を使用する。</dd>
  <dt>--with-regex=oniguruma</dt>
  <dd>
    POSIX regex のかわりに鬼車を使用する。
    鬼車はBSDライセンスなのでJDimをバイナリ配布する場合には注意すること(ライセンスはGPLになる)。
  </dd>
  <dt>--with-regex=pcre</dt>
  <dd>
    POSIX regex のかわりに PCRE を使用する。
    PCREはBSDライセンスなのでJDimをバイナリ配布する場合には注意すること(ライセンスはGPLになる)。
    UTF-8が有効な ( <code>--enable-utf</code> オプションを用いて make する ) PCRE 6.5 以降が必要となる。
    Perl互換の正規表現なので、従来の POSIX 拡張の正規表現から設定変更が必要になる場合がある。
  </dd>
  <dt>--with-oniguruma</dt>
  <dd><strong>非推奨</strong>: かわりに <code>--with-regex=oniguruma</code> を使用してください。</dd>
  <dt>--with-pcre</dt>
  <dd><strong>非推奨</strong>: かわりに <code>--with-regex=pcre</code> を使用してください。</dd>

  <dt>--with-thread=[posix|glib|std]</dt>
  <dd>使用するスレッドライブラリを設定する。デフォルトでは std::thread を使用する。(v0.3.0以降)</dd>
  <dt>--with-thread=posix</dt>
  <dd>std::thread のかわりに pthread を使用する。</dd>
  <dt>--with-thread=glib</dt>
  <dd><strong>非推奨</strong>: かわりに <code>--with-thread=std</code> を使用してください。</dd>
  <dt>--with-[gthread|stdthread]</dt>
  <dd><strong>非推奨</strong>: かわりに <code>--with-thread=[glib|std]</code> を使用してください。</dd>

  <dt>--with-gtkmm3=no</dt>
  <dd>gtkmm3のかわりにgtkmm2を使用する。</dd>

  <dt>--disable-compat-cache-dir</dt>
  <dd>JDのキャッシュディレクトリ <code>~/.jd</code> を読み込む互換機能を無効化する。</dd>
</dl>


<a name="memo"></a>
### メモ
最近のディストリビューションの場合は `autogen.sh` よりも `autoreconf -i` の方を推奨。

実行するには直接 `src/jdim` を起動するか手動で `/usr/bin` あたりに `src/jdim` を `cp` する。

以上の操作でmakeが通らなかったり動作が変な時は configure のオプションを変更する。


[wiki-install]: https://ja.osdn.net/projects/jd4linux/wiki/OS%2F%E3%83%87%E3%82%A3%E3%82%B9%E3%83%88%E3%83%AA%E3%83%93%E3%83%A5%E3%83%BC%E3%82%B7%E3%83%A7%E3%83%B3%E5%88%A5%E3%82%A4%E3%83%B3%E3%82%B9%E3%83%88%E3%83%BC%E3%83%AB%E6%96%B9%E6%B3%95
