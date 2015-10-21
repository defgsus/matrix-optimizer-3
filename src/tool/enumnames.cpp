/** @file enumnames.cpp

    @brief strings for enums in Qt

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/9/2014</p>
*/

#include <QApplication>
#include "enumnames.h"

namespace MO {

QString enumName(Qt::Modifier m)
{
    switch (m)
    {
    case Qt::SHIFT: return QApplication::tr("Shift");
    case Qt::META: return QApplication::tr("Meta");
    case Qt::CTRL: return QApplication::tr("Ctrl");
    case Qt::ALT: return QApplication::tr("Alt");
    default: return QApplication::tr("*unknown*");
    }
}

namespace {
    static QMap<int, QString> keyCodeMap_;

    static void createKeyCodeMap_()
    {
#define MO__KEY(key__) keyCodeMap_.insert(Qt::Key_##key__, #key__)

        MO__KEY(Escape);                // misc keys
        MO__KEY(Tab);
        MO__KEY(Backtab);
        MO__KEY(Backspace);
        MO__KEY(Return);
        MO__KEY(Enter);
        MO__KEY(Insert);
        MO__KEY(Delete);
        MO__KEY(Pause);
        MO__KEY(Print);
        MO__KEY(SysReq);
        MO__KEY(Clear);
        MO__KEY(Home);                // cursor movement
        MO__KEY(End);
        MO__KEY(Left);
        MO__KEY(Up);
        MO__KEY(Right);
        MO__KEY(Down);
        MO__KEY(PageUp);
        MO__KEY(PageDown);
        MO__KEY(Shift);                // modifiers
        MO__KEY(Control);
        MO__KEY(Meta);
        MO__KEY(Alt);
        MO__KEY(CapsLock);
        MO__KEY(NumLock);
        MO__KEY(ScrollLock);
        MO__KEY(F1);                // function keys
        MO__KEY(F2);
        MO__KEY(F3);
        MO__KEY(F4);
        MO__KEY(F5);
        MO__KEY(F6);
        MO__KEY(F7);
        MO__KEY(F8);
        MO__KEY(F9);
        MO__KEY(F10);
        MO__KEY(F11);
        MO__KEY(F12);
        MO__KEY(F13);
        MO__KEY(F14);
        MO__KEY(F15);
        MO__KEY(F16);
        MO__KEY(F17);
        MO__KEY(F18);
        MO__KEY(F19);
        MO__KEY(F20);
        MO__KEY(F21);
        MO__KEY(F22);
        MO__KEY(F23);
        MO__KEY(F24);
        MO__KEY(F25);                // F25 .. F35 only on X11
        MO__KEY(F26);
        MO__KEY(F27);
        MO__KEY(F28);
        MO__KEY(F29);
        MO__KEY(F30);
        MO__KEY(F31);
        MO__KEY(F32);
        MO__KEY(F33);
        MO__KEY(F34);
        MO__KEY(F35);
        MO__KEY(Super_L);                 // extra keys
        MO__KEY(Super_R);
        MO__KEY(Menu);
        MO__KEY(Hyper_L);
        MO__KEY(Hyper_R);
        MO__KEY(Help);
        MO__KEY(Direction_L);
        MO__KEY(Direction_R);
        MO__KEY(Space);                // 7 bit printable ASCII
        //MO__KEY(Any);
        MO__KEY(Space);
        MO__KEY(Exclam);
        MO__KEY(QuoteDbl);
        MO__KEY(NumberSign);
        MO__KEY(Dollar);
        MO__KEY(Percent);
        MO__KEY(Ampersand);
        MO__KEY(Apostrophe);
        MO__KEY(ParenLeft);
        MO__KEY(ParenRight);
        MO__KEY(Asterisk);
        MO__KEY(Plus);
        MO__KEY(Comma);
        MO__KEY(Minus);
        MO__KEY(Period);
        MO__KEY(Slash);
        MO__KEY(0);
        MO__KEY(1);
        MO__KEY(2);
        MO__KEY(3);
        MO__KEY(4);
        MO__KEY(5);
        MO__KEY(6);
        MO__KEY(7);
        MO__KEY(8);
        MO__KEY(9);
        MO__KEY(Colon);
        MO__KEY(Semicolon);
        MO__KEY(Less);
        MO__KEY(Equal);
        MO__KEY(Greater);
        MO__KEY(Question);
        MO__KEY(At);
        MO__KEY(A);
        MO__KEY(B);
        MO__KEY(C);
        MO__KEY(D);
        MO__KEY(E);
        MO__KEY(F);
        MO__KEY(G);
        MO__KEY(H);
        MO__KEY(I);
        MO__KEY(J);
        MO__KEY(K);
        MO__KEY(L);
        MO__KEY(M);
        MO__KEY(N);
        MO__KEY(O);
        MO__KEY(P);
        MO__KEY(Q);
        MO__KEY(R);
        MO__KEY(S);
        MO__KEY(T);
        MO__KEY(U);
        MO__KEY(V);
        MO__KEY(W);
        MO__KEY(X);
        MO__KEY(Y);
        MO__KEY(Z);
        MO__KEY(BracketLeft);
        MO__KEY(Backslash);
        MO__KEY(BracketRight);
        MO__KEY(AsciiCircum);
        MO__KEY(Underscore);
        MO__KEY(QuoteLeft);
        MO__KEY(BraceLeft);
        MO__KEY(Bar);
        MO__KEY(BraceRight);
        MO__KEY(AsciiTilde);

        MO__KEY(nobreakspace);
        MO__KEY(exclamdown);
        MO__KEY(cent);
        MO__KEY(sterling);
        MO__KEY(currency);
        MO__KEY(yen);
        MO__KEY(brokenbar);
        MO__KEY(section);
        MO__KEY(diaeresis);
        MO__KEY(copyright);
        MO__KEY(ordfeminine);
        MO__KEY(guillemotleft);        // left angle quotation mark
        MO__KEY(notsign);
        MO__KEY(hyphen);
        MO__KEY(registered);
        MO__KEY(macron);
        MO__KEY(degree);
        MO__KEY(plusminus);
        MO__KEY(twosuperior);
        MO__KEY(threesuperior);
        MO__KEY(acute);
        MO__KEY(mu);
        MO__KEY(paragraph);
        MO__KEY(periodcentered);
        MO__KEY(cedilla);
        MO__KEY(onesuperior);
        MO__KEY(masculine);
        MO__KEY(guillemotright);        // right angle quotation mark
        MO__KEY(onequarter);
        MO__KEY(onehalf);
        MO__KEY(threequarters);
        MO__KEY(questiondown);
        MO__KEY(Agrave);
        MO__KEY(Aacute);
        MO__KEY(Acircumflex);
        MO__KEY(Atilde);
        MO__KEY(Adiaeresis);
        MO__KEY(Aring);
        MO__KEY(AE);
        MO__KEY(Ccedilla);
        MO__KEY(Egrave);
        MO__KEY(Eacute);
        MO__KEY(Ecircumflex);
        MO__KEY(Ediaeresis);
        MO__KEY(Igrave);
        MO__KEY(Iacute);
        MO__KEY(Icircumflex);
        MO__KEY(Idiaeresis);
        MO__KEY(ETH);
        MO__KEY(Ntilde);
        MO__KEY(Ograve);
        MO__KEY(Oacute);
        MO__KEY(Ocircumflex);
        MO__KEY(Otilde);
        MO__KEY(Odiaeresis);
        MO__KEY(multiply);
        MO__KEY(Ooblique);
        MO__KEY(Ugrave);
        MO__KEY(Uacute);
        MO__KEY(Ucircumflex);
        MO__KEY(Udiaeresis);
        MO__KEY(Yacute);
        MO__KEY(THORN);
        MO__KEY(ssharp);
        MO__KEY(division);
        MO__KEY(ydiaeresis);

        // International input method support (X keycode - 0xEE00, the
        // definition follows Qt/Embedded 2.3.7) Only interesting if
        // you are writing your own input method

        // International & multi-key character composition
        MO__KEY(AltGr              );
        MO__KEY(Multi_key          );  // Multi-key character compose
        MO__KEY(Codeinput          );
        MO__KEY(SingleCandidate    );
        MO__KEY(MultipleCandidate  );
        MO__KEY(PreviousCandidate  );

        // Misc Functions
        MO__KEY(Mode_switch        );  // Character set switch
        //MO__KEY(script_switch      );  // Alias for mode_switch

        // Japanese keyboard support
        MO__KEY(Kanji              );  // Kanji, Kanji convert
        MO__KEY(Muhenkan           );  // Cancel Conversion
        //MO__KEY(Henkan_Mode        );  // Start/Stop Conversion
        MO__KEY(Henkan             );  // Alias for Henkan_Mode
        MO__KEY(Romaji             );  // to Romaji
        MO__KEY(Hiragana           );  // to Hiragana
        MO__KEY(Katakana           );  // to Katakana
        MO__KEY(Hiragana_Katakana  );  // Hiragana/Katakana toggle
        MO__KEY(Zenkaku            );  // to Zenkaku
        MO__KEY(Hankaku            );  // to Hankaku
        MO__KEY(Zenkaku_Hankaku    );  // Zenkaku/Hankaku toggle
        MO__KEY(Touroku            );  // Add to Dictionary
        MO__KEY(Massyo             );  // Delete from Dictionary
        MO__KEY(Kana_Lock          );  // Kana Lock
        MO__KEY(Kana_Shift         );  // Kana Shift
        MO__KEY(Eisu_Shift         );  // Alphanumeric Shift
        MO__KEY(Eisu_toggle        );  // Alphanumeric toggle
        //MO__KEY(Kanji_Bangou       );  // Codeinput
        //MO__KEY(Zen_Koho           );  // Multiple/All Candidate(s)
        //MO__KEY(Mae_Koho           );  // Previous Candidate

        // Korean keyboard support
        //
        // In fact, many Korean users need only 2 keys, MO__KEY(Hangul and
        // MO__KEY(Hangul_Hanja. But rest of the keys are good for future.

        MO__KEY(Hangul             );  // Hangul start/stop(toggle)
        MO__KEY(Hangul_Start       );  // Hangul start
        MO__KEY(Hangul_End         );  // Hangul end, English start
        MO__KEY(Hangul_Hanja       );  // Start Hangul->Hanja Conversion
        MO__KEY(Hangul_Jamo        );  // Hangul Jamo mode
        MO__KEY(Hangul_Romaja      );  // Hangul Romaja mode
        //MO__KEY(Hangul_Codeinput   );  // Hangul code input mode
        MO__KEY(Hangul_Jeonja      );  // Jeonja mode
        MO__KEY(Hangul_Banja       );  // Banja mode
        MO__KEY(Hangul_PreHanja    );  // Pre Hanja conversion
        MO__KEY(Hangul_PostHanja   );  // Post Hanja conversion
        //MO__KEY(Hangul_SingleCandidate  );  // Single candidate
        //MO__KEY(Hangul_MultipleCandidate);  // Multiple candidate
        //MO__KEY(Hangul_PreviousCandidate);  // Previous candidate
        MO__KEY(Hangul_Special     );  // Special symbols
        //MO__KEY(Hangul_switch      );  // Alias for mode_switch

        // dead keys (X keycode - 0xED00 to avoid the conflict)
        MO__KEY(Dead_Grave         );
        MO__KEY(Dead_Acute         );
        MO__KEY(Dead_Circumflex    );
        MO__KEY(Dead_Tilde         );
        MO__KEY(Dead_Macron        );
        MO__KEY(Dead_Breve         );
        MO__KEY(Dead_Abovedot      );
        MO__KEY(Dead_Diaeresis     );
        MO__KEY(Dead_Abovering     );
        MO__KEY(Dead_Doubleacute   );
        MO__KEY(Dead_Caron         );
        MO__KEY(Dead_Cedilla       );
        MO__KEY(Dead_Ogonek        );
        MO__KEY(Dead_Iota          );
        MO__KEY(Dead_Voiced_Sound  );
        MO__KEY(Dead_Semivoiced_Sound);
        MO__KEY(Dead_Belowdot      );
        MO__KEY(Dead_Hook          );
        MO__KEY(Dead_Horn          );

        // multimedia/internet keys - ignored by default - see QKeyEvent c'tor
        MO__KEY(Back );
        MO__KEY(Forward );
        MO__KEY(Stop );
        MO__KEY(Refresh );
        MO__KEY(VolumeDown);
        MO__KEY(VolumeMute );
        MO__KEY(VolumeUp);
        MO__KEY(BassBoost);
        MO__KEY(BassUp);
        MO__KEY(BassDown);
        MO__KEY(TrebleUp);
        MO__KEY(TrebleDown);
        MO__KEY(MediaPlay );
        MO__KEY(MediaStop );
        MO__KEY(MediaPrevious );
        MO__KEY(MediaNext );
        MO__KEY(MediaRecord);
        MO__KEY(MediaPause);
        MO__KEY(MediaTogglePlayPause);
        MO__KEY(HomePage );
        MO__KEY(Favorites );
        MO__KEY(Search );
        MO__KEY(Standby);
        MO__KEY(OpenUrl);
        MO__KEY(LaunchMail );
        MO__KEY(LaunchMedia);
        MO__KEY(Launch0 );
        MO__KEY(Launch1 );
        MO__KEY(Launch2 );
        MO__KEY(Launch3 );
        MO__KEY(Launch4 );
        MO__KEY(Launch5 );
        MO__KEY(Launch6 );
        MO__KEY(Launch7 );
        MO__KEY(Launch8 );
        MO__KEY(Launch9 );
        MO__KEY(LaunchA );
        MO__KEY(LaunchB );
        MO__KEY(LaunchC );
        MO__KEY(LaunchD );
        MO__KEY(LaunchE );
        MO__KEY(LaunchF );
        MO__KEY(MonBrightnessUp);
        MO__KEY(MonBrightnessDown);
        MO__KEY(KeyboardLightOnOff);
        MO__KEY(KeyboardBrightnessUp);
        MO__KEY(KeyboardBrightnessDown);
        MO__KEY(PowerOff);
        MO__KEY(WakeUp);
        MO__KEY(Eject);
        MO__KEY(ScreenSaver);
        MO__KEY(WWW);
        MO__KEY(Memo);
        MO__KEY(LightBulb);
        MO__KEY(Shop);
        MO__KEY(History);
        MO__KEY(AddFavorite);
        MO__KEY(HotLinks);
        MO__KEY(BrightnessAdjust);
        MO__KEY(Finance);
        MO__KEY(Community);
        MO__KEY(AudioRewind); // Media rewind
        MO__KEY(BackForward);
        MO__KEY(ApplicationLeft);
        MO__KEY(ApplicationRight);
        MO__KEY(Book);
        MO__KEY(CD);
        MO__KEY(Calculator);
        MO__KEY(ToDoList);
        MO__KEY(ClearGrab);
        MO__KEY(Close);
        MO__KEY(Copy);
        MO__KEY(Cut);
        MO__KEY(Display); // Output switch key
        MO__KEY(DOS);
        MO__KEY(Documents);
        MO__KEY(Excel);
        MO__KEY(Explorer);
        MO__KEY(Game);
        MO__KEY(Go);
        MO__KEY(iTouch);
        MO__KEY(LogOff);
        MO__KEY(Market);
        MO__KEY(Meeting);
        MO__KEY(MenuKB);
        MO__KEY(MenuPB);
        MO__KEY(MySites);
        MO__KEY(News);
        MO__KEY(OfficeHome);
        MO__KEY(Option);
        MO__KEY(Paste);
        MO__KEY(Phone);
        MO__KEY(Calendar);
        MO__KEY(Reply);
        MO__KEY(Reload);
        MO__KEY(RotateWindows);
        MO__KEY(RotationPB);
        MO__KEY(RotationKB);
        MO__KEY(Save);
        MO__KEY(Send);
        MO__KEY(Spell);
        MO__KEY(SplitScreen);
        MO__KEY(Support);
        MO__KEY(TaskPane);
        MO__KEY(Terminal);
        MO__KEY(Tools);
        MO__KEY(Travel);
        MO__KEY(Video);
        MO__KEY(Word);
        MO__KEY(Xfer);
        MO__KEY(ZoomIn);
        MO__KEY(ZoomOut);
        MO__KEY(Away);
        MO__KEY(Messenger);
        MO__KEY(WebCam);
        MO__KEY(MailForward);
        MO__KEY(Pictures);
        MO__KEY(Music);
        MO__KEY(Battery);
        MO__KEY(Bluetooth);
        MO__KEY(WLAN);
        MO__KEY(UWB);
        MO__KEY(AudioForward); // Media fast-forward
        MO__KEY(AudioRepeat); // Toggle repeat mode
        MO__KEY(AudioRandomPlay); // Toggle shuffle mode
        MO__KEY(Subtitle);
        MO__KEY(AudioCycleTrack);
        MO__KEY(Time);
        MO__KEY(Hibernate);
        MO__KEY(View);
        MO__KEY(TopMenu);
        MO__KEY(PowerDown);
        MO__KEY(Suspend);
        MO__KEY(ContrastAdjust);

        MO__KEY(LaunchG );
        MO__KEY(LaunchH );

        MO__KEY(TouchpadToggle);
        MO__KEY(TouchpadOn);
        MO__KEY(TouchpadOff);

        MO__KEY(MicMute);

        MO__KEY(Red);
        MO__KEY(Green);
        MO__KEY(Yellow);
        MO__KEY(Blue);

        MO__KEY(ChannelUp);
        MO__KEY(ChannelDown);

        MO__KEY(Guide   );
        MO__KEY(Info    );
        MO__KEY(Settings);

        MO__KEY(MicVolumeUp  );
        MO__KEY(MicVolumeDown);

        MO__KEY(New     );
        MO__KEY(Open    );
        MO__KEY(Find    );
        MO__KEY(Undo    );
        MO__KEY(Redo    );

        MO__KEY(MediaLast);

        // Keypad navigation keys
        MO__KEY(Select);
        MO__KEY(Yes);
        MO__KEY(No);

        // Newer misc keys
        MO__KEY(Cancel );
        MO__KEY(Printer);
        MO__KEY(Execute);
        MO__KEY(Sleep  );
        MO__KEY(Play   ); // Not the same as MO__KEY(MediaPlay
        MO__KEY(Zoom   );
        //MO__KEY(Jisho  ); // IME: Dictionary key
        //MO__KEY(Oyayubi_Left); // IME: Left Oyayubi key
        //MO__KEY(Oyayubi_Right); // IME: Right Oyayubi key
        MO__KEY(Exit   );

        // Device keys
        MO__KEY(Context1);
        MO__KEY(Context2);
        MO__KEY(Context3);
        MO__KEY(Context4);
        MO__KEY(Call);      // set absolute state to in a call (do not toggle state)
        MO__KEY(Hangup);    // set absolute state to hang up (do not toggle state)
        MO__KEY(Flip);
        MO__KEY(ToggleCallHangup); // a toggle key for answering, or hanging up, based on current call state
        MO__KEY(VoiceDial);
        MO__KEY(LastNumberRedial);

        MO__KEY(Camera);
        MO__KEY(CameraFocus);

        MO__KEY(unknown);

#undef MO__KEY
    }
} // namespace

QString enumName(Qt::Key k)
{
    if (keyCodeMap_.isEmpty())
        createKeyCodeMap_();
    auto i = keyCodeMap_.find(k);
    return i == keyCodeMap_.end()
            ? "*unknown*"
            : i.value();
}

const QMap<int, QString>& keycodeNames()
{
    if (keyCodeMap_.isEmpty())
        createKeyCodeMap_();
    return keyCodeMap_;
}

} // namespace MO
